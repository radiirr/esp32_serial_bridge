// =================== TCPConsoleServer.h ===================
#ifndef TCPCONSOLESERVER_H
#define TCPCONSOLESERVER_H

#include <Arduino.h>
#include <WiFi.h>

extern WiFiClient tcpConsoleClient;

void startTCPConsoleServer();
void handleTCPConsoleClient();
void handlePassword(const String &pw);
void processConsoleCommand(const String &cmd);
void handleSetCommand(const String &param);

#endif
