#ifndef LED_H
#define LED_H

#include <FastLED.h>

#define RUNS 20
#define NUM_LEDS 2160
#define NUM_LEDS_PER_STRIP 72
#define NUM_LEDS_PER_SEGMENT 360

extern CRGB leds1[]; // 1–5
extern CRGB leds2[]; // 6–10
extern CRGB leds3[]; // 11–15
extern CRGB leds4[]; // 16–20
extern CRGB leds5[]; // 21–25
extern CRGB leds6[]; // 26–30

// Define the LED strip type
#define LED_TYPE WS2812B
// Define the color order of the LED strip.
#define COLOR_ORDER RGB

// Divisions
extern struct CRGB *bottom[];
extern struct CRGB *midBottom[];
extern struct CRGB *middle[];
extern struct CRGB *midTop[];
extern struct CRGB *top[];

#endif
