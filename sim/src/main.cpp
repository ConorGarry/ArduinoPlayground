#include <Arduino.h>
#include <FastLED.h>

#define RUNS 20
#define NUM_LEDS 1440
#define NUM_LEDS_PER_STRIP 72
#define NUM_LEDS_PER_SEGMENT 360

// Define the data pins for the LED strips
#define DP_1 17
// #define DP_2 8
// #define DP_3 9
// #define DP_4 10
// Define the LED strip type
#define LED_TYPE WS2812B
// Define the color order of the LED strip
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
// CRGB leds1[NUM_LEDS_PER_SEGMENT];
// CRGB leds2[NUM_LEDS_PER_SEGMENT];
// CRGB leds3[NUM_LEDS_PER_SEGMENT];
// CRGB leds4[NUM_LEDS_PER_SEGMENT];

const bool DEBUG_MODE = false;

//const int switchPins[] = {2, 3, 4, 5, 6}; // Pins connected to the switches
const int switchPins[] = {15, 2, 0, 4, 16}; // ESP32

int mode = 0;

void setup() {
  if (DEBUG_MODE) {
    Serial.begin(9600);
  }
  // Initialize the LED strip
  FastLED.addLeds<LED_TYPE, DP_1, COLOR_ORDER>(leds, NUM_LEDS);
  // FastLED.addLeds<LED_TYPE, DP_1, COLOR_ORDER>(leds1, NUM_LEDS_PER_SEGMENT);
  // FastLED.addLeds<LED_TYPE, DP_2, COLOR_ORDER>(leds2, NUM_LEDS_PER_SEGMENT);
  // FastLED.addLeds<LED_TYPE, DP_3, COLOR_ORDER>(leds3, NUM_LEDS_PER_SEGMENT);
  // FastLED.addLeds<LED_TYPE, DP_4, COLOR_ORDER>(leds4, NUM_LEDS_PER_SEGMENT);
  FastLED.setMaxRefreshRate(0);
  FastLED.clear();

  // Initialize switch pins as inputs
  // for (int i = 0; i < 5; i++) {
  //   pinMode(switchPins[i], INPUT_PULLUP); // Enable internal pull-up resistors
  // }
}

// Animate a chase effect with a rainbow pattern.
void rainbowChase() {
  static uint8_t startIndex = 0;
  startIndex += 5; /* motion speed */
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((startIndex + (i * 2)), 255, 255);
  }

  // Iterate all LEDs to create a rainbow chase effect, do the different pins in parralel.
  // for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
  //   leds1[i] = CHSV((startIndex + (i * 2)), 255, 255);
  //   leds2[i] = CHSV((startIndex + (i * 2)), 255, 255);
  //   leds3[i] = CHSV((startIndex + (i * 2)), 255, 255);
  //   leds4[i] = CHSV((startIndex + (i * 2)), 255, 255);
  // }

  //fill_rainbow(leds, NUM_LEDS, startIndex, 10);
  FastLED.show();
  //delay(1000 / 600); /* frame rate */
  delay(1); /* frame rate */
}

void loop() {
  // Fill the LED strip with a solid color
  // fill_solid(leds, NUM_LEDS, CRGB::Red);
  //  Show the updated LED colors
  //leds[0] = CRGB::Green;
  //leds[0] = CRGB::Black;
  //FastLED.show();

  //selectMode();
  rainbowChase();
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
    break;
  case 1:
    //whiteBreathingChase();
    break;
  // Add the rest 2..31 as needed.
  default:
    break;
  }
  mode = 0;
}
