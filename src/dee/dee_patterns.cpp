#include "./dee/dee_patterns.h"

void freePalestineFullBlink() {
  // Blink Palestinian Colours
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] = CRGB::Red;
  }
  FastLED.show();
  delay(500);
   for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] =  CRGB::Black;
   }
  FastLED.show();
  delay(500);
   for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] =  CRGB::White;
   }
  FastLED.show();
  delay(500);  
   for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] =  CRGB::Green;//20,153,84);//0x149954;
   }
  FastLED.show();
  delay(500);  
}

void freePalestineScan() {
  // Blink Palestinian Colours
    for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
     leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] = CRGB::Red;
     leds1[i+1] = leds2[i+1] = leds3[i+1] = leds4[i+1] = leds5[i+1] = leds6[i+1] = CRGB::Red;
     leds1[i+2] = leds2[i+2] = leds3[i+2] = leds4[i+2] = leds5[i+2] = leds6[i+2] = CRGB::Red;
     FastLED.show();
     delay(20);
     leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] = CRGB::Black;
    }
  // FastLED.show();
  delay(500);
 
   for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] =  CRGB::White;
    leds1[i+1] = leds2[i+1] = leds3[i+1] = leds4[i+1] = leds5[i+1] = leds6[i+1] = CRGB::White;
    leds1[i+2] = leds2[i+2] = leds3[i+2] = leds4[i+2] = leds5[i+2] = leds6[i+2] = CRGB::White;
    FastLED.show();
    delay(20);
    leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] = CRGB::Black;
   }
  // FastLED.show();
  delay(500);  
   for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] =  CRGB::DarkGreen;
    leds1[i+1] = leds2[i+1] = leds3[i+1] = leds4[i+1] = leds5[i+1] = leds6[i+1] = CRGB::DarkGreen;
    leds1[i+2] = leds2[i+2] = leds3[i+2] = leds4[i+2] = leds5[i+2] = leds6[i+2] = CRGB::DarkGreen;
    FastLED.show();
    delay(20);
    leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] = CRGB::Black;
   }
  // FastLED.show();
  delay(500);  
}

void freePalestineChase() {
  // Chase Palestinian Colours back and forth on full segment 
  // TO DO: Change this to chase on each pentagon side instead of full segment
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] = CRGB::Red;
    FastLED.show();
  }

  for (int i = NUM_LEDS_PER_SEGMENT-1; i >=0; i--) {
    leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] =  CRGB::Black;
    FastLED.show();
 }

   for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] =  CRGB::White;
    FastLED.show();
   }

   for (int i = NUM_LEDS_PER_SEGMENT-1; i >=0; i--) {
    leds1[i] = leds2[i] = leds3[i] = leds4[i] = leds5[i] = leds6[i] =  CRGB::Green;
    FastLED.show();
   }
}