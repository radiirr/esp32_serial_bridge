// =================== framer.h ===================
#ifndef FRAMER_H
#define FRAMER_H

#pragma once
#include <Arduino.h>

enum FramingMode {
  RAW = 0,
  SLIP = 1,
  KISS = 2,
  HDLC = 3
};

const char* framingModeToString(FramingMode mode);

void setFramingMode(FramingMode mode);
FramingMode getFramingMode();

// size_t encodeFrame(const uint8_t* in, size_t len, uint8_t* out, size_t outMax);
// size_t decodeFrame(const uint8_t* in, size_t len, uint8_t* out, size_t outMax);

size_t encodeFrame(const uint8_t* in, size_t inLen, uint8_t* out, size_t outMax);
size_t decodeFrame(const uint8_t* in, size_t inLen, uint8_t* out, size_t outMax);


#endif // FRAMER_H
