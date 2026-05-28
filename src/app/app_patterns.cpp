#include <FastLED.h>
#include "app_patterns.h"
#include "../led.h"

static const CRGB WARM_WHITE = CRGB(255, 180, 100);

// --- Fire-ritual patterns (indices 8–10) ---------------------------------

// Dim warm ambient — "one more thing" transition before the fire choice.
// Roughly 20% of full brightness.
void dimAmbient() {
  fill_solid(leds, NUM_LEDS, WARM_WHITE);
  FastLED.setBrightness(51);
  FastLED.show();
}

// Full-installation fire that climbs from the base. The 30 strips are grouped
// into 5 height bands; within each band, the fire creeps pixel-by-pixel up
// each strip as the band's slice of CLIMB_MS elapses. After the climb the
// whole structure holds a roaring fire for the rest of the ritual.
//
// Per-strip direction model: each strip has a `stripWiredFromTop` flag that
// tells us which physical end its data input is on, so the climb is always
// from the real base upward regardless of how the strip was soldered into the
// chain. After the first bench test of the original "data-in at base" default
// many strips climbed downward — strong evidence that most strips on this
// assembly are wired data-in at the TOP. The default below is therefore
// "true" for every strip; flip any individual strip to false if it still
// climbs downward after this update.
//
// The led.cpp height-band helpers (bottom/midBottom/...) point at the dead
// leds1..6 buffers, so we carry our own strip->band table here and index
// allStrips[] (real CRGBSet views into leds[]). See
// CLAUDE/fire-ritual-base-climb-design.md.
void fireFull() {
  static const unsigned long CLIMB_MS  = 5000;  // base -> fully ablaze
  static const uint8_t       NUM_BANDS = 5;     // bottom..top

  // 1-based strip number -> height-band rank (0 = base ... 4 = top). The 6
  // non-vertical strips (3, 8, 11, 16, 19, 22) have no documented band; they
  // default to middle (2) so the whole shape ends up ablaze. Index [0] is
  // unused (strips are 1-based).
  static const uint8_t stripBand[NUM_STRIPS + 1] = {
    /* [0] unused */ 0,
    /*  1.. 6 */ 0, 1, 2, 1, 0, 0,
    /*  7..12 */ 1, 2, 1, 2, 2, 2,
    /* 13..18 */ 3, 3, 2, 2, 2, 1,
    /* 19..24 */ 2, 1, 2, 2, 2, 3,
    /* 25..30 */ 3, 4, 4, 4, 3, 3,
  };

  // 1-based per-strip data-flow direction.
  //   true  = data input is at the TOP of the physical run; the climb fills
  //           from high LED-index toward LED 0 (visually: bottom → top).
  //   false = data input is at the BASE of the run; the climb fills from
  //           LED 0 toward LED 71 (visually: bottom → top).
  //
  // Default for all 30 strips: true (inferred from the bench test of the
  // previous "all false except strip 1" default). Flip individual entries to
  // false here for any specific strip that climbs downward after testing.
  // SPENCER - fix these 
  static const bool stripWiredFromTop[NUM_STRIPS + 1] = {
    /* [0] unused */ false,
    /*  1 */ false, /*  2 */ false,  /*  3 */ true,  /*  4 */ true,  /*  5 */ true,
    /*  6 */ false, /*  7 */ true,  /*  8 */ true,  /*  9 */ true,  /* 10 */ true,
    /* 11 */ true,  /* 12 */ true,  /* 13 */ true,  /* 14 */ true,  /* 15 */ true,
    /* 16 */ true,  /* 17 */ true,  /* 18 */ true,  /* 19 */ true,  /* 20 */ true,
    /* 21 */ true,  /* 22 */ true,  /* 23 */ true,  /* 24 */ true,  /* 25 */ true,
    /* 26 */ false,  /* 27 */ true,  /* 28 */ true,  /* 29 */ true,  /* 30 */ true,
  };

  unsigned long now = millis();
  static unsigned long fireStartMs = 0;
  static unsigned long lastCallMs  = 0;
  static uint16_t      z           = 0;

  // A >250 ms gap since the last call means the ritual just (re)started — the
  // dispatcher calls this every loop tick (<1 ms) while the override is live.
  if (now - lastCallMs > 250) fireStartMs = now;
  lastCallMs = now;

  unsigned long elapsed = now - fireStartMs;
  float front = (elapsed >= CLIMB_MS)
      ? (float) NUM_BANDS
      : (float) elapsed / CLIMB_MS * NUM_BANDS;

  z += 35;  // animate the flame flicker

  for (int s = 1; s <= NUM_STRIPS; s++) {
    float fill = front - (float) stripBand[s];
    if (fill < 0.0f) fill = 0.0f;
    if (fill > 1.0f) fill = 1.0f;
    int litCount = (int) (fill * NUM_LEDS_PER_STRIP);

    CRGB* strip = allStrips[s - 1];
    bool fromTop = stripWiredFromTop[s];
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      // heightFromBase: 0 at the physical base of the strip, NUM_LEDS_PER_STRIP-1
      // at the top. Lit if the climb front has reached that height.
      int heightFromBase = fromTop ? (NUM_LEDS_PER_STRIP - 1 - i) : i;
      if (heightFromBase < litCount) {
        uint8_t n = inoise8(((s - 1) * NUM_LEDS_PER_STRIP + i) * 60, z);
        uint8_t r = n;
        uint8_t g = (n > 110) ? (uint8_t)((n - 110) * 2) : 0;
        strip[i] = CRGB(r, g, 0);
      } else {
        strip[i] = CRGB::Black;
      }
    }
  }

  FastLED.setBrightness(255);
  FastLED.show();
}

