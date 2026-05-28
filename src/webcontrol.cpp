#include <string.h>
#include <ctype.h>
#include "webcontrol.h"
#include "./app/app_patterns.h"
// Mic / beat-detect disabled until MAX4466 hardware lands. BEAT:on/off
// commands are now silently ignored; BEAT: status broadcast is suppressed.
// #include "beatdetect.h"

namespace webcontrol {

static const long          BRIDGE_BAUD          = 115200;
static const unsigned long OVERRIDE_TIMEOUT_MS  = 30000;
static const unsigned long STATUS_BROADCAST_MS  = 500;

// The ESP pings every 5 s. We only honour web overrides while those pings are
// arriving — proof a real, protocol-speaking ESP is connected. With the ESP
// unplugged the Serial4 RX pin floats and reads noise; gating on a live link
// (plus the strict command validation below) keeps that garbage off the
// override path so it can't hijack the operator switches.
static const unsigned long LINK_ALIVE_MS = 12000;  // ~2 missed 5 s pings

// Highest valid pattern index (app_patterns::CALIBRATE — the wiring-direction
// bench tool). Commands outside 0..MAX are rejected rather than blindly
// dispatched.
static const int MAX_PATTERN_INDEX = app_patterns::CALIBRATE;

// Mapping of Q:yes / Q:no when the ESP32 doesn't send an explicit P:<n>.
static const int Q_YES_PATTERN_FALLBACK = 0;  // rainbowChase
static const int Q_NO_PATTERN_FALLBACK  = 3;  // waveVerticalsOverwards

// Fixed-size line buffer — avoids per-byte heap churn that an Arduino String
// would cause across hours of operation.
static const size_t RX_BUF_MAX = 128;
static char         _rxBuf[RX_BUF_MAX + 1];
static size_t       _rxLen           = 0;

static int           _overridePattern = -1;
static unsigned long _overrideSetMs   = 0;
static int           _currentPattern  = -1;
static unsigned long _lastBroadcastMs = 0;
static unsigned long _lastPingMs      = 0;   // last valid PING from the ESP

// True only for a non-empty, all-decimal-digit string.
static bool allDigits(const char* s) {
  if (*s == '\0') return false;
  for (; *s; ++s) if (!isdigit((unsigned char)*s)) return false;
  return true;
}

// The bridge link is "alive" only while the ESP's 5 s pings keep arriving.
static bool linkAlive() {
  return _lastPingMs != 0 && (millis() - _lastPingMs) < LINK_ALIVE_MS;
}

static void handleLine(const char* line) {
  if (line[0] == '\0') return;

  if (strncmp(line, "P:", 2) == 0) {
    using namespace app_patterns;
    int idx = -1;
    if      (strcmp(line, "P:dim_ambient")  == 0) idx = DIM_AMBIENT;
    else if (strcmp(line, "P:fire_full")    == 0) idx = FIRE_FULL;
    else if (strcmp(line, "P:idle_ambient") == 0) idx = IDLE_AMBIENT;
    else if (strcmp(line, "P:calibrate")    == 0) idx = CALIBRATE;
    else if (allDigits(line + 2))                 idx = atoi(line + 2);
    // Reject unknown tokens, non-numeric junk, and out-of-range indices so
    // stray/garbage bytes (e.g. a floating RX pin) can't set an override.
    if (idx < 0 || idx > MAX_PATTERN_INDEX) return;
    _overridePattern = idx;
    _overrideSetMs   = millis();
  } else if (strcmp(line, "Q:yes") == 0) {
    _overridePattern = Q_YES_PATTERN_FALLBACK;
    _overrideSetMs   = millis();
  } else if (strcmp(line, "Q:no") == 0) {
    _overridePattern = Q_NO_PATTERN_FALLBACK;
    _overrideSetMs   = millis();
  } else if (strcmp(line, "PING") == 0) {
    _lastPingMs = millis();   // link liveness — gates overrideActive()
  } else {
    // Unknown — silently ignored. Future BEAT:on/off, C:, S:, B:, etc. land
    // here and can be added without touching the framing.
  }
}

static void drainSerial() {
  while (Serial4.available()) {
    char c = (char) Serial4.read();
    if (c == '\n') {
      // Trim trailing whitespace in place.
      while (_rxLen > 0 && isspace((unsigned char)_rxBuf[_rxLen - 1])) _rxLen--;
      _rxBuf[_rxLen] = '\0';
      // Skip leading whitespace.
      const char* start = _rxBuf;
      while (*start && isspace((unsigned char)*start)) start++;
      handleLine(start);
      _rxLen = 0;
    } else if (c != '\r') {
      if (_rxLen < RX_BUF_MAX) {
        _rxBuf[_rxLen++] = c;
      } else {
        _rxLen = 0;  // overflow guard — drop the garbage line
      }
    }
  }
}

static void broadcastStatus() {
  unsigned long now = millis();
  if (now - _lastBroadcastMs < STATUS_BROADCAST_MS) return;
  _lastBroadcastMs = now;

  Serial4.print(overrideActive() ? "MODE:web\n" : "MODE:operator\n");

  if (_currentPattern >= 0) {
    Serial4.print("PATTERN:");
    Serial4.print(_currentPattern);
    Serial4.print('\n');
  }
  // BEAT: status broadcast suppressed while beat detection is disabled.
}

void begin() {
  Serial4.begin(BRIDGE_BAUD);
}

void poll() {
  drainSerial();
  broadcastStatus();
}

bool overrideActive() {
  if (_overridePattern < 0) return false;
  // Ignore overrides unless the ESP is alive (pinging). With the ESP unplugged
  // the Serial4 RX floats; without this gate that noise hijacks the switches.
  if (!linkAlive()) return false;
  return (millis() - _overrideSetMs) < OVERRIDE_TIMEOUT_MS;
}

int overridePattern() {
  return _overridePattern;
}

void cancelOverride() {
  _overridePattern = -1;
}

void setCurrentPattern(int idx) {
  _currentPattern = idx;
}

}  // namespace webcontrol
