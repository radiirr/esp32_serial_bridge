// =================== ConfigManager.cpp ===================
#include "ConfigManager.h"
#include <EEPROM.h>

#define EEPROM_SIZE 512 // eeprom size
#define CONFIG_MAGIC 0x42  // simple marker for valid config

Config config;

void ConfigManager::setDefaultConfig() {
  config.serialSpeed = 115200; // serial baud
//  strcpy(config.serialMode, "raw"); // serial mode !unused_parameter
  strcpy(config.wifiSSID, "ap1"); // wifi ap ssid
  strcpy(config.wifiPassword, "LEtjOWn6BesRJ1ZVN"); // wifi passwd
  strcpy(config.tcpDstIP, "192.168.1.11"); // IPv4/6 destination address of the remote host
  config.tcpDstPort = 99; // remote host destination port
  config.tcpServerEnabled = true; // tcp console server enabled or disabled
  config.tcpServerPort = 23; // tcp console server source port
  strcpy(config.authPassword, "admin"); // tcp console server auth passwd
  config.framingMode = FramingMode::RAW; // RAW framing mode for default - 0=RAW, 1=SLIP, 2=KISS, 3=HDLC

}

void ConfigManager::load() {
  EEPROM.begin(EEPROM_SIZE);

  uint8_t marker = 0;
  EEPROM.get(0, marker);

  if (marker == CONFIG_MAGIC) {
    EEPROM.get(1, config);
    Serial.println("[config loaded from EEPROM]");
  } else {
    setDefaultConfig();
    Serial.println("[no valid config found, using defaults]");
  }

  EEPROM.end();
}

void ConfigManager::save() {
  EEPROM.begin(EEPROM_SIZE);

  EEPROM.put(0, CONFIG_MAGIC);
  EEPROM.put(1, config);
  EEPROM.commit();

  EEPROM.end();
  Serial.println("[config saved to EEPROM]");
}
