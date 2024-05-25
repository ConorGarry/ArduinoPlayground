#include "./conor/conor_patterns.h"

void bottomChase() {
  int chaseLength = 32; // Number of LEDs in the chase
  int numLeds = NUM_LEDS_PER_STRIP;
  int trailLength = 4; // Length of the trailing tail

  // Iterate over each LED position in the strip
  for (int i = 0; i < numLeds; i++) {
    // Turn off all LEDs in the strip
    for (int j = 0; j < numLeds; j++) {
      bottom[0][j] = CRGB::Black;
      bottom[1][j] = CRGB::Black;
      bottom[2][j] = CRGB::Black;
    }

    // Turn on the chase LEDs with full intensity within the strip bounds
    for (int j = 0; j < chaseLength; j++) {
      int index = (i + j) % numLeds; // Calculate the index with wraparound
      bottom[0][index] = CRGB::Red;
      bottom[1][index] = CRGB::Green;
      bottom[2][index] = CRGB::Blue;
    }

    // Dim the trailing tail within the strip bounds
    for (int j = 0; j < trailLength; j++) {
      int index = (i - j - 1 + numLeds) % numLeds; // Calculate the index with wraparound
      bottom[0][index] = CRGB(255 - (trailLength - j) * 20, 0, 0);
      bottom[1][index] = CRGB(0, 255 - (trailLength - j) * 20, 0);
      bottom[2][index] = CRGB(0, 0, 255 - (trailLength - j) * 20);
    }

    FastLED.show();
  }
}

void midBottomChase() {
  int chaseLength = 32; // Number of LEDs in the chase
  int numLeds = NUM_LEDS_PER_STRIP;
  int trailLength = 4; // Length of the trailing tail

  // Loop the chase animation indefinitely
  for (int i = 0; i < numLeds; i++) {
    // Turn off all LEDs in the strip
    for (int j = 0; j < numLeds; j++) {
      midBottom[0][j] = CRGB::Black;
      midBottom[1][j] = CRGB::Black;
      midBottom[2][j] = CRGB::Black;
      midBottom[3][j] = CRGB::Black;
      midBottom[4][j] = CRGB::Black;
      midBottom[5][j] = CRGB::Black;
    }

    // Turn on the chase LEDs with full intensity within the strip bounds
    for (int j = 0; j < chaseLength; j++) {
      int index = (i + j) % numLeds; // Calculate the index with wraparound
      midBottom[0][index] = CRGB::Red;
      midBottom[1][index] = CRGB::Green;
      midBottom[2][index] = CRGB::Blue;
      midBottom[3][index] = CRGB::Red;
      midBottom[4][index] = CRGB::Green;
      midBottom[5][index] = CRGB::Blue;
    }

    // Dim the trailing tail within the strip bounds
    for (int j = 0; j < trailLength; j++) {
      int index = (i - j - 1 + numLeds) % numLeds; // Calculate the index with wraparound
      midBottom[0][index] = CRGB(255 - (trailLength - j) * 20, 0, 0);
      midBottom[1][index] = CRGB(0, 255 - (trailLength - j) * 20, 0);
      midBottom[2][index] = CRGB(0, 0, 255 - (trailLength - j) * 20);
      midBottom[3][index] = CRGB(255 - (trailLength - j) * 20, 0, 0);
      midBottom[4][index] = CRGB(0, 255 - (trailLength - j) * 20, 0);
      midBottom[5][index] = CRGB(0, 0, 255 - (trailLength - j) * 20);
    }

    FastLED.show();
  }
}

void middleChase() {
  int chaseLength = 32; // Number of LEDs in the chase
  int numLeds = NUM_LEDS_PER_STRIP;
  int trailLength = 4; // Length of the trailing tail

  // Loop the chase animation indefinitely
  // Iterate over each LED position in the strip
  for (int i = 0; i < numLeds; i++) {
    // Turn off all LEDs in the strip
    for (int j = 0; j < numLeds; j++) {
      middle[0][j] = CRGB::Black;
      middle[1][j] = CRGB::Black;
      middle[2][j] = CRGB::Black;
      middle[3][j] = CRGB::Black;
      middle[4][j] = CRGB::Black;
      middle[5][j] = CRGB::Black;
    }

    // Turn on the chase LEDs with full intensity within the strip bounds
    for (int j = 0; j < chaseLength; j++) {
      int index = (i + j) % numLeds; // Calculate the index with wraparound
      middle[0][index] = CRGB::Red;
      middle[1][index] = CRGB::Green;
      middle[2][index] = CRGB::Blue;
      middle[3][index] = CRGB::Red;
      middle[4][index] = CRGB::Green;
      middle[5][index] = CRGB::Blue;
    }

    // Dim the trailing tail within the strip bounds
    for (int j = 0; j < trailLength; j++)
    {
      int index = (i - j - 1 + numLeds) % numLeds; // Calculate the index with wraparound
      middle[0][index] = CRGB(255 - (trailLength - j) * 20, 0, 0);
      middle[1][index] = CRGB(0, 255 - (trailLength - j) * 20, 0);
      middle[2][index] = CRGB(0, 0, 255 - (trailLength - j) * 20);
      middle[3][index] = CRGB(255 - (trailLength - j) * 20, 0, 0);
      middle[4][index] = CRGB(0, 255 - (trailLength - j) * 20, 0);
      middle[5][index] = CRGB(0, 0, 255 - (trailLength - j) * 20);
    }

    // Show the LEDs
    FastLED.show();
  }
}

