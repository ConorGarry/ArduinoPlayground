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

// --- Painter patterns (indices 16–19) ------------------------------------
//
// "I'll paint you a picture in the style of…" — four LED interpretations of
// painter modes. Triggered by a `type:choice` question in questions.json.

// Monet — water lilies. Pond foliage, lily blooms, reflected sky, dabs of
// warm gold where the sun hits the surface. The painterly softness is baked
// into the palette itself (pre-mixed pastels rather than saturated CHSV), so
// the noise field can wander across the whole water-lily colour story
// without ever going neon. 2-D Perlin noise drifts the field slowly across
// the structure with no hard edges; brightness pulses on a 10 BPM breath.
void paintMonet() {
  // Saturated water-lily palette. Pastel RGB values like (200,240,180) look
  // painterly on a screen but read as WHITE on an LED — additive light has
  // no "soft", so every channel near-equal = white. Each stop below has one
  // channel dominant so it actually renders as its colour. The Monet feel
  // comes from a low brightness breath (below), not a low-saturation palette.
  static const CRGBPalette16 lilies = CRGBPalette16(
    CRGB(  0, 200,  50),    // pond green
    CRGB( 50, 230,  80),    // leaf green
    CRGB(180, 255,  40),    // chartreuse
    CRGB(255, 200,  30),    // sun gold
    CRGB(255, 140,  50),    // peach
    CRGB(255,  60, 130),    // hot lily pink
    CRGB(255,  30, 180),    // bright pink
    CRGB(220,  50, 220),    // magenta bloom
    CRGB(180,  60, 240),    // pink-violet
    CRGB(140, 100, 240),    // soft violet (only "blue-ish" stop)
    CRGB( 50, 200, 220),    // teal-cyan sky reflection
    CRGB( 30, 220, 180),    // teal pond
    CRGB( 50, 255, 150),    // mint
    CRGB(200, 255,  80),    // pale spring green
    CRGB(255, 180,  60),    // golden hour
    CRGB(255,  90, 110)     // coral lily
  );

  static uint16_t t           = 0;
  static uint16_t paletteSpin = 0;
  EVERY_N_MILLISECONDS(40)  { t += 3; }            // slow weather
  EVERY_N_MILLISECONDS(120) { paletteSpin += 1; }  // very slow palette drift

  // Soft brightness — the painterly hush. At 255 a saturated palette reads
  // as nightclub; at 80..160 it reads as tinted gallery light.
  uint8_t breath = beatsin8(10, 80, 160);

  for (int s = 0; s < NUM_STRIPS; s++) {
    CRGB* strip = allStrips[s];
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      // Two-octave noise — coarse blobs (i*30) plus a finer wobble for the
      // painterly bleed where blobs touch.
      uint8_t coarse = inoise8(s * 80 + i * 30, t);
      uint8_t fine   = inoise8(s * 80 + i * 90 + 9999, t + 7777);
      // Stretch coarse from inoise8's natural ~60..200 bell into full 0..255.
      // Without this, every strip clusters around mid-palette and shows just
      // one or two colours; with it, each strip walks the whole palette.
      uint8_t spread = qadd8(qsub8(coarse, 60), qsub8(coarse, 60));
      uint8_t paletteIdx = spread + (fine >> 3) + (uint8_t)paletteSpin;
      strip[i] = ColorFromPalette(lilies, paletteIdx, breath, LINEARBLEND);
    }
    // Gentle blur — just enough that adjacent colours bleed into each other
    // instead of meeting on a hard line. Painterly merge.
    blur1d(strip, NUM_LEDS_PER_STRIP, 40);
  }
  FastLED.setBrightness(255);
  FastLED.show();
}

