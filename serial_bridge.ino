// =================== serial_bridge.ino ===================
#include <WiFi.h>
#include "ConfigManager.h"
#include "MenuSystem.h"
#include "TCPBridge.h"
#include "TCPConsoleServer.h"

static WiFiClient client;
static unsigned long lastConnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // wifi trying for every 5 secs

void setup() {
  ConfigManager::load();
  Serial.begin(config.serialSpeed);
  delay(1000);

  Serial.println("BOOTING...");
  Serial.println("");
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.printf("SSID: %s", config.wifiSSID);
  Serial.println("");
  Serial.printf("PASSWD: %s", config.wifiPassword);
  Serial.println("");

  WiFi.begin(config.wifiSSID, config.wifiPassword);
  int retries = 20;
  while (WiFi.status() != WL_CONNECTED && retries-- > 0) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected - IP: ");
    Serial.println(WiFi.localIP());

    if (config.tcpServerEnabled) {
      startTCPConsoleServer();
    }
  } else {
    Serial.println("WiFi connection failed.");
  }
  startBootMenu();
  setupTCPBridge();
}

void loop() {
  if (bootMenuActive) {
    updateBootMenu(); // non blocking menu handler
  } else {
    handleTCPConsoleClient();
    handleTCPBridge();
  }
}
