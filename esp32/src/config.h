#pragma once

static const char* const AP_SSID = "Enter Infinity";

#define AP_IP_OCTET_1 192
#define AP_IP_OCTET_2 168
#define AP_IP_OCTET_3 4
#define AP_IP_OCTET_4 1

static const int   BRIDGE_RX_PIN  = 16;  // ESP32 RX2 <- Teensy TX (Pin 17)
static const int   BRIDGE_TX_PIN  = 17;  // ESP32 TX2 -> Teensy RX (Pin 16 via mod)
static const long  BRIDGE_BAUD    = 115200;

// Spec suggests 5 s; 2 s is friendlier on the bench.
static const unsigned long PER_IP_COOLDOWN_MS = 2000;

// One-at-a-time controller: the active phone's slot frees if no activity or
// heartbeat arrives for this long (covers walk-aways / locked phones). The
// active client heartbeats every ~8 s, so this tolerates several missed beats.
static const unsigned long SESSION_IDLE_MS = 45000;

// Both default OFF for the festival — every enabled line is a blocking
// Serial.print on the USB CDC that can stall the request path under load.
// DEBUG_BRIDGE: ~6 lines/sec of bridge traffic. DEBUG_HTTP: per-request +
// captive-probe logging (storms hard when every phone's OS probes).
#define DEBUG_BRIDGE 0
#define DEBUG_HTTP   0
