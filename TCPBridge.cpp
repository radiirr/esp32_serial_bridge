// =================== TCPBridge.cpp ===================
#include "TCPBridge.h"
#include "ConfigManager.h"
#include "Framer.h"

WiFiClient tcpBridgeClient;

static uint8_t buffer[BUFFER_SIZE];
static uint8_t encoded[BUFFER_SIZE];
static uint8_t decoded[BUFFER_SIZE];

//static unsigned long lastConnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // ms
unsigned long lastConnectAttempt = 0;

bool isBridgeConnected() {
  return tcpBridgeClient.connected();
}

unsigned long getLastBridgeAttempt() {
  return lastConnectAttempt;
}

void setupTCPBridge() {
  if (tcpBridgeClient.connected()) {
    tcpBridgeClient.stop();
  }
  lastConnectAttempt = 0;  // resetting for loop attempt try
}

void handleTCPBridge() {
//  static uint8_t buffer[BUFFER_SIZE];
  static unsigned long ledTimer = 0;

  // Connect
  if (!tcpBridgeClient.connected()) {
    unsigned long now = millis();
    if (now - lastConnectAttempt >= reconnectInterval) {
      lastConnectAttempt = now;
      Serial.println(F(""));
      Serial.printf("Connecting to %s:%d ...", config.tcpDstIP, config.tcpDstPort);
      if (tcpBridgeClient.connect(config.tcpDstIP, config.tcpDstPort)) {
        tcpBridgeClient.setNoDelay(true);
        Serial.println(F(""));
        Serial.println("[TCP connected, let i/o flow...]");
      } else {
        Serial.println(F(""));
        Serial.println("TCP connection failed.");
      }
    }
    return;
  }

  // Serial to TCP
int i = 0;
while (Serial.available() && i < BUFFER_SIZE) {
  buffer[i++] = Serial.read();
}

if (config.framingMode != RAW && i > 0) {
//  uint8_t encoded[BUFFER_SIZE];  // not multiplied!
  size_t outLen = encodeFrame(buffer, i, encoded, BUFFER_SIZE);
  if (outLen > 0) {
    tcpBridgeClient.write(encoded, outLen);
    digitalWrite(LED_PIN, HIGH);
    ledTimer = millis();
  }
}

if (config.framingMode == RAW && i > 0) {
  tcpBridgeClient.write(buffer, i);
  digitalWrite(LED_PIN, HIGH);
  ledTimer = millis();
}

// TCP to Serial
int len = tcpBridgeClient.available();
if (len > 0) {
  int n = tcpBridgeClient.read(buffer, min(len, BUFFER_SIZE));
  if (n > 0) {
    if (config.framingMode != RAW) {
//      uint8_t decoded[BUFFER_SIZE];
      size_t outLen = decodeFrame(buffer, n, decoded, BUFFER_SIZE);
      if (outLen > 0) {
        Serial.write(decoded, outLen);
        digitalWrite(LED_PIN, HIGH);
        ledTimer = millis();
      }
    } else {
      Serial.write(buffer, n);
      digitalWrite(LED_PIN, HIGH);
      ledTimer = millis();
    }
  }
}

  // LED indicator return
  if (millis() - ledTimer > 30) {
    digitalWrite(LED_PIN, LOW);
  }
}
