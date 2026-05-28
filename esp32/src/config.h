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

// Turn OFF for festival — ~6 noisy lines/sec at the 500 ms broadcast cadence.
#define DEBUG_BRIDGE 1
