#include "./dee/dee_patterns.h"

void freePalestineFullBlink() {
  // Blink Palestinian Colours
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  delay(500);
   for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
   }
  FastLED.show();
  delay(500);
   for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
   }
  FastLED.show();
  delay(500);  
   for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Green; //20,153,84);//0x149954;
   }
  FastLED.show();
  delay(500);  
}

void freePalestineScan() {
  // Scan Palestinian Colours

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

void freePalestineStripScan() {
  // Scan along Each Strip and back again with changing Palestinian Colours
  // for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {      
  //  for (int j = 0; j < NUM_STRIPS; j++){
  //    allStrips[j][i] = CRGB::White;
  //    allStrips[j][i+1] = CRGB::White;
  //    allStrips[j][i+2] = CRGB::White;
  //    allStrips[j][i+3] = CRGB::White;
  //    allStrips[j][i+4] = CRGB::White;
  //    FastLED.show();
  //    allStrips[j][i] = CRGB::Black;
  //    }
  //   // delay(20);
  // }
  // FastLED.show();
  // delay(50);
  for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {      
    for (int j = 0; j < NUM_STRIPS; j++){
      allStrips[j][i] = CRGB::SpringGreen;
      allStrips[j][i+1] = CRGB::SpringGreen;
      allStrips[j][i+2] = CRGB::SpringGreen;
      allStrips[j][i+3] = CRGB::SpringGreen;
      allStrips[j][i+4] = CRGB::SpringGreen;
      FastLED.show();
      allStrips[j][i] = CRGB::Black;
     }
    // delay(20);
    }
  // FastLED.show();
  // delay(50);   
}

void showLights() {
    static byte j = 0;
    j+=4;
    byte k = j;

    // Roughly equivalent to fill_rainbow(g_LEDs, NUM_LEDS, j, 8);

    CRGB c;
    for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i ++)
    {
        leds[i] = c.setHue(k+=8);
        leds[i + NUM_LEDS_PER_SEGMENT] = c.setHue(k+=8);
        leds[i+NUM_LEDS_PER_SEGMENT * 2] = c.setHue(k+=8);
        leds[i+NUM_LEDS_PER_SEGMENT * 3] = c.setHue(k+=8);
        leds[i+NUM_LEDS_PER_SEGMENT * 4] = c.setHue(k+=8);
        leds[i+NUM_LEDS_PER_SEGMENT * 5] = c.setHue(k+=8);
    }
    static int scroll = 0;
    scroll++;
    FastLED.show();
    for (int i = scroll % 5; i < NUM_LEDS - 1; i += 5)
    {
        leds[i] = CRGB::Black;
        leds[i+NUM_LEDS_PER_SEGMENT] = CRGB::Black;
        leds[i+NUM_LEDS_PER_SEGMENT*2] = CRGB::Black;
        leds[i+NUM_LEDS_PER_SEGMENT*3] = CRGB::Black;
        leds[i+NUM_LEDS_PER_SEGMENT*4] = CRGB::Black;
        leds[i+NUM_LEDS_PER_SEGMENT*5] = CRGB::Black;
    }
    FastLED.show();
    delay(50);

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