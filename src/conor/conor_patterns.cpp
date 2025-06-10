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
      int index = (i - j - 1 + numLeds) % numLeds;
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
      int index = (i - j - 1 + numLeds) % numLeds;
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
    for (int j = 0; j < trailLength; j++) {
      int index = (i - j - 1 + numLeds) % numLeds;
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
      int index = (i - j - 1 + numLeds) % numLeds;
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
      int index = (i - j - 1 + numLeds) % numLeds;
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
  // Iterate all LEDs to create a rainbow chase effect, do the different pins in
  // parralel.
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    CHSV color = CHSV((startIndex + (i * 2)), 255, 255);
    leds[i] = color;
    leds[i + NUM_LEDS_PER_SEGMENT] = color;
    leds[i + NUM_LEDS_PER_SEGMENT * 2] = color;
    leds[i + NUM_LEDS_PER_SEGMENT * 3] = color;
    leds[i + NUM_LEDS_PER_SEGMENT * 4] = color;
    leds[i + NUM_LEDS_PER_SEGMENT * 5] = color;
  }
  EVERY_N_MILLISECONDS(20) { 
    startIndex += 16;
  }
  FastLED.show();
}

void movingLavaNoise() {
  // Define palette colors for lava effect
  CRGBPalette16 palette = HeatColors_p;

  // Scale factor for noise animation
  uint8_t scale = 60;

  // Animate noise
  for (int i = 0; i < 2130; i++) {
    uint8_t index = inoise8(i * scale, millis() / 1000);
    CRGB color = ColorFromPalette(palette, index);
    leds[i] = color;
  }

  // Slowly shift colors along the LED strip
  static uint8_t hue = 0;
  fill_rainbow(leds, NUM_LEDS, hue++, 3);

  // Show LED strip
  FastLED.show();

  // Delay for animation
  delay(50); // Adjust the delay value for animation speed
}

void rainbowFade() {
  static uint8_t startIndex = 0;
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds[i] = CHSV(startIndex, 255, 255);
    leds[i + NUM_LEDS_PER_SEGMENT] = CHSV(startIndex, 255, 255);
    leds[i + NUM_LEDS_PER_SEGMENT * 2] = CHSV(startIndex, 255, 255);
    leds[i + NUM_LEDS_PER_SEGMENT * 3] = CHSV(startIndex, 255, 255);
    leds[i + NUM_LEDS_PER_SEGMENT * 4] = CHSV(startIndex, 255, 255);
    leds[i + NUM_LEDS_PER_SEGMENT * 5] = CHSV(startIndex, 255, 255);
  }
  EVERY_N_MILLISECONDS(20) {
    startIndex++;
  }
  FastLED.show();
}

void middleOutRainbowEffect(CRGB *ledArray, int numLeds, int chaseLength) {
  static int step = 0;
  int middle = numLeds / 2;
  int hueOffset = 0;

  EVERY_N_MILLISECONDS(16) {
    // Clear the LED strip
    fill_solid(ledArray, numLeds, CRGB::Black);

    // Set the rainbow colors starting from the middle
    for (int j = 0; j <= middle; j++) {
      int hue = (hueOffset + j * 10) % 255;
      if (middle + j < numLeds) {
        ledArray[middle + j] = CHSV(hue, 255, 255);
      }
      if (middle - j >= 0) {
        ledArray[middle - j] = CHSV(hue, 255, 255);
      }
    }

    // Increment the step and hue offset
    step = (step + 1) % (middle + 1);
    hueOffset = (hueOffset + 1) % 255;

    // Show the LEDs
    FastLED.show();
  }
}

void rainbowMiddleOut() {
  //for (int i = 0; i < 6; i++) {
    middleOutRainbowEffect(leds, 2130, 32);
  //}
}

void heartbeatPulse(CRGB *ledArray, int numLeds, CRGB color, int interval) {
  static uint8_t brightness = 0;
  static int pulseState = 0; // 0: increasing brightness, 1: decreasing
                             // brightness, 2: short pause, 3: long pause

  EVERY_N_MILLISECONDS(interval) {
    // Clear the LED strip
    fill_solid(ledArray, numLeds, CRGB::Black);

    // Set the brightness based on the pulse state
    switch (pulseState) {
    case 0: // First pulse - increasing brightness
      brightness += 25;
      if (brightness >= 255) {
        brightness = 255;
        pulseState = 1;
      }
      break;
    case 1: // Short pause between pulses
      brightness = 0;
      pulseState = 2;
      break;
    case 2: // Second pulse - increasing brightness
      brightness += 25;
      if (brightness >= 255) {
        brightness = 255;
        pulseState = 3;
      }
      break;
    case 3: // Long pause
      brightness -= 25;
      if (brightness <= 0) {
        brightness = 0;
        pulseState = 0;
      }
      break;
    }

    // Apply the brightness to all LEDs
    for (int i = 0; i < numLeds; i++) {
      ledArray[i] = color;
      ledArray[i].fadeLightBy(255 - brightness);
    }

    // Show the LEDs
    FastLED.show();
  }
}

void heartBeat() {
  int interval = 60; // Adjust the interval for the speed of the heartbeat
  // Apply the heartbeat pulse effect to each strip
  //for (int i = 0; i < 6; i++) {
    heartbeatPulse(leds, 2130, CRGB::Red, interval);
  //}
}

