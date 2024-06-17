#include "./denise/denise_patterns.h"

void twinkle()
{  
  static int passCount =  0;
  passCount++;

  if(passCount == NUM_LEDS_PER_SEGMENT)
  {
    passCount = 0;
    FastLED.clear(false);

  }
  // for(int i=0; i < NUM_LEDS_PER_STRIP; i++)
  // {
  for(int i = 0; i < NUM_STRIPS; i++)
  {
    allStrips[i][random(NUM_LEDS_PER_STRIP)] = TwinkleColours[random(NUM_TWINKLE_COLOURS)];
  }

  // const byte fadeAmt = 128;
  // const int cometSize = 5;
  // const int deltaHue  = 4;

  // static byte hue = HUE_RED;
  // static int iDirection = 1;
  // static int iPos = 0;

  // static int randomVerticalStrip = random(NUM_VERTICAL_STRIPS);
  // randomVerticalStrip--;

  // hue += deltaHue;

  // iPos += iDirection;
  // if (iPos == (NUM_LEDS - cometSize) || iPos == 0)
  //     iDirection *= -1;
  // for  (int j = 0; j<NUM_LEDS_PER_STRIP; j++)
  // {

  // }
  
  // for (int i = 0; i < cometSize; i++)
  //     allVerticals[randomVerticalStrip][iPos + i].setHue(hue);
  
  // // Randomly fade the LEDs
  // for (int j = 0; j < NUM_LEDS_PER_STRIP; j++)
  //     if (random(10) > 5)
  //         allVerticals[randomVerticalStrip][j] = allVerticals[randomVerticalStrip][j].fadeToBlackBy(fadeAmt);  

  // delay(50);
}

//something wrong with this. Not working with sumulator anyway. Fails after about 20 loops
void comets() 
{  
  const byte fadeAmt = 128;
  const int cometSize = 20;
  const int deltaHue  = 4;

  static byte hue = HUE_RED;
  static int iDirection = 1;
  static int iPos = 0;

  static int randomVerticalStrip = random(NUM_VERTICAL_STRIPS);
  randomVerticalStrip--;

  hue += deltaHue;

  iPos += iDirection;
  if (iPos == (NUM_LEDS - cometSize) || iPos == 0)
      iDirection *= -1;
  for  (int j = 0; j<NUM_LEDS_PER_STRIP; j++)
  {

  }
  
  for (int i = 0; i < cometSize; i++)
      allVerticals[randomVerticalStrip][iPos + i].setHue(hue);
  
  // Randomly fade the LEDs
  for (int j = 0; j < NUM_LEDS_PER_STRIP; j++)
      if (random(10) > 5)
          allVerticals[randomVerticalStrip][j] = allVerticals[randomVerticalStrip][j].fadeToBlackBy(fadeAmt);  

  delay(200);
}

void waveVerticalsOverwards()
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

CRGB nextUnicornColour(const CRGB currentColour)
{
  return (currentColour == CRGB::Purple) ? CRGB::BlueViolet : (currentColour == CRGB::BlueViolet) ? CRGB::Pink : CRGB::Purple;
}

// Animate a chase effect  on the pentagons
void unicornColourOver()
{  
  // Set up initial colours
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++)
  {
    leds1[i] = CRGB::Purple;
    if (i < (NUM_LEDS_PER_SEGMENT / 5))
    {
      leds2[i] = CRGB::Purple;
      leds3[i] = leds4[i] = leds5[i] = CRGB::BlueViolet;
      leds6[i] = CRGB::Pink;
    }
    else if (i < (NUM_LEDS_PER_SEGMENT / 5) * 2)
    {
      leds2[i] = leds4[i] = leds6[i] = CRGB::BlueViolet;
      leds3[i] = CRGB::Purple;
      leds5[i] = CRGB::Pink;
    }
    else if (i < (NUM_LEDS_PER_SEGMENT / 5) * 3)
    {
      leds2[i] = leds3[i] = leds5[i] = CRGB::BlueViolet;
      leds4[i] = CRGB::Purple;
      leds6[i] = CRGB::Pink;
    }
    else if (i < (NUM_LEDS_PER_SEGMENT / 5) * 4)
    {
      leds2[i] = CRGB::Purple;
      leds3[i] = leds4[i] = leds6[i] = CRGB::BlueViolet;
      leds5[i] = CRGB::Pink;
    }
    else
    {
      leds2[i] = leds4[i] = leds5[i] = CRGB::BlueViolet;
      leds3[i] = CRGB::Purple;
      leds6[i] = CRGB::Pink;
    }
  }
  FastLED.show();
  delay(1000);  
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++)
  {
    leds1[i] = nextUnicornColour(leds1[i]);
    leds2[i] = nextUnicornColour(leds2[i]);
    leds3[i] = nextUnicornColour(leds3[i]);
    leds4[i] = nextUnicornColour(leds4[i]);
    leds5[i] = nextUnicornColour(leds5[i]);
    leds6[i] = nextUnicornColour(leds6[i]);
  }    
  FastLED.show();  
  delay(1000);
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++)
  {
    leds1[i] = nextUnicornColour(leds1[i]);
    leds2[i] = nextUnicornColour(leds2[i]);
    leds3[i] = nextUnicornColour(leds3[i]);
    leds4[i] = nextUnicornColour(leds4[i]);
    leds5[i] = nextUnicornColour(leds5[i]);
    leds6[i] = nextUnicornColour(leds6[i]);
  }    
  FastLED.show(); 
  delay(1000); /* frame rate */
}
