// =================== MenuSystem.cpp ===================
#include "MenuSystem.h"
#include "ConfigManager.h"
#include <Arduino.h>

bool inputMode = false;
String inputField = "";
String inputBuffer = "";
static char currentMenu = 'b';  // 'b' = boot, '0' = speed, '1' = mode, '2' = wifi/tcp
bool inBootMenu = true;  // in the menu by default for the first 10 seconds

bool bootMenuActive = true;
unsigned long bootMenuStartTime = 0;
const unsigned long bootMenuTimeout = 10000;  // 10 secs for boot menu timeout

void startBootMenu() {
  Serial.println(F(""));
  Serial.println(F("+======================[menu control]=====================+"));
  Serial.println(F("|-type:|'e' to enter settings menu, or 'x' exit serial now|"));
  Serial.println(F("|-n:| autoexiting serial within 10 seconds...             |"));
  Serial.println(F("+---------------------------------------------------------+"));
  Serial.println(F(""));
  printBootMenu();

  bootMenuStartTime = millis();
  bootMenuActive = true;
}

void updateBootMenu() {
  if (!bootMenuActive) return;

  // Check timeout
  if (millis() - bootMenuStartTime > bootMenuTimeout) {
    Serial.println("|-i:| autoexiting boot menu, starting serial bridge...");
    bootMenuActive = false;
    return;
  }

  // Check serial input
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'x') {
      Serial.println("[|-i:| exit menu now - let i/o flow]");
      bootMenuActive = false;
    } else if (c == 'e') {
      Serial.println("|-i:| Entered into menu...");
      handleMenuLoop();
      bootMenuActive = false;
    }
  }
}

void printBootMenu() {
  Serial.println(F("+==========[serial control]==========+"));
  Serial.println(F("|-i:| running settings:"));
  Serial.println(F("|------------------------------------|"));
  Serial.print(F("| [mode]: ")); Serial.println(config.framingMode);
  Serial.print(F("| [speed]: ")); Serial.println(config.serialSpeed);
  Serial.print(F("| [ssid]: ")); Serial.println(config.wifiSSID);
  Serial.print(F("| [tcp client]: ")); Serial.print(config.tcpDstIP); Serial.print(":"); Serial.println(config.tcpDstPort);
  Serial.print(F("| [tcp console]: ")); Serial.print(config.tcpServerEnabled ? "enabled - on" : "disabled - off");
  Serial.print(" @ "); Serial.println(config.tcpServerPort);
  Serial.println(F("|------------------------------------|"));
  Serial.println(F("|->| [ 0 ] = set serial speed"));
  Serial.println(F("|->| [ 1 ] = set serial mode"));
  Serial.println(F("|->| [ 2 ] = set wifi,tcp settings"));
  Serial.println(F("|------------------------------------|"));
  Serial.println(F("|-e| [ e ] = enter into settings menu"));
  Serial.println(F("|-s| [ s ] = save settings"));
  Serial.println(F("|-r| [ r ] = reboot"));
  Serial.println(F("|-x| [ x ] = exit"));
  Serial.println(F("+====================================+"));

}

// === SUB MENU 0: Serial Speed ===
void printSerialSpeedMenu() {
  Serial.println(F("+==========[serial speed]===========+"));
  Serial.print(F("|-i:| [running_speed]: ")); Serial.println(config.serialSpeed);
  Serial.println(F("|-----------------------------------|"));
  Serial.println(F("|->| [ 0 ] = 115200  bps - default"));
  Serial.println(F("|->| [ 1 ] = 9600    bps"));
  Serial.println(F("|->| [ 2 ] = 300     bps"));
  Serial.println(F("|->| [ 3 ] = 1200    bps"));
  Serial.println(F("|->| [ 4 ] = 2400    bps"));
  Serial.println(F("|->| [ 5 ] = 4800    bps"));
  Serial.println(F("|->| [ 6 ] = 19200   bps"));
  Serial.println(F("|->| [ 7 ] = 38400   bps"));
  Serial.println(F("|->| [ 8 ] = 57600   bps"));
  Serial.println(F("|-s| [ s ] = save setting"));
  Serial.println(F("|-r| [ r ] = reboot"));
  Serial.println(F("|-b| [ b ] = boot menu"));
  Serial.println(F("|-x| [ x ] = exit"));
  Serial.println(F("+-----------------------------------+"));
}

