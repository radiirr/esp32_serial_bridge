// =================== TCPConsoleServer.cpp ===================
#include <WiFi.h>
#include "TCPConsoleServer.h"
#include "TCPBridge.h"
#include "ConfigManager.h"
#include "MenuSystem.h"
#include "Framer.h"
#include <Arduino.h>

WiFiServer* tcpServer = nullptr;
WiFiClient tcpConsoleClient;

bool consoleActive = true; // console active or disabled
bool authenticated = false; // console authed or not
bool waitingForPassword = true; // console auth passwd required or not
String inputLine = "";


void startTCPConsoleServer() {
  tcpServer = new WiFiServer(config.tcpServerPort);
  tcpServer->begin();
  Serial.printf("TCP console server starting on port %d", config.tcpServerPort);
}

void handleTCPConsoleClient() {
  if (!tcpConsoleClient || !tcpConsoleClient.connected()) {
    if (tcpServer && tcpServer->hasClient()) {
//      Serial.println("TCP Server: new client connected"); // do not print into serial while transfer!
      if (tcpConsoleClient) {
        tcpConsoleClient.stop();
      }
      tcpConsoleClient = tcpServer->available();
      tcpConsoleClient.setNoDelay(true);
      consoleActive = true;
      authenticated = false;
      waitingForPassword = true;
      inputLine = "";
      tcpConsoleClient.println("welcome to serial bridge console");
      tcpConsoleClient.println("please enter password:> ");
    }

  } else if (tcpConsoleClient.connected() && tcpConsoleClient.available()) {
    char c = tcpConsoleClient.read();
    if (c == '\r') return;
    if (c == '\n') {
        if (waitingForPassword) {
            handlePassword(inputLine);
        } else if (authenticated) {
            processConsoleCommand(inputLine);
        }
        inputLine.clear(); // string deletion
        if (tcpConsoleClient.connected()) {
            tcpConsoleClient.print(authenticated ? "> " : "please enter password:> ");
        }
    } else {
        inputLine += c;
//        tcpConsoleClient.println(c); // echo back (optional)
    }
  }
}


void handlePassword(const String &pw) {
  if (pw == config.authPassword) {
    authenticated = true;
    waitingForPassword = false;
    tcpConsoleClient.println("[ authentication successful ]");
    tcpConsoleClient.println("type 'help' for commands.");
  } else {
    tcpConsoleClient.println("incorrect password, try again...");
  }
}

void processConsoleCommand(const String &cmd) {
  String command = cmd;
  command.trim();

  if (command.equalsIgnoreCase("help")) {
    tcpConsoleClient.println("available commands:");
    tcpConsoleClient.println("-------------------");
    tcpConsoleClient.println("  help                - show this help");
    tcpConsoleClient.println("  status              - show current/running config and status");
    tcpConsoleClient.println("  set speed N         - set serial speed [ 115200, 9600, 300, 1200, 2400, 4800, 19200, 38400, 57600 ]");
    tcpConsoleClient.println("  set mode M          - set serial mode [ raw, slip, kiss, hdlc ]");
    tcpConsoleClient.println("  set tcpdstip IP     - set TCP destination IP (e.g., 192.168.1.100)");
    tcpConsoleClient.println("  set tcpdstport PORT - set TCP destination port (e.g., 2323)");
    tcpConsoleClient.println("  save                - save current config");
    tcpConsoleClient.println("  reboot              - restart device");
    tcpConsoleClient.println("  exit                - close this console");
  } 
  else if (command.equalsIgnoreCase("status")) {
    tcpConsoleClient.println("-------------------");
    tcpConsoleClient.printf("buffer = %d \n", BUFFER_SIZE);
    tcpConsoleClient.printf("serial mode = %s \n", framingModeToString(config.framingMode));
    tcpConsoleClient.printf("serial speed = %d \n", config.serialSpeed);
    tcpConsoleClient.printf("wifi ssid = %s \n", config.wifiSSID);
    tcpConsoleClient.printf("tcp client = %s:%d \n", config.tcpDstIP, config.tcpDstPort);
    tcpConsoleClient.printf("tcp console = %s @ %d \n", config.tcpServerEnabled ? "[ ENABLED ]" : "[ DISABLED ]", config.tcpServerPort);
    tcpConsoleClient.printf("serial bridge status = %s \n", isBridgeConnected() ? "[ CONNECTED ]" : "[ DISCONNECTED ]");
    tcpConsoleClient.printf("last reconnect attempt = %lu ms ago \n", millis() - getLastBridgeAttempt());
  } 
  else if (command.equalsIgnoreCase("reboot")) {
    tcpConsoleClient.println("[ rebooting... ]");
    delay(100);
    ESP.restart();
  } 
  else if (command.equalsIgnoreCase("save")) {
    ConfigManager::save();
    tcpConsoleClient.println("[ configuration saved ]");
  }
  else if (command.startsWith("set ")) {
    handleSetCommand(command.substring(4));
  }
  else if (command.equalsIgnoreCase("exit")) {
    tcpConsoleClient.println("goodbye!");
    tcpConsoleClient.stop();
    consoleActive = false;
    authenticated = false;
    waitingForPassword = false;
  }
  else {
    tcpConsoleClient.println("Unknown command, type 'help' for list.");
  }
}

void handleSetCommand(const String &param) {
  int spaceIndex = param.indexOf(' ');
  if (spaceIndex == -1) {
    tcpConsoleClient.println("Invalid set command. Usage: set <param> <value>");
    return;
  }

  String key = param.substring(0, spaceIndex);
  String val = param.substring(spaceIndex + 1);

  if (key.equalsIgnoreCase("speed")) {
    int spd = val.toInt();
    if (spd > 0) {
      config.serialSpeed = spd;
      tcpConsoleClient.printf("serial speed set to %d \n", spd);
    } else {
      tcpConsoleClient.println("Invalid speed value.");
    }
  } 
  else if (key.equalsIgnoreCase("mode")) {
    String v = val;
    v.toLowerCase();

    if (v == "raw") {
      config.framingMode = RAW;
    } else if (v == "slip") {
      config.framingMode = SLIP;
    } else if (v == "kiss") {
      config.framingMode = KISS;
    } else if (v == "hdlc") {
      config.framingMode = HDLC;
    } else {
      tcpConsoleClient.println("Invalid mode value. Allowed: raw, slip, kiss, hdlc");
      return;
    }

    tcpConsoleClient.printf("serial mode set to [ %s ] \n", v.c_str());
  }
  else if (key.equalsIgnoreCase("tcpdstip")) {
    val.trim();
    val.toCharArray(config.tcpDstIP, sizeof(config.tcpDstIP));
    tcpConsoleClient.printf("TCP destination IP set to: %s \n", config.tcpDstIP);
    }
  else if (key.equalsIgnoreCase("tcpdstport")) {
    int port = val.toInt();
    if (port > 0 && port <= 65535) {
      config.tcpDstPort = port;
      tcpConsoleClient.printf("TCP destination port set to: %d \n", config.tcpDstPort);
    } else {
      tcpConsoleClient.println("Invalid port number (must be 1–65535).");
    }
  }
  else {
    tcpConsoleClient.println("Unknown parameter.");
  }
}
