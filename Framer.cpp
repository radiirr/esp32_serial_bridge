// =================== framer.cpp ===================
#include "Framer.h"

FramingMode currentFramingMode = RAW;

void setFramingMode(FramingMode mode) {
  currentFramingMode = mode;
}

FramingMode getFramingMode() {
  return currentFramingMode;
}

const char* framingModeToString(FramingMode mode) {
  switch (mode) {
    case RAW: return "raw";
    case SLIP: return "slip";
    case KISS: return "kiss";
    case HDLC: return "hdlc";
  }
}

// ========== ENCODERS ==========

// ---------- SLIP ----------
size_t encodeSLIP(const uint8_t* in, size_t len, uint8_t* out, size_t outMax) {
  size_t outPos = 0;
  if (outPos < outMax) out[outPos++] = 0xC0;

  for (size_t i = 0; i < len && outPos + 2 < outMax; i++) {
    if (in[i] == 0xC0) {
      out[outPos++] = 0xDB;
      out[outPos++] = 0xDC;
    } else if (in[i] == 0xDB) {
      out[outPos++] = 0xDB;
      out[outPos++] = 0xDD;
    } else {
      out[outPos++] = in[i];
    }
  }

  if (outPos < outMax) out[outPos++] = 0xC0;

  return outPos;
}

// ---------- KISS ----------
size_t encodeKISS(const uint8_t* in, size_t len, uint8_t* out, size_t outMax) {
  size_t outPos = 0;
  if (outPos < outMax) out[outPos++] = 0xC0;
  if (outPos < outMax) out[outPos++] = 0x00;  // TNC data frame

  for (size_t i = 0; i < len && outPos + 2 < outMax; i++) {
    if (in[i] == 0xC0) {
      out[outPos++] = 0xDB;
      out[outPos++] = 0xDC;
    } else if (in[i] == 0xDB) {
      out[outPos++] = 0xDB;
      out[outPos++] = 0xDD;
    } else {
      out[outPos++] = in[i];
    }
  }

  if (outPos < outMax) out[outPos++] = 0xC0;

  return outPos;
}

// ---------- HDLC ----------
size_t encodeHDLC(const uint8_t* in, size_t len, uint8_t* out, size_t outMax) {
  const uint8_t FLAG = 0x7E;
  const uint8_t ESC = 0x7D;
  const uint8_t XOR = 0x20;

  size_t outPos = 0;
  if (outPos < outMax) out[outPos++] = FLAG;

  for (size_t i = 0; i < len && outPos + 2 < outMax; i++) {
    if (in[i] == FLAG || in[i] == ESC) {
      out[outPos++] = ESC;
      out[outPos++] = in[i] ^ XOR;
    } else {
      out[outPos++] = in[i];
    }
  }

  if (outPos < outMax) out[outPos++] = FLAG;

  return outPos;
}

// ---------- RAW ----------
size_t encodeRAW(const uint8_t* in, size_t len, uint8_t* out, size_t outMax) {
  size_t copyLen = (len < outMax) ? len : outMax;
  memcpy(out, in, copyLen);
  return copyLen;
}

// ---------- Dispatcher ----------
size_t encodeFrame(const uint8_t* in, size_t len, uint8_t* out, size_t outMax) {
  switch (currentFramingMode) {
    case SLIP: return encodeSLIP(in, len, out, outMax);
    case KISS: return encodeKISS(in, len, out, outMax);
    case HDLC: return encodeHDLC(in, len, out, outMax);
    case RAW:
    default:   return encodeRAW(in, len, out, outMax);
  }
}

// ========== DECODERS ==========

// --- RAW ---
size_t decodeRAW(const uint8_t* in, size_t len, uint8_t* out, size_t outMax) {
  size_t copyLen = (len < outMax) ? len : outMax;
  memcpy(out, in, copyLen);
  return copyLen;
}

// --- SLIP ---
size_t decodeSLIP(const uint8_t* in, size_t len, uint8_t* out, size_t outMax) {
  size_t outPos = 0;
  bool inPacket = false;

  for (size_t i = 0; i < len; i++) {
    uint8_t b = in[i];
    if (b == 0xC0) {
      if (inPacket) break;  // End of frame
      else {
        inPacket = true;    // Start of frame
        continue;
      }
    }

    if (!inPacket) continue;

    if (b == 0xDB && i + 1 < len) {
      uint8_t next = in[++i];
      if (next == 0xDC) b = 0xC0;
      else if (next == 0xDD) b = 0xDB;
      else continue;  // Invalid
    }

    if (outPos < outMax) out[outPos++] = b;
  }

  return outPos;
}

// --- KISS ---
size_t decodeKISS(const uint8_t* in, size_t len, uint8_t* out, size_t outMax) {
  size_t outPos = 0;
  bool inPacket = false;

  for (size_t i = 0; i < len; i++) {
    uint8_t b = in[i];
    if (b == 0xC0) {
      if (inPacket) break;
      else {
        inPacket = true;
        continue;
      }
    }

    if (!inPacket) continue;

    if (b == 0x00 && outPos == 0) continue;  // first byte is port+cmd
    if (b == 0xDB && i + 1 < len) {
      uint8_t next = in[++i];
      if (next == 0xDC) b = 0xC0;
      else if (next == 0xDD) b = 0xDB;
      else continue;
    }

    if (outPos < outMax) out[outPos++] = b;
  }

  return outPos;
}

// --- HDLC ---
size_t decodeHDLC(const uint8_t* in, size_t len, uint8_t* out, size_t outMax) {
  const uint8_t FLAG = 0x7E;
  const uint8_t ESC = 0x7D;
  const uint8_t XOR = 0x20;

  size_t outPos = 0;
  bool inPacket = false;

  for (size_t i = 0; i < len; i++) {
    uint8_t b = in[i];

    if (b == FLAG) {
      if (inPacket) break; // end of frame
      else {
        inPacket = true;
        continue;
      }
    }

    if (!inPacket) continue;

    if (b == ESC && i + 1 < len) {
      b = in[++i] ^ XOR;
    }

    if (outPos < outMax) out[outPos++] = b;
  }

  return outPos;
}

// --- Dispatcher ---
size_t decodeFrame(const uint8_t* in, size_t len, uint8_t* out, size_t outMax) {
  switch (currentFramingMode) {
    case SLIP: return decodeSLIP(in, len, out, outMax);
    case KISS: return decodeKISS(in, len, out, outMax);
    case HDLC: return decodeHDLC(in, len, out, outMax);
    case RAW:
    default:   return decodeRAW(in, len, out, outMax);
  }
}