// === SUB MENU 1: Serial Mode ===
void printSerialModeMenu() {
  Serial.println(F("+=============[serial mode]===============+"));
  Serial.print(F("|-i:| [running_mode]: ")); Serial.println(config.framingMode);
  Serial.println(F("|-----------------------------------------|"));
  Serial.println(F("|->| [ 0 ] = raw_mode - default"));
  Serial.println(F("|->| [ 1 ] = slip_mode"));
  Serial.println(F("|->| [ 2 ] = kiss_mode"));
  Serial.println(F("|->| [ 3 ] = hdlc_mode"));
  Serial.println(F("|-----------------------------------------|"));
  Serial.println(F("|-s| [ s ] = save setting"));
  Serial.println(F("|-r| [ r ] = reboot"));
  Serial.println(F("|-b| [ b ] = boot menu"));
  Serial.println(F("|-x| [ x ] = exit"));
  Serial.println(F("|-i:|-------------------------------------|"));
  Serial.println(F("| [raw] - should be @ speed, nortscts"));
  Serial.println(F("| [ppp] - ~ 10.8KB/s , rts/cts"));
  Serial.println(F("| [kissattach],[ax.25] - ~ 8.25KB/s"));
  Serial.println(F("| [tncattach],[ethernet] - ~ 10.5KB/s"));
  Serial.println(F("|  serial no control ideal for experiment"));
  Serial.println(F("| [slip] framing as (0xC0,0xDB,0xDC,0xDD)"));
  Serial.println(F("|  rts/cts, 8N1, ideal for rawip,tcp_rawip"));
  Serial.println(F("| [kiss] framing as (0xC0,0xDB,0xDC,0xDD)"));
  Serial.println(F("|  ideal for kiss,tnc,ax.25"));
  Serial.println(F("| [hdlc] framing as (0x7E,0x7D,0x20) "));
  Serial.println(F("|  ideal for hdlc abm framing, reliable"));
  Serial.println(F("|  direct,frame relay,X.25 - no crc"));
  Serial.println(F("+-----------------------------------------+"));
}

// === SUB MENU 2: WiFi/TCP Settings ===
void printWiFiTCPMenu() {
  Serial.println(F("+=========[wifi,tcp settings]=========+"));
  Serial.println(F("|-i:| running settings:"));
  Serial.println(F("|-------------------------------------|"));
  Serial.print(F("| [ssid]: ")); Serial.println(config.wifiSSID);
  Serial.print(F("| [passwd]: ")); Serial.println(config.wifiPassword);
  Serial.print(F("| [tcpdstIP:port]: ")); Serial.print(config.tcpDstIP); Serial.print(":"); Serial.println(config.tcpDstPort);
  Serial.print(F("| [tcpServer]: ")); Serial.print(config.tcpServerEnabled ? "enabled" : "disabled");
  Serial.print(" @ "); Serial.println(config.tcpServerPort);
  Serial.print(F("| [auth passwd]: ")); Serial.println(config.authPassword);
  Serial.println(F("|-------------------------------------|"));
  Serial.println(F("|->| [ 0 ] = set wifi ap ssid"));
  Serial.println(F("|->| [ 1 ] = set wifi password"));
  Serial.println(F("|->| [ 2 ] = set remote tcp dst_ip"));
  Serial.println(F("|->| [ 3 ] = set remote tcp dst_port"));
  Serial.println(F("|->| [ 4 ] = set tcpconsole server, toggle on/off"));
  Serial.println(F("|->| [ 5 ] = set tcpconsole port"));
  Serial.println(F("|->| [ 6 ] = set tcpconsole auth passwd"));
  Serial.println(F("|-s| [ s ] = save setting"));
  Serial.println(F("|-r| [ r ] = reboot"));
  Serial.println(F("|-b| [ b ] = boot menu"));
  Serial.println(F("|-x| [ x ] = exit"));
  Serial.println(F("+-------------------------------------+"));
}

void handleMenuLoop() {
  inBootMenu = true;
  while (inBootMenu) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'x') {
        Serial.println("[exit menu]");
        inBootMenu = false;
      } else {
        handleMenuInput(c);
      }
    }
    delay(10);
  }
}