// Picasso — cubism, fractured across thirty faces.
// Every strip is its own painting. Each holds one of five bold primaries;
// every 600–900 ms a random subset snaps to a new colour. Some strips run
// their colour as a hard-edged travelling bar against near-black — multiple
// perspectives on the same object. Disagreement is the point.
void paintPicasso() {
  static const CRGB PALETTE[5] = {
    CRGB(230,  57,  70),   // cadmium red
    CRGB(241, 196,  15),   // chrome yellow
    CRGB( 30, 111, 186),   // cobalt blue
    CRGB(  5,   5,   7),   // near-black
    CRGB(242, 239, 233),   // off-white
  };

  static CRGB    stripColor[NUM_STRIPS];
  static int8_t  stripDir[NUM_STRIPS];           // -1 / 0 / +1
  static bool    initted = false;
  static unsigned long nextCutMs   = 0;
  static unsigned long lastCallMs  = 0;
  static uint8_t sweepPos = 0;

  unsigned long now = millis();

  // Re-seed on (re)entry to the pattern so it doesn't look static after a
  // long absence.
  if (!initted || now - lastCallMs > 500) {
    for (int s = 0; s < NUM_STRIPS; s++) {
      stripColor[s] = PALETTE[random8(5)];
      stripDir[s]   = (int8_t) random8(3) - 1;   // -1, 0, +1
    }
    initted   = true;
    nextCutMs = now + 700;
  }
  lastCallMs = now;

  // Snap a random subset to new colours on each cut.
  if (now >= nextCutMs) {
    uint8_t n = 5 + random8(8);                   // 5..12 strips change
    for (uint8_t k = 0; k < n; k++) {
      uint8_t s = random8(NUM_STRIPS);
      stripColor[s] = PALETTE[random8(5)];
      stripDir[s]   = (int8_t) random8(3) - 1;
    }
    nextCutMs = now + 600 + random16(300);        // 600..899 ms cadence
  }

  EVERY_N_MILLISECONDS(35) { sweepPos++; }

  for (int s = 0; s < NUM_STRIPS; s++) {
    CRGB* strip = allStrips[s];
    if (stripDir[s] == 0) {
      fill_solid(strip, NUM_LEDS_PER_STRIP, stripColor[s]);
      continue;
    }
    // Travelling bar against near-black contrast.
    uint8_t pos = stripDir[s] > 0 ? sweepPos : (uint8_t)(255 - sweepPos);
    int leadLed = (pos * NUM_LEDS_PER_STRIP) >> 8;
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      int dist = abs(i - leadLed);
      strip[i] = (dist < 7) ? stripColor[s] : PALETTE[3];
    }
  }
  FastLED.setBrightness(255);
  FastLED.show();
}

