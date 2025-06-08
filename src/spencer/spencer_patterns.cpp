#include "./spencer/spencer_patterns.h"
#include <FastLED.h>

extern CRGB leds[];
 extern const int NUM_LEDS;

#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 0

uint8_t starBrightness[NUM_LEDS];  // array to track each star's life
bool starGrowing[NUM_LEDS];        // true = brightening, false = fading

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
}