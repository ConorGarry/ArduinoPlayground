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
  for(int i = 0; i < NUM_PINS; i++)
  {
    allStrips[i][random(NUM_LEDS_PER_STRIP)] = TwinkleColours[random(NUM_TWINKLE_COLOURS)];
  }
}

  unsigned long previousMillis = 0;           // Stores last time LEDs were updated
  int count = 0;                              // Stores count for incrementing up to the NUM_LEDs

//something wrong with this. Not working with sumulator anyway. Fails after about 20 loops
void comets() 
{  

// tried this too but copped the issue with the first one
	unsigned int interval = random(2000, 8000);
  int randomStrip = random(1, NUM_PINS);
	
  unsigned long currentMillis = millis();   // Get the time
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;         // Save the last time the LEDs were updated
    count = 0;                              // Reset the count to 0 after each interval
  }
  for (int i = 0; i < 20; i++){
    if (count < NUM_LEDS_PER_STRIP) {     // Forward direction option for LEDs
      allStrips[randomStrip][count % (NUM_LEDS_PER_STRIP+1)].setRGB(255, 255, 255);    // Set LEDs with the color value
      count++;
    }
  }

  // fadeToBlackBy( allStrips[randomStrip], NUM_LEDS_PER_STRIP, 15);                 // Fade the tail LEDs to black
  for (int j = 0; j < NUM_LEDS_PER_STRIP; j++)
    if (random(2) == 1)
        allStrips[randomStrip][j] = allStrips[randomStrip][j].fadeToBlackBy(128);  
  FastLED.show();
  delay(random(5, 40));                                      // Delay to set the speed of the animation

}

void Fire2012()
{
  bool gReverseDirection = false;
// Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS_PER_STRIP];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS_PER_STRIP) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS_PER_STRIP - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS_PER_STRIP-1) - j;
      } else {
        pixelnumber = j;
      }
      for (int k = 0; k < NUM_VERTICAL_STRIPS; k++)
        allVerticals[k][pixelnumber] = color;      
    }
}

void prettyNoise() {
  for (int i = 0; i < 6; i++) {
    fill_noise16(ledSegments[i], NUM_LEDS_PER_SEGMENT, 1, 0, 100, 1, 1, 50, millis() / 3, 5);    
  }    
}

void otherTwinkle() {
  EVERY_N_MILLISECONDS(75) {
    int randomLED = random16(0, NUM_LEDS_PER_SEGMENT -1);
    for (int i = 0; i < 6; i++)
      ledSegments[i][randomLED] = CRGB::LightGrey;

  }

  for (int i = 0; i < 6; i++){
    for (int j = 0; j < NUM_LEDS_PER_SEGMENT; j++) {
      
      // Brightness
      uint8_t bNoise = inoise8(j * 80, millis());
      bNoise = constrain(bNoise, 50, 200);
      bNoise = map(bNoise, 50, 200, 20, 80);

      // Hue
      uint8_t hNoise = inoise8(j * 20, millis() / 5);
      hNoise = constrain(hNoise, 50, 200);
      hNoise = map(hNoise, 50, 200, 160, 192);
      
      if (leds1[j].g == 0) {        
        ledSegments[i][j] = CHSV(hNoise, 255, bNoise);
      }
    }
    fadeToBlackBy(ledSegments[i], NUM_LEDS_PER_SEGMENT, 5);
  }
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

// possibly could be brighter. wait and see
void pride() 
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;
 
  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);
  
  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;
  for (int j = 0; j < NUM_PINS; j++){
    for( uint16_t i = 0 ; i < NUM_LEDS_PER_STRIP; i++) {
      hue16 += hueinc16;
      uint8_t hue8 = hue16 / 256;

      brightnesstheta16  += brightnessthetainc16;
      uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

      uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
      uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
      bri8 += (255 - brightdepth);
      
      CRGB newcolor = CHSV( hue8, sat8, bri8);
      
      uint16_t pixelnumber = i;
      pixelnumber = (NUM_LEDS_PER_STRIP-1) - pixelnumber;
      
      nblend( allStrips[j][pixelnumber], newcolor, 64);
    }
  }
}
