#ifndef WEBCONTROL_H
#define WEBCONTROL_H

#include <Arduino.h>

// Serial4 bridge to the ESP32.
//
// Hardware: Teensy 4.1 Serial4 = pins 16 (RX4) / 17 (TX4). RX requires the
// OctoWS2811 "Pin 18" trace cut + jumper to Pin 16 — see
// ../docs/ESP32-Web-App.md §2.1.
//
// Protocol (subset of LED_Controller_Spec §4.4):
//   ESP32 -> Teensy:   "P:<n>"   "Q:yes" | "Q:no"   "PING"
//   Teensy -> ESP32:   "MODE:operator" | "MODE:web"   "PATTERN:<n>"
//
// Usage:
//   webcontrol::begin();             // in setup()
//   webcontrol::poll();              // every loop(); reads serial, broadcasts status
//   if (webcontrol::overrideActive()) {
//       int idx = webcontrol::overridePattern();   // 0..N
//   }
//
// Override semantics: any incoming P: or Q: command refreshes a 30-second
// timer. While the timer is live, overrideActive() returns true and the
// pattern dispatcher should use overridePattern() instead of the switch state.

namespace webcontrol {

void begin();
void poll();
bool overrideActive();
int  overridePattern();

// Drop the web override immediately (e.g. an operator moved a physical switch
// and should reclaim control this instant, not after the timeout).
void cancelOverride();

// Report the currently-running pattern back to the ESP32 (broadcast every
// 500 ms). Call once main.cpp has decided what to run this frame.
void setCurrentPattern(int idx);

}  // namespace webcontrol

#endif