// Dalí — the persistence of memory, melting in technicolour.
// Neon droplets form at the physical top of each strip and fall, leaving
// tapered wet streaks behind. Colours are pure neon (CHSV at full sat/val
// from a hand-picked vivid palette) so the whole installation reads as a
// fluorescent-paint chandelier dripping onto itself. Drips that collide
// brighten into hotspots; the slow background fade leaves a few seconds of
// streak history before the next drip overpaints it.
//
// Inspired by github.com/drsteveplatt/dripper and ctkjedi/IcicleDrips —
// same physics (falling head + fading trail) scaled to 30 strips with vivid
// hues instead of the muted melt-wax that was here before.
//
// FastLED helpers doing the heavy lifting:
//   fadeToBlackBy            — leaves the trail behind each moving head
//   CHSV (full sat + val)    — vivid neon colour, no muddy in-between hues
//   random8 / random16       — drip choice, spawn cadence
void paintDali() {
  // Per-strip data-flow direction. Mirrored from fireFull()'s calibrated
  // table — keep them in sync if Spencer flips any strip there. true = data
  // input at the physical TOP of the run, so "fall from top" means
  // increasing LED index → 0; false = the opposite.
  static const bool stripWiredFromTop[NUM_STRIPS + 1] = {
    /* [0] unused */ false,
    /*  1 */ false, /*  2 */ false, /*  3 */ true,  /*  4 */ true,  /*  5 */ true,
    /*  6 */ false, /*  7 */ true,  /*  8 */ true,  /*  9 */ true,  /* 10 */ true,
    /* 11 */ true,  /* 12 */ true,  /* 13 */ true,  /* 14 */ true,  /* 15 */ true,
    /* 16 */ true,  /* 17 */ true,  /* 18 */ true,  /* 19 */ true,  /* 20 */ true,
    /* 21 */ true,  /* 22 */ true,  /* 23 */ true,  /* 24 */ true,  /* 25 */ true,
    /* 26 */ false, /* 27 */ true,  /* 28 */ true,  /* 29 */ true,  /* 30 */ true,
  };

  // Vivid neon hue pool. Hand-picked so every drip lands on a punchy
  // fluorescent colour — no in-between muddy yellow-greens or murky browns
  // that random8() across the whole HSV wheel would produce.
  //   0   = pure red          224 = hot pink
  //   24  = neon orange       208 = electric magenta
  //   64  = lime              192 = neon purple
  //   96  = neon green        144 = cyan
  //   160 = electric blue
  static const uint8_t NEON_HUES[] = {
    0, 24, 64, 96, 144, 160, 192, 208, 224
  };
  static const uint8_t NUM_HUES = sizeof(NEON_HUES) / sizeof(NEON_HUES[0]);

  // Drip pool. Sized for a wet, busy installation — 120 simultaneous drips
  // means roughly 4 drips in flight per strip on average, which reads as
  // continuous dripping rather than occasional drops. State stored in fixed
  // arrays of POD types so this never touches the heap. ~8 bytes per drip.
  static const int MAX_DRIPS = 120;
  static int16_t  dStrip[MAX_DRIPS];     // 0..NUM_STRIPS-1, or -1 = inactive
  static uint16_t dHeadQ8[MAX_DRIPS];    // head pos along strip (Q8.8 fixed-point)
  static uint16_t dVelQ8[MAX_DRIPS];     // LEDs per frame * 256
  static uint8_t  dHue[MAX_DRIPS];
  static uint8_t  dTailLen[MAX_DRIPS];   // 6..22 LEDs
  static bool initted = false;

  static unsigned long lastSpawnMs = 0;
  static unsigned long lastCallMs  = 0;
  unsigned long now = millis();

  // Reset pool on (re)entry so the pattern always starts clean rather than
  // resuming with stale drips frozen mid-fall after the override expired.
  if (!initted || now - lastCallMs > 500) {
    for (int i = 0; i < MAX_DRIPS; i++) dStrip[i] = -1;
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    initted = true;
  }
  lastCallMs = now;

  // Background fade — leaves the wet streak. 10/255 ≈ 2.5 s half-life at
  // ~60 fps, long enough that the slow drips' trails linger as syrupy
  // streaks behind the heads.
  fadeToBlackBy(leds, NUM_LEDS, 10);

  // Spawn cadence: small burst every 55 ms = ~30 drips/sec across the
  // structure. Drips spawn closer together in time so the streaks pack
  // more densely on each strip without losing the ooze feeling.
  if (now - lastSpawnMs >= 55) {
    lastSpawnMs = now;
    uint8_t toSpawn = 1 + random8(2);   // 1..2 per tick
    for (uint8_t sp = 0; sp < toSpawn; sp++) {
      for (int i = 0; i < MAX_DRIPS; i++) {
        if (dStrip[i] < 0) {
          dStrip[i]   = random8(NUM_STRIPS);
          dHeadQ8[i]  = 0;
          // 14..64 → 0.05..0.25 LEDs/frame → a drip crosses 71 LEDs in
          // ~285..1300 frames (~5..21 s at 60 fps). Honey-slow ooze.
          dVelQ8[i]   = 14 + random8(50);
          dHue[i]     = NEON_HUES[random8(NUM_HUES)];
          dTailLen[i] = 8 + random8(17);
          break;
        }
      }
    }
  }

  // Advance + paint each live drip.
  for (int d = 0; d < MAX_DRIPS; d++) {
    if (dStrip[d] < 0) continue;
    dHeadQ8[d] += dVelQ8[d];
    int headFromTop = dHeadQ8[d] >> 8;
    if (headFromTop >= NUM_LEDS_PER_STRIP) {
      dStrip[d] = -1;
      continue;
    }

    CRGB*   strip   = allStrips[dStrip[d]];
    bool    fromTop = stripWiredFromTop[dStrip[d] + 1];
    uint8_t hue     = dHue[d];
    uint8_t tail    = dTailLen[d];

    // Draw head + tapering tail. Brightness ramps from 255 at the head to
    // 0 at the tail end so each drip looks like a wet droplet with a comet
    // streak above it. Additive write so two drips meeting brighten into
    // a saturated hotspot rather than overwriting each other.
    for (int t = 0; t < tail; t++) {
      int posFromTop = headFromTop - t;
      if (posFromTop < 0) break;
      int pix = fromTop ? posFromTop
                        : (NUM_LEDS_PER_STRIP - 1 - posFromTop);
      uint8_t bri = (uint8_t)(255 - ((uint16_t)t * 255 / tail));
      strip[pix] += CHSV(hue, 255, bri);
    }
  }

  FastLED.setBrightness(255);
  FastLED.show();
}

