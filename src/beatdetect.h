#ifndef BEATDETECT_H
#define BEATDETECT_H

#include <Arduino.h>

// MAX4466 amplitude-peak beat detector.
//
// Pin choice: A1 / Pin 15 (NOT A0 — parent spec lists A0 = Pin 14 but that
// collides with the OctoWS2811 pinList entry driving leds2). See
// ../docs/ESP32-Web-App.md §6.1 for the deviation note.
//
// Algorithm: envelope follower + slow-tracking adaptive baseline + threshold
// gate with lockout. See ../docs/ESP32-Web-App.md §6.2.
//
// Boot state: disabled. Enable via webcontrol receiving "BEAT:on" (or
// directly via setEnabled(true) for bench testing).

namespace beatdetect {

void begin();
void poll();
void setEnabled(bool on);
bool enabled();
bool beatActive();              // true for ~100 ms after each detected beat
unsigned long lastBeatMs();     // millis() at most recent beat (0 if never)

}  // namespace beatdetect

#endif
