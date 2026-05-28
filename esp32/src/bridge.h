#pragma once

#include <Arduino.h>
#include <string.h>
#include <ctype.h>
#include "config.h"

// Serial bridge to the Teensy. Wire protocol: ../../docs/ESP32-Web-App.md

namespace bridge {

inline HardwareSerial& link() { return Serial2; }

// Fixed buffer (not String) to keep the heap clean — rewritten every 500 ms.
inline char            lastTeensyMode[16] = "unknown";
inline int             lastTeensyPattern  = -1;
inline bool            lastTeensyBeat     = false;
inline unsigned long   lastInboundMs      = 0;

inline void begin() {
  link().begin(BRIDGE_BAUD, SERIAL_8N1, BRIDGE_RX_PIN, BRIDGE_TX_PIN);
}

inline void sendLine(const String& line) {
#if DEBUG_BRIDGE
  Serial.print("[bridge → teensy] ");
  Serial.println(line);
#endif
  link().print(line);
  link().print('\n');
}

inline void sendPattern(int idx)            { sendLine(String("P:") + idx); }
inline void sendAnswer(const String& yn)    { sendLine(String("Q:") + yn); }
inline void sendPing()                      { sendLine("PING"); }

inline void handleLine(const char* line) {
#if DEBUG_BRIDGE
  Serial.print("[bridge ← teensy] ");
  Serial.println(line);
#endif
  if (strncmp(line, "MODE:", 5) == 0) {
    strncpy(lastTeensyMode, line + 5, sizeof(lastTeensyMode) - 1);
    lastTeensyMode[sizeof(lastTeensyMode) - 1] = '\0';
  } else if (strncmp(line, "PATTERN:", 8) == 0) {
    lastTeensyPattern = atoi(line + 8);
  } else if (strncmp(line, "BEAT:", 5) == 0) {
    lastTeensyBeat = (atoi(line + 5) != 0);
  }
  lastInboundMs = millis();
}

inline void poll() {
  static const size_t BUF_MAX = 128;
  static char         buf[BUF_MAX + 1];
  static size_t       len = 0;

  while (link().available()) {
    char c = (char) link().read();
    if (c == '\n') {
      while (len > 0 && isspace((unsigned char)buf[len - 1])) len--;
      buf[len] = '\0';
      const char* start = buf;
      while (*start && isspace((unsigned char)*start)) start++;
      if (*start != '\0') handleLine(start);
      len = 0;
    } else if (c != '\r') {
      if (len < BUF_MAX) buf[len++] = c;
      else               len = 0;  // overflow guard
    }
  }
}

}  // namespace bridge
