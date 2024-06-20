#ifndef LED_H
#define LED_H


#include <FastLED.h>

#define RUNS 30
#define NUM_RUNS_PER_PENTAGON 5

#define NUM_LEDS 355
//#define NUM_LEDS 2130
#define NUM_LEDS_PER_STRIP 71
#define NUM_LEDS_PER_SEGMENT 355
#define NUM_PENTAGONS 12
#define NUM_PINS 6
#define NUM_VERTICAL_STRIPS 24
#define NUM_STRIPS 30
#define NUM_PATTERNS 9


extern CRGB leds1[]; // 1–5
extern CRGB leds2[]; // 6–10
extern CRGB leds3[]; // 11–15
extern CRGB leds4[]; // 16–20
extern CRGB leds5[]; // 21–25
extern CRGB leds6[]; // 26–30
extern CRGB *ledSegments[];

extern byte pinList[];
extern CRGB leds[];

#define RED 0xFF0000
#define GREEN 0x00FF00
#define BLUE 0x0000FF
#define YELLOW 0xFFFF00
#define PINK 0xFF1088
#define ORANGE 0xE05800
#define WHITE 0xFFFFFF

// Define the LED strip type
#define LED_TYPE WS2812B
// Define the color order of the LED strip.
#define COLOR_ORDER GRB

// Vertical Divisions
extern struct CRGB *bottom[];
extern struct CRGB *midBottom[];
extern struct CRGB *middle[];
extern struct CRGB *midTop[];
extern struct CRGB *top[];
extern struct CRGB *one[];
extern struct CRGB *two[];
extern struct CRGB *allVerticals[];

// All Strips
extern struct CRGB *allStrips[];

// Array of pentagon faces.
extern struct CRGB **pentagons[];

// Util functions
void fillRun(CRGB *run, CRGB color);
void fillPentagon(CRGB **pentagon, CRGB color);

#endif
