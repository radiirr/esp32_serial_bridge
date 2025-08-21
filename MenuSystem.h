// =================== MenuSystem.h ===================
#ifndef MENUSYSTEM_H
#define MENUSYSTEM_H

#include <Arduino.h>
#include "ConfigManager.h"

extern bool bootMenuActive;

void printBootMenu();
void printSerialSpeedMenu();
void printSerialModeMenu();
void printWiFiTCPMenu();
// void showBootMenu(int ledPin);
void startBootMenu();
void updateBootMenu();
void handleMenuInput(char c);
void handleSerialInput();
void handleMenuLoop();
void requestTextInput(const String& field);
void processTextInput(const String& value);


#endif // MENUSYSTEM_H
