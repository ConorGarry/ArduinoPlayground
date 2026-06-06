#ifndef APP_PATTERNS_H
#define APP_PATTERNS_H

// Patterns triggered by the ESP32 web app — separate from the operator/button
// bank. The IDs below are the single source of truth: both runPattern() in
// main.cpp and the named-command parser in webcontrol.cpp reference these,
// and esp32/data/questions.json holds the numeric values for the JSON pool.
//
// All functions write into the global `leds[]` (from led.h) and call
// FastLED.show() before returning.

namespace app_patterns {

// Fire-ritual finale
constexpr int DIM_AMBIENT  = 8;
constexpr int FIRE_FULL    = 9;
constexpr int IDLE_AMBIENT = 10;

// Question reactions (esp32/data/questions.json)
constexpr int WORM_YES = 11;
constexpr int WORM_NO  = 12;
constexpr int SCRUNCH  = 13;
constexpr int FOLD     = 14;

// Per-strip wiring-direction calibration (bench tool, not a festival pattern).
// Lights every strip with a red→blue HSV gradient so you can read which
// physical end of each strip is the data-in. See `calibrate()` body for the
// detailed read-off instructions.
constexpr int CALIBRATE = 15;

// Painter question — "I'll paint you a picture in the style of…"
// Each pattern is a visual interpretation of the painter's mode, not just a
// palette grab. See CLAUDE/meta-sonic-flux-design.md and the function bodies.
constexpr int PAINT_MONET    = 16;
constexpr int PAINT_PICASSO  = 17;
constexpr int PAINT_DALI     = 18;
constexpr int PAINT_DA_VINCI = 19;

// "Could you keep it down there please?" → "What? Turn it up you say?"
// Full-structure strobe with per-strip random hues — sensory payoff for the
// joke. Index 20.
constexpr int TURN_IT_UP     = 20;

}  // namespace app_patterns

void dimAmbient();
void fireFull();
void idleAmbient();

void wormYes();
void wormNo();
void scrunch();
void fold();

void calibrate();

void paintMonet();
void paintPicasso();
void paintDali();
void paintDaliCascade();   // alt: descending-front variant of paintDali (test-only)
void paintDaVinci();

void turnItUp();

#endif
