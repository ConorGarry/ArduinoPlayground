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
  for(int i = 0; i < NUM_STRIPS; i++)
  {
    allStrips[i][random(NUM_LEDS_PER_STRIP)] = TwinkleColours[random(NUM_TWINKLE_COLOURS)];
  }
  delay(200);
}

// for comets
unsigned long previousMillis = 0;           // Stores last time LEDs were updated
int cometCount = 0;                         // Stores count for incrementing up to the NUM_LEDs

void comets() 
{  

// tried this too but copped the issue with the first one
	unsigned int interval = random(2000, 8000);
  int randomStrip = random(1, NUM_STRIPS);
  int cometLength = 20;
	
  unsigned long currentMillis = millis();   // Get the time
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;         // Save the last time the LEDs were updated
    cometCount = 0;                              // Reset the count to 0 after each interval
  }
  for (int i = 0; i < cometLength; i++){
    if (cometCount < NUM_LEDS_PER_STRIP) {     // Forward direction option for LEDs
      allStrips[randomStrip][cometCount % (NUM_LEDS_PER_STRIP+1)].setRGB(255, 255, 255);    // Set LEDs with the color value
      cometCount++;
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
  // for (int i = 0; i < 6; i++) {
    // fill_noise16(ledSegments[i], NUM_LEDS_PER_SEGMENT, 1, 0, 100, 1, 1, 50, millis() / 3, 5);    
    fill_noise16(leds, NUM_LEDS * NUM_PINS, 1, 0, 100, 1, 1, 50, millis() / 3, 5);    
  // }    
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

// might be shite but see what it looks like I suppose
void waveVerticalsOverwards()
{  
  // Loop through each LED in the segment
  for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
  {    
    // Calculate brightness using a sine wave
    uint8_t brightness = 255 * (0.5 + 0.5 * sin(2 * PI * (millis() / 1000.0) + i * PI / NUM_LEDS_PER_STRIP));

    // Set colors based on segment position
      allStrips[5][i] = CRGB(brightness, 0, 0); // Red wave //6
      allStrips[20][i] = CRGB(brightness, 0, 255); // Indigo wave   //21   

  }
  for (int i = NUM_LEDS_PER_STRIP -1; i >=0; i--){
    uint8_t brightness = 255 * (0.5 + 0.5 * sin(2 * PI * (millis() / 1000.0) + i * PI / NUM_LEDS_PER_STRIP));
          
        allStrips[11][i] = CRGB(brightness, brightness, 0); // Yellow wave //12
        allStrips[26][i] = CRGB(brightness, 215, 0); // Gold wave      //27
            
        allStrips[17][i] = CRGB(0, 0, brightness); // Blue wave //18
        allStrips[22][i] = CRGB(brightness, 0, brightness); // Violet wave    //23  
      
        allStrips[8][i] = CRGB(brightness, 100, 0); // Orange wave //9
        allStrips[28][i] = CRGB(brightness, brightness, brightness); // White wave      //29
          
        allStrips[14][i] = CRGB(0, brightness, 0); // Green wave //15
        allStrips[24][i] = CRGB(brightness, brightness, brightness); // Silver wave      //25
     
    }
  FastLED.show();  
}

CRGB nextUnicornColour(const CRGB currentColour)
{
  return (currentColour == CRGB::Purple) ? CRGB::BlueViolet : (currentColour == CRGB::BlueViolet) ? CRGB::Pink : CRGB::Purple;
}

// Animate a chase effect  on the pentagons
// Bad code but in case we're desperate or it happens to actually look good
void unicornColourOver()
{  
  // Set up initial colours
  for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
  {
    allStrips[0][i] = allStrips[5][i]  = allStrips[11][i] = allStrips[17][i] = allStrips[8][i] = allStrips[14][i] = CRGB::Purple; //1, 6, 12, 18, 9, 15
    // allStrips[6][i] = allStrips[7][i]  = allStrips[9][i] = allStrips[10][i] = allStrips[12][i] = allStrips[13][i] = allStrips[15][i] = allStrips[16][i] = 
    //   allStrips[18][i] = allStrips[19][i] = allStrips[20][i] = allStrips[22][i] = allStrips[24][i] = allStrips[26][i] = allStrips[28][i] = CRGB::Purple; //7, 8, 10, 11, 13, 14, 16, 17, 19, 10, 21, 23, 25, 27, 28
    for (int j = 6; j < 29; j++){
      if(j != 8 && j != 11 && j != 14 && j != 17 && j != 21 && j != 23 && j != 25 && j != 27)
        allStrips[j][i] = CRGB::Purple;
    }    
    allStrips[25][i] = allStrips[21][i]  = allStrips[27][i] = allStrips[23][i] = allStrips[29][i] = CRGB::Pink; //26, 22, 28, 24, 30  
  }
  FastLED.show();
  delay(1000);
  for (int i = 0; i < NUM_STRIPS; i++){
    for (int j = 0; j < NUM_LEDS_PER_SEGMENT; j++)
    {
      allStrips[i][j] = nextUnicornColour(allStrips[i][j]);
    }   
  } 
  FastLED.show();  
  delay(1000);
  for (int i = 0; i < NUM_STRIPS; i++){
    for (int j = 0; j < NUM_LEDS_PER_SEGMENT; j++)
    {
      allStrips[i][j] = nextUnicornColour(allStrips[i][j]);
    }   
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
  for (int j = 0; j < NUM_STRIPS; j++){
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