void pentagonTest() {
  CRGB colors[] = {CRGB::Red,        CRGB::Green,  CRGB::Blue,
                   CRGB::Yellow,     CRGB::Purple, CRGB::White,
                   CRGB::BlueViolet, CRGB::Cyan,   CRGB::Brown};

  for (unsigned int i = 0; i < NUM_PENTAGONS; i++) {
    fillPentagon(pentagons[i], colors[i % 9]);
    FastLED.show();
    delay(250);
    fillPentagon(pentagons[i], CRGB::Black);
    FastLED.show();
    delay(250);
  }


  // EVERY_N_MILLISECONDS(750) {
  //   static int colorIndex = 0;
  //   static int i = 0;
  //   fillPenatgon(pentagons[i], colors[colorIndex % 9]);
  //   FastLED.show();
  //   fillPenatgon(pentagons[i], CRGB::Black);
  //   colorIndex = (colorIndex + 1) % NUM_PENTAGONS;
  //   i++;
  // }

  // Not what I wanted, but keeping because it's cool!
  /*EVERY_N_MILLISECONDS(500) {
    for (int i = 0; i < NUM_PENTAGONS; i++) {
    static boolean on;
    on = !on;
    if (on) {
      fillPentagon(pentagons[i], colors[i % 9]); // Choose color based on time
    } else {
      fillPentagon(pentagons[i], CRGB::Black);
    }
    FastLED.show();
  }*/
}

void colorWipe(int color, int wait) {
  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds[i] = color;
    leds[i + NUM_LEDS_PER_SEGMENT] = color;
    leds[i + NUM_LEDS_PER_SEGMENT * 2] = color;
    leds[i + NUM_LEDS_PER_SEGMENT * 3] = color;
    leds[i + NUM_LEDS_PER_SEGMENT * 4] = color;
    leds[i + NUM_LEDS_PER_SEGMENT * 5] = color;
    FastLED.show();
    delayMicroseconds(wait);
  }
}

void colorWipeAll() {
  int microsec = 600000 / NUM_LEDS_PER_SEGMENT;
  colorWipe(RED, microsec);
  colorWipe(GREEN, microsec);
  colorWipe(BLUE, microsec);
  colorWipe(YELLOW, microsec);
  colorWipe(PINK, microsec);
  colorWipe(ORANGE, microsec);
  colorWipe(WHITE, microsec);
  FastLED.show();
}

void fireFlies() {
  static int ledIndex = 0;
  static int colorIndex = 0;
  static CRGB colors[] = {
      CHSV(10, 255, 180), // deep orange
      CHSV(5, 220, 200),  // glowing ember red-orange
      CHSV(0, 255, 150),  // red, dimmed
      CHSV(20, 180, 180), // golden amber
      CHSV(15, 255, 255), // bright orange
      CHSV(7, 240, 170),  // warm flicker
      CHSV(12, 200, 220)  // muted flame
  };

  const int stepDelayMs = 10;
  static unsigned long lastStep = 0;

  if (millis() - lastStep >= stepDelayMs) {
    lastStep = millis();

    // Fill all pixels up to current index
    for (int i = 0; i <= ledIndex && i < NUM_LEDS_PER_SEGMENT; i++) {
      leds[ledIndex] = colors[colorIndex % i];
      leds[ledIndex + NUM_LEDS_PER_SEGMENT] = colors[colorIndex % i];
      leds[ledIndex + NUM_LEDS_PER_SEGMENT * 2] = colors[colorIndex % random()];
      leds[ledIndex + NUM_LEDS_PER_SEGMENT * 3] = colors[colorIndex % random()];
      leds[ledIndex + NUM_LEDS_PER_SEGMENT * 4] = colors[colorIndex % random()];
      leds[ledIndex + NUM_LEDS_PER_SEGMENT * 5] = colors[colorIndex % random()];
      leds[ledIndex + (NUM_LEDS_PER_SEGMENT / 2) * 2] = colors[colorIndex % random(0, 7)];
      leds[ledIndex + (NUM_LEDS_PER_SEGMENT / 3) * 3] = colors[colorIndex % random(0, 7)];
      leds[ledIndex + (NUM_LEDS_PER_SEGMENT / 4) * 4] = colors[colorIndex % random(0, 7)];
      leds[ledIndex + (NUM_LEDS_PER_SEGMENT / 5) * 5] = colors[colorIndex % random(0, 7)];
      leds[ledIndex + (NUM_LEDS_PER_SEGMENT / 2) * 2] = colors[colorIndex % random(0, 7)];
      leds[ledIndex + (NUM_LEDS_PER_SEGMENT / 3) * 3] = colors[colorIndex % random(0, 7)];
      leds[ledIndex + (NUM_LEDS_PER_SEGMENT / 4) * 4] = colors[colorIndex % random(0, 7)];
      leds[ledIndex + (NUM_LEDS_PER_SEGMENT / 5) * 5] = colors[colorIndex % random(0, 7)];
      leds[ledIndex + (NUM_LEDS_PER_SEGMENT / 2) * 2] = colors[colorIndex % random(0, 7)];
      leds[ledIndex + (NUM_LEDS_PER_SEGMENT / 3) * 3] = colors[colorIndex % random(0, 7)];
      leds[ledIndex + (NUM_LEDS_PER_SEGMENT / 4) * 4] = colors[colorIndex % random(0, 7)];
      leds[ledIndex + (NUM_LEDS_PER_SEGMENT / 5) * 5] = colors[colorIndex % random(0, 7)];
    }

    ledIndex++;

    if (ledIndex >= NUM_LEDS_PER_SEGMENT) {
      ledIndex = 0;
      colorIndex = (colorIndex + 1) % (sizeof(colors) / sizeof(colors[0]));

      // Optionally: clear first, or just overwrite in next loop
      FastLED.clear();
    }

    FastLED.show(); // Update display each frame
  }
}