// Slow warm breathing post-ritual.
void idleAmbient() {
  // 4-second cycle, brightness ~60..123
  uint8_t b = 60 + (sin8(millis() / 16) >> 2);
  fill_solid(leds, NUM_LEDS, WARM_WHITE);
  FastLED.setBrightness(b);
  FastLED.show();
}

// --- Question-reaction patterns (indices 11–14) --------------------------

// Q1 (worm) yes — fast flowing rainbow. Distinct from rainbowChase via the
// finer hue step (3 vs 2) and tighter scroll cadence.
void wormYes() {
  static uint8_t hue = 0;
  fill_rainbow(leds, NUM_LEDS, hue, 3);
  EVERY_N_MILLISECONDS(15) { hue += 3; }
  FastLED.setBrightness(255);
  FastLED.show();
}

// Q1 (worm) no — full blackout. leds[] was cleared by selectMode() already;
// just push the zeros out.
void wormNo() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

// Q2 scrunch (no) — solid muddy brown.
void scrunch() {
  fill_solid(leds, NUM_LEDS, CRGB(80, 45, 15));
  FastLED.setBrightness(180);
  FastLED.show();
}

// Q2 fold (yes) — slow soft white breathing, ~10 BPM.
void fold() {
  uint8_t b = beatsin8(10, 60, 220);
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.setBrightness(b);
  FastLED.show();
}

// --- Calibration (index 15) ----------------------------------------------
//
// Lights every strip with a red→blue HSV gradient so you can read which
// physical end of each strip is the data-in (LED 0). Static — does not
// animate. Trigger it from a phone joined to the AP:
//
//     http://192.168.4.1/command?cmd=P:calibrate
//
// Looking at the dodecahedron:
//   - LED 0  of each strip is BRIGHT RED.
//   - LED 70 of each strip is BRIGHT BLUE.
//   - The gradient runs red → orange → yellow → green → cyan → blue.
//
// For each strip, note the position of the RED end on the physical structure
// and report back:
//   - RED at the BOTTOM  → stripWiredFromTop[s] should be FALSE
//   - RED at the TOP     → stripWiredFromTop[s] should be TRUE
// (Horizontal strips at the very top/bottom edges of the dodecahedron don't
// have a meaningful "up"; just note where red lands and we'll pick whichever.)
//
// After the read-off, this pattern won't be needed again unless the wiring
// changes.
void calibrate() {
  for (int s = 1; s <= NUM_STRIPS; s++) {
    CRGB* strip = allStrips[s - 1];
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      // Hue 0 (red) at LED 0, hue 170 (blue) at LED 70 — the rainbow path
      // 0..170 of 8-bit HSV avoids wrapping back into red.
      uint8_t hue = (uint8_t) ((long) i * 170L / (NUM_LEDS_PER_STRIP - 1));
      strip[i] = CHSV(hue, 255, 255);
    }
  }
  FastLED.setBrightness(255);
  FastLED.show();
}