// Dalí — cascade variant. Same vivid neon drip pool as paintDali, but with a
// descending activation front that's the geometric inverse of fireFull(): the
// top height band starts dripping first, then mid-top, then middle, …, until
// the bottom band joins. Spencer wanted to A/B this against the
// all-strips-drip-from-time-zero original — flip runPattern()'s PAINT_DALI
// case between paintDali() and paintDaliCascade() to compare. Once the front
// reaches the bottom (after CASCADE_MS), it's visually identical to
// paintDali() — the difference lives in the opening cascade.
void paintDaliCascade() {
  // --- shared with paintDali() ------------------------------------------
  static const bool stripWiredFromTop[NUM_STRIPS + 1] = {
    /* [0] unused */ false,
    /*  1 */ false, /*  2 */ false, /*  3 */ true,  /*  4 */ true,  /*  5 */ true,
    /*  6 */ false, /*  7 */ true,  /*  8 */ true,  /*  9 */ true,  /* 10 */ true,
    /* 11 */ true,  /* 12 */ true,  /* 13 */ true,  /* 14 */ true,  /* 15 */ true,
    /* 16 */ true,  /* 17 */ true,  /* 18 */ true,  /* 19 */ true,  /* 20 */ true,
    /* 21 */ true,  /* 22 */ true,  /* 23 */ true,  /* 24 */ true,  /* 25 */ true,
    /* 26 */ false, /* 27 */ true,  /* 28 */ true,  /* 29 */ true,  /* 30 */ true,
  };
  static const uint8_t NEON_HUES[] = { 0, 24, 64, 96, 144, 160, 192, 208, 224 };
  static const uint8_t NUM_HUES = sizeof(NEON_HUES) / sizeof(NEON_HUES[0]);

  // --- band model mirrored from fireFull() ------------------------------
  // 0 = bottom band, 4 = top band. The 6 non-vertical strips inherit the
  // same middle (2) default fireFull() uses.
  static const uint8_t stripBand[NUM_STRIPS + 1] = {
    /* [0] unused */ 0,
    /*  1.. 6 */ 0, 1, 2, 1, 0, 0,
    /*  7..12 */ 1, 2, 1, 2, 2, 2,
    /* 13..18 */ 3, 3, 2, 2, 2, 1,
    /* 19..24 */ 2, 1, 2, 2, 2, 3,
    /* 25..30 */ 3, 4, 4, 4, 3, 3,
  };
  static const unsigned long CASCADE_MS = 4000;  // top -> bottom front travel
  static const uint8_t NUM_BANDS = 5;

  // --- drip pool (same shape + sizing as paintDali) ---------------------
  static const int MAX_DRIPS = 120;
  static int16_t  dStrip[MAX_DRIPS];
  static uint16_t dHeadQ8[MAX_DRIPS];
  static uint16_t dVelQ8[MAX_DRIPS];
  static uint8_t  dHue[MAX_DRIPS];
  static uint8_t  dTailLen[MAX_DRIPS];
  static bool initted = false;

  static unsigned long startMs     = 0;
  static unsigned long lastSpawnMs = 0;
  static unsigned long lastCallMs  = 0;
  unsigned long now = millis();

  // (Re)entry detection — also restarts the cascade so the opening drama is
  // visible every time the operator (or web app) lands on this pattern.
  if (!initted || now - lastCallMs > 500) {
    for (int i = 0; i < MAX_DRIPS; i++) dStrip[i] = -1;
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    startMs = now;
    initted = true;
  }
  lastCallMs = now;

  // Same syrupy fade as paintDali so trails persist behind each drip.
  fadeToBlackBy(leds, NUM_LEDS, 10);

  // Descending cascade front. Same math as fireFull()'s `front`, but used in
  // reverse: a strip's band b is "live" once front > (NUM_BANDS - 1 - b),
  // i.e. the top band (b=4) goes live immediately, the bottom (b=0) goes
  // live just as the front reaches NUM_BANDS.
  unsigned long elapsed = now - startMs;
  float front = (elapsed >= CASCADE_MS)
      ? (float) NUM_BANDS
      : (float) elapsed / CASCADE_MS * NUM_BANDS;

  if (now - lastSpawnMs >= 55) {
    lastSpawnMs = now;
    uint8_t toSpawn = 1 + random8(2);
    for (uint8_t sp = 0; sp < toSpawn; sp++) {
      // Try up to 6 random strips before giving up — early in the cascade
      // only the top few bands are live, so most random picks miss.
      for (uint8_t attempt = 0; attempt < 6; attempt++) {
        uint8_t candidate = random8(NUM_STRIPS);
        uint8_t band = stripBand[candidate + 1];   // table is 1-based
        if (front <= (float)(NUM_BANDS - 1 - band)) continue;   // not yet
        for (int i = 0; i < MAX_DRIPS; i++) {
          if (dStrip[i] < 0) {
            dStrip[i]   = candidate;
            dHeadQ8[i]  = 0;
            dVelQ8[i]   = 14 + random8(50);
            dHue[i]     = NEON_HUES[random8(NUM_HUES)];
            dTailLen[i] = 8 + random8(17);
            break;
          }
        }
        break;
      }
    }
  }

  // Per-drip advance + paint — identical to paintDali().
  for (int d = 0; d < MAX_DRIPS; d++) {
    if (dStrip[d] < 0) continue;
    dHeadQ8[d] += dVelQ8[d];
    int headFromTop = dHeadQ8[d] >> 8;
    if (headFromTop >= NUM_LEDS_PER_STRIP) {
      dStrip[d] = -1;
      continue;
    }

    CRGB*   strip   = allStrips[dStrip[d]];
    bool    fromTop = stripWiredFromTop[dStrip[d] + 1];
    uint8_t hue     = dHue[d];
    uint8_t tail    = dTailLen[d];

    for (int t = 0; t < tail; t++) {
      int posFromTop = headFromTop - t;
      if (posFromTop < 0) break;
      int pix = fromTop ? posFromTop
                        : (NUM_LEDS_PER_STRIP - 1 - posFromTop);
      uint8_t bri = (uint8_t)(255 - ((uint16_t)t * 255 / tail));
      strip[pix] += CHSV(hue, 255, bri);
    }
  }

  FastLED.setBrightness(255);
  FastLED.show();
}

