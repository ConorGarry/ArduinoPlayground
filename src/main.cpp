#include <Arduino.h>
#include <FastLED.h>

#define RUNS 20
#define NUM_LEDS 2160
#define NUM_LEDS_PER_STRIP 72
#define NUM_LEDS_PER_SEGMENT 360

// Define the data pins for the LED strips
#define DP_1 7
#define DP_2 8
#define DP_3 9
#define DP_4 10
#define DP_5 11
#define DP_6 12
// Define the LED strip type
#define LED_TYPE WS2812B
// Define the color order of the LED strip
#define COLOR_ORDER GRB

CRGB leds1[NUM_LEDS_PER_SEGMENT];
CRGB leds2[NUM_LEDS_PER_SEGMENT];
CRGB leds3[NUM_LEDS_PER_SEGMENT];
CRGB leds4[NUM_LEDS_PER_SEGMENT];
CRGB leds5[NUM_LEDS_PER_SEGMENT];
CRGB leds6[NUM_LEDS_PER_SEGMENT];
CRGB leds7[NUM_LEDS_PER_SEGMENT];

const bool DEBUG_MODE = true;

const int switchPins[] = {2, 3, 4, 5, 6}; // Pins connected to the switches

int mode = 0;

void setup() {
  if (DEBUG_MODE) {
    Serial.begin(115200);
  }

  //Serial.println("Hello, world!");
  // Initialize the LED strip
  FastLED.addLeds<LED_TYPE, DP_1, COLOR_ORDER>(leds1, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_2, COLOR_ORDER>(leds2, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_3, COLOR_ORDER>(leds3, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_4, COLOR_ORDER>(leds4, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_5, COLOR_ORDER>(leds5, NUM_LEDS_PER_SEGMENT);
  FastLED.addLeds<LED_TYPE, DP_6, COLOR_ORDER>(leds6, NUM_LEDS_PER_SEGMENT);
  FastLED.setMaxRefreshRate(0);
  FastLED.clear();

  // Initialize switch pins as inputs
  for (int i = 0; i < 5; i++) {
    pinMode(switchPins[i], INPUT_PULLUP); // Enable internal pull-up resistors
  }
}

// Animate a chase effect with a rainbow pattern.
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

void selectMode();

void loop() {
  FastLED.show();
  selectMode();
  // Log current frame rate to Serial Monitor:
  if (DEBUG_MODE) {
    static unsigned long lastMillis = 0;
    static unsigned long lastFrame = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - lastMillis >= 1000) {
      Serial.print("Frame rate: ");
      Serial.print(lastFrame);
      Serial.println(" fps");
      lastFrame = 0;
      lastMillis = currentMillis;
    }
    lastFrame++;
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
  mode = 0;

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
    //movingLavaNoise();
    break;
  case 1:
    //whiteBreathingChase();
    break;
  // Add the rest 2..31 as needed.
  default:
    break;
  }
}