void handleMenuInput(char c) {
    if (inputMode) {
    // Enter key: process the input
    if (c == '\n' || c == '\r') {
      processTextInput(inputBuffer);
      inputBuffer.clear();
    }
    // Backspace handler
    else if (c == 8 || c == 127) {
      if (inputBuffer.length() > 0) {
        inputBuffer.remove(inputBuffer.length() - 1);
        Serial.print("\b \b"); // backspace in terminal
      }
    }
    // Attach other charachter
    else {
      inputBuffer += c;
      Serial.print(c);  // write back a charachter
    }
    return;
  }
  switch (currentMenu) {
    case 'b':  // Boot menu
      switch (c) {
        case '0': currentMenu = '0'; printSerialSpeedMenu(); break;
        case '1': currentMenu = '1'; printSerialModeMenu(); break;
        case '2': currentMenu = '2'; printWiFiTCPMenu(); break;
        case 's': ConfigManager::save(); Serial.println("config [saved]"); break;
        case 'r': ESP.restart(); break;
        case 'x': Serial.println("[exiting serial interface]"); break;
//        default: Serial.print("[boot menu: unknown cmd "); Serial.print(c); Serial.println("]"); break;
      }
      break;

    case '0':  // Serial speed submenu
      switch (c) {
        case '0': config.serialSpeed = 115200; Serial.println("[set: 115200]"); break;
        case '1': config.serialSpeed = 9600; Serial.println("[set: 9600]"); break;
        case '2': config.serialSpeed = 300; Serial.println("[set: 300]"); break;
        case '3': config.serialSpeed = 1200; Serial.println("[set: 1200]"); break;
        case '4': config.serialSpeed = 2400; Serial.println("[set: 2400]"); break;
        case '5': config.serialSpeed = 4800; Serial.println("[set: 4800]"); break;
        case '6': config.serialSpeed = 19200; Serial.println("[set: 19200]"); break;
        case '7': config.serialSpeed = 38400; Serial.println("[set: 38400]"); break;
        case '8': config.serialSpeed = 57600; Serial.println("[set: 57600]"); break;
        case 's': ConfigManager::save(); Serial.println("serial speed [saved]"); break;
        case 'r': ESP.restart(); break;
        case 'b': currentMenu = 'b'; printBootMenu(); break;
        case 'x': Serial.println("[exiting serial interface]"); break;
//        default: Serial.println("[invalid speed option]"); break;
      }
      break;

    case '1':  // Serial mode submenu
      switch (c) {
        case '0': config.framingMode = RAW; Serial.println("[mode: raw]"); break;
        case '1': config.framingMode = SLIP; Serial.println("[mode: slip]"); break;
        case '2': config.framingMode = KISS; Serial.println("[mode: kiss]"); break;
        case '3': config.framingMode = HDLC; Serial.println("[mode: hdlc]"); break;
        case 's': ConfigManager::save(); Serial.println("serial mode [saved]"); break;
        case 'r': ESP.restart(); break;
        case 'b': currentMenu = 'b'; printBootMenu(); break;
        case 'x': Serial.println("[exiting serial interface]"); break;
    // default: Serial.println("[invalid serial mode option]"); break;
      }
      break;

    case '2':  // WiFi/TCP settings submenu
      switch (c) {
        case '0': requestTextInput("SSID"); break;
        case '1': requestTextInput("PASSWORD"); break;
        case '2': requestTextInput("DST_IP"); break;
        case '3': requestTextInput("DST_PORT"); break;
        case '4': config.tcpServerEnabled = !config.tcpServerEnabled; Serial.print("[tcp server toggled: "); Serial.print(config.tcpServerEnabled ? "on" : "off"); Serial.println("]"); break;
        case '5': requestTextInput("LOCAL_PORT"); break;
        case '6': requestTextInput("AUTH"); break;
        case 's': ConfigManager::save(); Serial.println("wifi/tcp settings [saved]"); break;
        case 'r': ESP.restart(); break;
        case 'b': currentMenu = 'b'; printBootMenu(); break;
        case 'x': Serial.println("[exiting serial interface]"); break;
//        default: Serial.println("[invalid wifi/tcp option]"); break;
      }
      break;

    default:
      Serial.println("[unknown menu context]");
      currentMenu = 'b';
      break;
  }
}

void requestTextInput(const String& field) {
  // Flushing the input puffer for avoid "latest" Enters
  while (Serial.available()) {
    Serial.read();
  }

  inputMode = true;
  inputField = field;
  inputBuffer = "";

  Serial.print("[enter value for ");
  Serial.print(field);
  Serial.println("]: ");
}

void processTextInput(const String& value) {
  if (inputField == "SSID") {
    value.toCharArray(config.wifiSSID, sizeof(config.wifiSSID));
    Serial.println(" - [SSID set]");
  } else if (inputField == "PASSWORD") {
    value.toCharArray(config.wifiPassword, sizeof(config.wifiPassword));
    Serial.println(" - [WiFi password set]");
  } else if (inputField == "DST_IP") {
    value.toCharArray(config.tcpDstIP, sizeof(config.tcpDstIP));
    Serial.println(" - [destination IP set]");
  } else if (inputField == "DST_PORT") {
    config.tcpDstPort = value.toInt();
    Serial.println(" - [destination port set]");
  } else if (inputField == "LOCAL_PORT") {
    config.tcpServerPort = value.toInt();
    Serial.println(" - [local TCP server port set]");
  } else if (inputField == "AUTH") {
    value.toCharArray(config.authPassword, sizeof(config.authPassword));
    Serial.println(" - [auth password set]");
  } else if (inputField == "FRAMING") {
  int mode = value.toInt();
  if (mode >= 0 && mode <= 3) {
    config.framingMode = (FramingMode)mode;
    setFramingMode((FramingMode)mode);
    Serial.println(" - [Framing mode set]");
  } else {
    Serial.println("[Invalid framing mode]");
  }
}

  inputMode = false;
  inputField = "";
  inputBuffer = "";

  // Refresh the current submenu after setting
  switch (currentMenu) {
    case '2': printWiFiTCPMenu(); break;
    default: printBootMenu(); break;
  }
}

void handleSerialInput() {
  if (Serial.available()) {
    char c = Serial.read();
    if (inputMode) {
      if (c == '\n' || c == '\r') {
        if (inputBuffer.length() > 0) {
          processTextInput(inputBuffer);
          inputBuffer.clear();
        }
      } else {
        inputBuffer += c;
      }
    } else {
      handleMenuInput(c);
    }
  }
}
