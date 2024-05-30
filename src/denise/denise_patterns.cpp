#include "./denise/denise_patterns.h"

void waveVerticalsUpwardseh(){  
  // Loop through each LED in the segment
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++)
  {
    // Calculate brightness using a sine wave
    uint8_t brightness = 255 * (0.5 + 0.5 * sin(2 * PI * (millis() / 1000.0) + i * PI / NUM_LEDS_PER_STRIP));
    upVerticals[0][i] = CRGB(brightness, 0, 0); // Red wave
    upVerticals[1][i] = CRGB(brightness, 0, 255); // Indigo wave      
    upVerticals[2][i] = CRGB(brightness, brightness, 0); // Yellow wave
    upVerticals[3][i] = CRGB(brightness, 215, 0); // Gold wave      
    upVerticals[4][i] = CRGB(0, 0, brightness); // Blue wave
    upVerticals[5][i] = CRGB(brightness, 0, brightness); // Violet wave
    upVerticals[6][i] = CRGB(brightness, 100, 0); // Orange wave
    upVerticals[7][i] = CRGB(brightness, brightness, brightness); // White wave
    upVerticals[8][i] = CRGB(0, brightness, 0); // Green wave
    upVerticals[9][i] = CRGB(brightness, brightness, brightness); // Silver wave
      
  }
  
  FastLED.show();  
}

void waveVerticalsUpwards()
{  
  // Loop through each LED in the segment
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++)
  {    
    // Calculate brightness using a sine wave
    uint8_t brightness = 255 * (0.5 + 0.5 * sin(2 * PI * (millis() / 1000.0) + i * PI / NUM_LEDS_PER_SEGMENT));

    // Set colors based on segment position
    if (i < (NUM_LEDS_PER_SEGMENT / 5))
    {
      leds2[i] = CRGB(brightness, 0, 0); // Red wave
      leds5[i] = CRGB(brightness, 0, 255); // Indigo wave      
    }
  }
  for (int i = NUM_LEDS_PER_SEGMENT -1; i >=(NUM_LEDS_PER_SEGMENT / 5); i--){
    uint8_t brightness = 255 * (0.5 + 0.5 * sin(2 * PI * (millis() / 1000.0) + i * PI / NUM_LEDS_PER_SEGMENT));
    if (i < (NUM_LEDS_PER_SEGMENT / 5) * 2 && i >= (NUM_LEDS_PER_SEGMENT / 5))
      {      
        leds3[i] = CRGB(brightness, brightness, 0); // Yellow wave
        leds6[i] = CRGB(brightness, 215, 0); // Gold wave      
      }
      else if (i < (NUM_LEDS_PER_SEGMENT / 5) * 3 && i >= (NUM_LEDS_PER_SEGMENT / 5) * 2)
      {      
        leds4[i] = CRGB(0, 0, brightness); // Blue wave
        leds5[i] = CRGB(brightness, 0, brightness); // Violet wave      
      }
      else if (i < (NUM_LEDS_PER_SEGMENT / 5) * 4 && i >= (NUM_LEDS_PER_SEGMENT / 5) * 3)
      {
        leds2[i] = CRGB(brightness, 100, 0); // Orange wave
        leds6[i] = CRGB(brightness, brightness, brightness); // White wave      
      }
      else
      {      
        leds3[i] = CRGB(0, brightness, 0); // Green wave
        leds5[i] = CRGB(brightness, brightness, brightness); // Silver wave      
      }
    }
  FastLED.show();  
}

CRGB nextTriColour(const CRGB currentColour)
{
  return (currentColour == CRGB::Green) ? CRGB::White : (currentColour == CRGB::White) ? CRGB::Orange : CRGB::Green;
}

// Animate a chase effect upwards on the pentagons
void triColourUp()
{  
  // Set up initial colours
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++)
  {
    leds1[i] = CRGB::Green;
    if (i < (NUM_LEDS_PER_SEGMENT / 5))
    {
      leds2[i] = CRGB::Green;
      leds3[i] = leds4[i] = leds5[i] = CRGB::White;
      leds6[i] = CRGB::Orange;
    }
    else if (i < (NUM_LEDS_PER_SEGMENT / 5) * 2)
    {
      leds2[i] = leds4[i] = leds6[i] = CRGB::White;
      leds3[i] = CRGB::Green;
      leds5[i] = CRGB::Orange;
    }
    else if (i < (NUM_LEDS_PER_SEGMENT / 5) * 3)
    {
      leds2[i] = leds3[i] = leds5[i] = CRGB::White;
      leds4[i] = CRGB::Green;
      leds6[i] = CRGB::Orange;
    }
    else if (i < (NUM_LEDS_PER_SEGMENT / 5) * 4)
    {
      leds2[i] = CRGB::Green;
      leds3[i] = leds4[i] = leds6[i] = CRGB::White;
      leds5[i] = CRGB::Orange;
    }
    else
    {
      leds2[i] = leds4[i] = leds5[i] = CRGB::White;
      leds3[i] = CRGB::Green;
      leds6[i] = CRGB::Orange;
    }
  }
  FastLED.show();
  delay(1000);  
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++)
  {
    leds1[i] = nextTriColour(leds1[i]);
    leds2[i] = nextTriColour(leds2[i]);
    leds3[i] = nextTriColour(leds3[i]);
    leds4[i] = nextTriColour(leds4[i]);
    leds5[i] = nextTriColour(leds5[i]);
    leds6[i] = nextTriColour(leds6[i]);
  }    
  FastLED.show();  
  delay(1000);
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++)
  {
    leds1[i] = nextTriColour(leds1[i]);
    leds2[i] = nextTriColour(leds2[i]);
    leds3[i] = nextTriColour(leds3[i]);
    leds4[i] = nextTriColour(leds4[i]);
    leds5[i] = nextTriColour(leds5[i]);
    leds6[i] = nextTriColour(leds6[i]);
  }    
  FastLED.show(); 
  delay(1000); /* frame rate */
}