// Da Vinci — sfumato, candlelit study.
// A single bright pinpoint sweeps the structure pixel-by-pixel in chain order
// (LED 0 of strip 0 → end of strip 29) over PASS_MS. Behind it, a long sfumato
// tail in candle-warm tones fades through cream → gold → burnt umber → shadow.
// No hard edges. Brief total-shadow rest between passes.
void paintDaVinci() {
  static const unsigned long PASS_MS = 30000;
  static const unsigned long REST_MS = 4000;
  static const int           TRAIL_LEN = 240;

  static unsigned long startMs    = 0;
  static unsigned long lastCallMs = 0;

  unsigned long now = millis();
  if (now - lastCallMs > 500) startMs = now;
  lastCallMs = now;

  unsigned long elapsed = (now - startMs) % (PASS_MS + REST_MS);

  if (elapsed >= PASS_MS) {
    // Rest — fade what's there to pure shadow.
    fadeToBlackBy(leds, NUM_LEDS, 4);
    FastLED.setBrightness(255);
    FastLED.show();
    return;
  }

  // Clear and redraw fresh each frame — the trail is computed, not faded.
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  long leadIdx = (long) elapsed * NUM_LEDS / PASS_MS;

  for (int t = 0; t < TRAIL_LEN; t++) {
    long idx = leadIdx - t;
    if (idx < 0) break;

    // Brightness falls linearly along the trail.
    uint8_t bri = (uint8_t)(255 - ((long) t * 255 / TRAIL_LEN));

    CRGB c;
    if (t < 2) {
      // Pinpoint highlight — near-white candlewick.
      c = CRGB(255, 250, 240);
    } else if (t < 30) {
      // Cream → gold.
      c = CRGB((201 * bri) >> 8, (169 * bri) >> 8, (97 * bri) >> 8);
    } else if (t < 110) {
      // Burnt amber.
      c = CRGB((139 * bri) >> 8, ( 90 * bri) >> 8, (43 * bri) >> 8);
    } else {
      // Deep umber → shadow.
      c = CRGB(( 61 * bri) >> 8, ( 40 * bri) >> 8, (23 * bri) >> 8);
    }
    leds[idx] = c;
  }

  FastLED.setBrightness(255);
  FastLED.show();
}

// --- Turn it up (index 20) -----------------------------------------------
//
// Solid white strobe response to "Could you keep it down there please?".
// Toggles every ~60 ms between an all-black blank frame and the full
// structure in pure white — ~8 Hz, properly disruptive, brief by design
// (a question reaction, not a sustained mode).
void turnItUp() {
  static unsigned long lastSwapMs = 0;
  static bool          on         = false;

  unsigned long now = millis();
  if (now - lastSwapMs >= 60) {
    on         = !on;
    lastSwapMs = now;
  }

  fill_solid(leds, NUM_LEDS, on ? CRGB::White : CRGB::Black);
  FastLED.setBrightness(255);
  FastLED.show();
}
