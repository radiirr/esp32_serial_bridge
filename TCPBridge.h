// =================== TCPBridge.h ===================
#ifndef TCPBRIDGE_H
#define TCPBRIDGE_H

#include <WiFi.h>

// buffer size for data
// not = (!=1024) some e.g:
//#define BUFFER_SIZE 512
//#define BUFFER_SIZE 1528
#define BUFFER_SIZE 8192
// led pin gpio, rx/tx indicator while transfer
#define LED_PIN 2

extern WiFiClient tcpBridgeClient;

void setupTCPBridge();
void handleTCPBridge();

bool isBridgeConnected();
unsigned long getLastBridgeAttempt();


#endif
