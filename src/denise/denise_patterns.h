#ifndef DENISE_PATTERNS_H
#define DENISE_PATTERNS_H

#include <FastLED.h>
#include "led.h"

#define NUM_TWINKLE_COLOURS 4

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 60

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

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
void Fire2012();
void prettyNoise();
void otherTwinkle();
void pride();

#endif