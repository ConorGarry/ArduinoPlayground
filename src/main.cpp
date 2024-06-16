#include <Arduino.h>
#include <FastLED.h>
#include "led.h"
#include "./conor/conor_patterns.h"
#include "./dee/dee_patterns.h"
#include "./denise/denise_patterns.h"

const bool DEBUG_MODE = true;

// Define the 6 data pins for the LED strips in parallel.
#define DP_1 7
#define DP_2 8
#define DP_3 9
#define DP_4 10
#define DP_5 11
#define DP_6 12

// Pins connected to the 5 toggle switches.
const int switchPins[] = {2, 3, 4, 5, 6}; 

// Start with mode 0 (first delcared pattern) as default.
int mode = 0;

void setup() {
  if (DEBUG_MODE) {
    Serial.begin(9600);
  }

  // Initialize the LED strip
  FastLED.addLeds<LED_TYPE, DP_1, COLOR_ORDER>(leds1, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_2, COLOR_ORDER>(leds2, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_3, COLOR_ORDER>(leds3, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_4, COLOR_ORDER>(leds4, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_5, COLOR_ORDER>(leds5, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_6, COLOR_ORDER>(leds6, NUM_LEDS_PER_SEGMENT);
  FastLED.setMaxRefreshRate(0);

  // Initialize switch pins as inputs
  for (int i = 0; i < 5; i++) {
    pinMode(switchPins[i], INPUT_PULLUP); // Enable internal pull-up resistors
  }
}

void selectMode() {
  // Read the state of each switch and calculate the binary value
  int binaryValue = 0;
  for (int i = 0; i < 5; i++) {
    binaryValue |= digitalRead(switchPins[i]) << i;
  }

  // Convert binary value to decimal mode
  mode = binaryValue;

  // Print the mode value to the Serial Monitor
  if (DEBUG_MODE) {
    Serial.print("Mode: ");
    Serial.print(mode);
    Serial.print(" (Binary: ");
    Serial.print(mode, BIN);
    Serial.println(")");
  }

  FastLED.clear();
  switch (mode) {
    case 0:
      rainbowChase();
      //pentagonTest();
      break;
    case 1:
      bottomChase();
      break;
    case 2:
      midBottomChase();
      break;
    case 3:
      middleChase();
      break;
    case 4:
      midTopChase();
      break;
    case 5:
      topChase();
      break;
    case 6:
      freePalestineStripScan();
      break;
    case 7:
      freePalestineFullBlink();
      break;
    case 8:
      freePalestineChase();
      break;
        default:
      break;
  }
}

void loop() {
  FastLED.show();
  selectMode();
}
