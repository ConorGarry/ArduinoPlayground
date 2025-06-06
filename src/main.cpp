#include <Arduino.h>
#include <FastLED.h>
#include "led.h"
#include "./conor/conor_patterns.h"
#include "./dee/dee_patterns.h"
#include "./denise/denise_patterns.h"
#include "teensy4controller.h"

const bool DEBUG_MODE = false;

// Define the 6 data pins for the LED strips in parallel.

#define DP_1 6
#define DP_2 7
#define DP_3 8
#define DP_4 9
#define DP_5 10
#define DP_6 11

// Pins connected to the 5 toggle switches.
const int switchPins[] = {0, 1, 23}; 

byte pinList[NUM_PINS] = {2, 14, 7, 8, 6, 20};
CRGB leds[NUM_PINS * NUM_LEDS];

// The total number of pixels is "ledsPerStrip * numPins".
// Each pixel needs 3 bytes, so multiply by 3.  An "int" is 4 bytes, so divide by 4.
// The array is created using "int" so the compiler will align it to 32 bit memory.
DMAMEM int displayMemory[NUM_LEDS * NUM_PINS * 3 / 4];
int drawingMemory[NUM_LEDS * NUM_PINS * 3 / 4];
OctoWS2811 octo(NUM_LEDS, displayMemory, drawingMemory, WS2811_RGB | WS2811_800kHz, NUM_PINS, pinList);
CTeensy4Controller<RGB, WS2811_800kHz> *pcontroller;

// Start with mode 0 (first delcared pattern) as default.
int mode = 0;

void setup() {
  // Read the state of each switch and calculate the binary value
 // int binaryValue = 0;
 // for (int i = 0; i < 3; i++) {
//    binaryValue |= digitalRead(switchPins[i]) << i;
 // }

  // Convert binary value to decimal mode
 // mode = binaryValue;

  if (DEBUG_MODE) {
    Serial.begin(9600);
  }

  for (int i = 0; i < NUM_LEDS_PER_SEGMENT; i++) {
    leds1[i] = leds[i];
    leds2[i] = leds[i + NUM_LEDS_PER_SEGMENT];
    leds3[i] = leds[i + NUM_LEDS_PER_SEGMENT * 2];
    leds4[i] = leds[i + NUM_LEDS_PER_SEGMENT * 3];
    leds5[i] = leds[i + NUM_LEDS_PER_SEGMENT * 4];
    leds6[i] = leds[i + NUM_LEDS_PER_SEGMENT * 5];
  }

  octo.begin();
  pcontroller = new CTeensy4Controller<RGB, WS2811_800kHz>(&octo);
  FastLED.setBrightness(255);
  FastLED.addLeds(pcontroller, leds, NUM_PINS * NUM_LEDS);

  // Initialize switch pins as inputs
  for (int i = 0; i < 3; i++) {
    pinMode(switchPins[i], INPUT_PULLUP); // Enable internal pull-up resistors
  }
}

void selectMode() {

  // Read the state of each switch and calculate the binary value
  int binaryValue = 0;
  for (int i = 0; i < 3; i++) {
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
      colorWipeAll();
      // Fire2012();      
      // bottomChase();
      // midBottomChase();
      // middleChase();
      // midTopChase();
      // topChase();
      break;
    case 2:
      twinkle();
      comets();
      break;
    case 3:
      waveVerticalsOverwards();
      // pride();
      break;
    case 4:
      freePalestineFullBlink();
      // prettyNoise();
      break;
    case 5:
      freePalestineStripScan();
      break;
    // case 7:
      // break;
    // case 8:
      // movingLavaNoise();
      // break;
    default:
      break;
  }

  // EVERY_N_SECONDS(60) {
  //   mode = (mode + 1) % NUM_PATTERNS;
  // }
}

void loop() {
  selectMode();
}
