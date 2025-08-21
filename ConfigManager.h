// =================== ConfigManager.h ===================
#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Arduino.h>
#include <Framer.h>


struct Config {
  int serialSpeed;
  char serialMode[8];
  char wifiSSID[32];
  char wifiPassword[32];
  char tcpDstIP[46];
  int tcpDstPort;
  bool tcpServerEnabled;
  int tcpServerPort;
  char authPassword[16];
  FramingMode framingMode; // 0=RAW, 1=SLIP, 2=KISS, 3=HDLC
};

extern Config config;

class ConfigManager {
public:
    static void load();
    static void save();
    static void setDefaultConfig();
};

#endif
