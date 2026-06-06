#include "./spencer/spencer_patterns.h"
#include <FastLED.h>

extern CRGB leds[];
//extern const int NUM_LEDS;

#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 0

uint8_t starBrightness[NUM_LEDS];  // array to track each star's life
bool    starGrowing[NUM_LEDS];     // true = brightening, false = fading
uint8_t starTier[NUM_LEDS];        // 0 = normal, 1 = bright-tier (peak 255)

void spencerSparkle() {
  //darkness, random stars appear faintly,
  //grow brighter, new stars join,  others fade and vanish
  for (int i = 0; i < NUM_LEDS; i++) {
    if (starBrightness[i] == 0 && random8() < 5) {
      // New star begins faint
      starBrightness[i] = 1;
      starGrowing[i] = true;
    }

    // If star is growing, increase brightness
    if (starGrowing[i]) {
      starBrightness[i] += 4;
      if (starBrightness[i] >= MAX_BRIGHTNESS) {
        starBrightness[i] = MAX_BRIGHTNESS;
        starGrowing[i] = false;  // Start fading
      }
    } else if (starBrightness[i] > 0) {
      // If fading, decrease brightness
      starBrightness[i] -= 2;
      if (starBrightness[i] <= MIN_BRIGHTNESS) {
        starBrightness[i] = 0;
      }
    }

    // Set the LED to white based on its brightness
    leds[i] = CRGB(starBrightness[i], starBrightness[i], starBrightness[i]);
  }
  FastLED.show();
}

// Galaxy: sparse night sky. Two tiers of stars share the structure —
//   • a small handful (~GALAXY_BRIGHT_TARGET) at true max brightness, the
//     headline points of light the eye anchors on,
//   • a quieter background of normal stars at ~GALAXY_NORMAL_MAX.
// No spacing constraint — Poisson placement gives natural clusters and
// voids instead of the even-grid look. Birth rate kept low so the field
// reads sparse. Shares starBrightness/starGrowing with spencerSparkle;
// only one pattern renders at a time.
#define GALAXY_NORMAL_MAX    200
#define GALAXY_BRIGHT_MAX    255
#define GALAXY_BRIGHT_TARGET 10    // approx concurrent bright-tier stars

void galaxy() {
  // Count currently-lit bright-tier stars so we know whether to mint more.
  uint8_t brightCount = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    if (starBrightness[i] > 0 && starTier[i]) brightCount++;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    if (starBrightness[i] == 0 && random16() < 12) {
      // No neighbour rejection — random Poisson placement allows clusters
      // and voids, which reads as a real sky instead of fairy lights.
      starBrightness[i] = 1;
      starGrowing[i]    = true;
      // While under the bright-tier cap, any new birth gets promoted to
      // bright. Once the cap is full no more bright births until one fades.
      if (brightCount < GALAXY_BRIGHT_TARGET) {
        starTier[i] = 1;
        brightCount++;
      } else {
        starTier[i] = 0;
      }
    }

    if (starGrowing[i]) {
      uint8_t peak = starTier[i] ? GALAXY_BRIGHT_MAX : GALAXY_NORMAL_MAX;
      starBrightness[i] = qadd8(starBrightness[i], 1);
      if (starBrightness[i] >= peak) {
        starBrightness[i] = peak;
        starGrowing[i]    = false;
      }
    } else if (starBrightness[i] > 0) {
      starBrightness[i] -= 1;
      if (starBrightness[i] == 0) {
        starTier[i] = 0;     // release the bright slot when fully faded
      }
    }

    leds[i] = CRGB(starBrightness[i], starBrightness[i], starBrightness[i]);
  }
  FastLED.show();
}