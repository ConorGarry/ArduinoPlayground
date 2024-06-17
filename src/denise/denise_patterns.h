#ifndef DENISE_PATTERNS_H
#define DENISE_PATTERNS_H

#include <FastLED.h>
#include "led.h"

#define NUM_TWINKLE_COLOURS 4

static const CRGB TwinkleColours[] =
{
    CRGB::Blue,
    CRGB::Purple,
    CRGB::Pink,
    CRGB::Aquamarine
}; 

void waveVerticalsOverwards();
void unicornColourOver();
void twinkle();
void comets();

#endif