void midTopChase() {
  int chaseLength = 32; // Number of LEDs in the chase
  int numLeds = NUM_LEDS_PER_STRIP;
  int trailLength = 4; // Length of the trailing tail

  // Loop the chase animation indefinitely
  // Iterate over each LED position in the strip
  for (int i = 0; i < numLeds; i++) {
    // Turn off all LEDs in the strip
    for (int j = 0; j < numLeds; j++) {
      midTop[0][j] = CRGB::Black;
      midTop[1][j] = CRGB::Black;
      midTop[2][j] = CRGB::Black;
      midTop[3][j] = CRGB::Black;
      midTop[4][j] = CRGB::Black;
      midTop[5][j] = CRGB::Black;
    }

    // Turn on the chase LEDs with full intensity within the strip bounds
    for (int j = 0; j < chaseLength; j++) {
      int index = (i + j) % numLeds; // Calculate the index with wraparound
      midTop[0][index] = CRGB::Red;
      midTop[1][index] = CRGB::Green;
      midTop[2][index] = CRGB::Blue;
      midTop[3][index] = CRGB::Red;
      midTop[4][index] = CRGB::Green;
      midTop[5][index] = CRGB::Blue;
    }

    // Dim the trailing tail within the strip bounds
    for (int j = 0; j < trailLength; j++) {
      int index = (i - j - 1 + numLeds) % numLeds; // Calculate the index with wraparound
      midTop[0][index] = CRGB(255 - (trailLength - j) * 20, 0, 0);
      midTop[1][index] = CRGB(0, 255 - (trailLength - j) * 20, 0);
      midTop[2][index] = CRGB(0, 0, 255 - (trailLength - j) * 20);
      midTop[3][index] = CRGB(255 - (trailLength - j) * 20, 0, 0);
      midTop[4][index] = CRGB(0, 255 - (trailLength - j) * 20, 0);
      midTop[5][index] = CRGB(0, 0, 255 - (trailLength - j) * 20);
    }

    // Show the LEDs
    FastLED.show();
  }
}

void topChase() {
  int chaseLength = 32; // Number of LEDs in the chase
  int numLeds = NUM_LEDS_PER_STRIP;
  int trailLength = 4; // Length of the trailing tail

  // Loop the chase animation indefinitely
  for (int i = 0; i < numLeds; i++) {
    // Turn off all LEDs in the strip
    for (int j = 0; j < numLeds; j++) {
      top[0][j] = CRGB::Black;
      top[1][j] = CRGB::Black;
      top[2][j] = CRGB::Black;
    }

    // Turn on the chase LEDs with full intensity within the strip bounds
    for (int j = 0; j < chaseLength; j++) {
      int index = (i + j) % numLeds; // Calculate the index with wraparound
      top[0][index] = CRGB::Red;
      top[1][index] = CRGB::Green;
      top[2][index] = CRGB::Blue;
    }

    // Dim the trailing tail within the strip bounds
    for (int j = 0; j < trailLength; j++) {
      int index = (i - j - 1 + numLeds) % numLeds; // Calculate the index with wraparound
      top[0][index] = CRGB(255 - (trailLength - j) * 20, 0, 0);
      top[1][index] = CRGB(0, 255 - (trailLength - j) * 20, 0);
      top[2][index] = CRGB(0, 0, 255 - (trailLength - j) * 20);
    }

    // Show the LEDs
    FastLED.show();
  }
}

void rainbowChase() {
  static uint8_t startIndex = 0;
  startIndex += 24; /* motion speed */

  // Iterate all LEDs to create a rainbow chase effect, do the different pins in parralel.
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds1[i] = CHSV((startIndex + (i * 2)), 255, 255);
    leds2[i] = CHSV((startIndex + (i * 2)), 255, 255);
    leds3[i] = CHSV((startIndex + (i * 2)), 255, 255);
    leds4[i] = CHSV((startIndex + (i * 2)), 255, 255);
    leds5[i] = CHSV((startIndex + (i * 2)), 255, 255);
    leds6[i] = CHSV((startIndex + (i * 2)), 255, 255);
  }
  FastLED.show();
  //delay(1); /* frame rate */
}

void movingLavaNoise() {
// Define palette colors for lava effect
  CRGBPalette16 palette = HeatColors_p;
  
  // Scale factor for noise animation
  uint8_t scale = 60;

  // Animate noise
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    uint8_t index = inoise8(i * scale, millis() / 1000);
    CRGB color = ColorFromPalette(palette, index);
    leds1[i] = color;
  }

  // Slowly shift colors along the LED strip
  static uint8_t hue = 0;
  fill_rainbow(leds2, NUM_LEDS_PER_SEGMENT, hue++, 3);

  // Show LED strip
  FastLED.show();

  // Delay for animation
  delay(50); // Adjust the delay value for animation speed
}
