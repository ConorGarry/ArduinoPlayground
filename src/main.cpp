#include <Arduino.h>
#include <FastLED.h>
#include "led.h"
#include "./conor/conor_patterns.h"
#include "./dee/dee_patterns.h"
#include "./denise/denise_patterns.h"
#include "./spencer/spencer_patterns.h"
#include "./app/app_patterns.h"
#include "teensy4controller.h"
#include "webcontrol.h"
// Mic / beat-detect disabled until MAX4466 hardware lands.
// #include "beatdetect.h"

const bool DEBUG_MODE = false;

// Physical operator switches are authoritative ("buttons always win" —
// CLAUDE/LED_Controller_Spec.md §4.3). The web override only changes WHICH
// pattern runs; it adds nothing to the LED data lines, so it is safe to leave
// on. (The scattered wrong-colour pixels seen whenever the bridge harness is
// wired are a separate data-line grounding issue — a ground loop from the
// bridge GND wire — NOT this flag. See the grounding section in
// ../docs/ESP32-Web-App.md §10.4.) Any switch change reclaims control from the
// web instantly. Set false only to lock the web app out of pattern selection
// entirely (the Serial4 status bridge keeps running either way).
const bool WEB_OVERRIDE_ENABLED = true;

// Pins connected to the 3 toggle switches on the operator panel.
const int switchPins[] = {0, 1, 23};
constexpr int NUM_SWITCH_PINS = sizeof(switchPins) / sizeof(switchPins[0]);

// OctoWS2811 DMA output pins (the OctoWS2811 board hard-wires these — only
// the entries in pinList are usable for LED data on this assembly).
byte pinList[NUM_PINS] = {2, 14, 7, 8, 6, 20};

CRGB leds[NUM_LEDS];

// OctoWS2811 needs 32-bit-aligned framebuffer pairs. int alignment is correct
// on Teensy 4.1; the size math is 3 bytes/LED × NUM_LEDS / 4 bytes/int, rounded
// UP — NUM_LEDS*3 is not a multiple of 4 (6390 bytes → 1598 ints, not 1597), so
// plain /4 truncation left the buffer 2 bytes short of what OctoWS2811 writes.
DMAMEM int displayMemory[(NUM_LEDS * 3 + 3) / 4];
int drawingMemory[(NUM_LEDS * 3 + 3) / 4];
OctoWS2811 octo(NUM_LEDS_PER_SEGMENT, displayMemory, drawingMemory, WS2811_RGB | WS2811_800kHz, NUM_PINS, pinList);
CTeensy4Controller<RGB, WS2811_800kHz> *pcontroller;

// Central pattern dispatcher. Shared by the switch path and the web-override
// path so the index → function mapping has exactly one home. App-pattern
// indices come from src/app/app_patterns.h to keep webcontrol.cpp in sync.
void runPattern(int idx) {
  using namespace app_patterns;
  switch (idx) {
    case 0: rainbowChase();           break;
    case 1: fireFlies();              break;
    case 2: twinkle(); comets();      break;
    case 3: waveVerticalsOverwards(); break;
    case 4: freePalestineFullBlink(); break;
    case 5: rainbowFade();            break;
    case 6: showLights();             break;
    case 7: galaxy();                 break;
    case DIM_AMBIENT:  dimAmbient();  break;
    case FIRE_FULL:    fireFull();    break;
    case IDLE_AMBIENT: idleAmbient(); break;
    case WORM_YES:     wormYes();     break;
    case WORM_NO:      wormNo();      break;
    case SCRUNCH:      scrunch();     break;
    case FOLD:         fold();        break;
    case CALIBRATE:    calibrate();   break;
    default:           galaxy();      break;
  }
}

void setup() {
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
  FastLED.addLeds(pcontroller, leds, NUM_LEDS);

  for (int i = 0; i < NUM_SWITCH_PINS; i++) {
    pinMode(switchPins[i], INPUT_PULLUP);
  }

  webcontrol::begin();
  // beatdetect::begin();   // re-enable when the mic is wired (A1 / Pin 15)
}

void selectMode() {
  // Pack the 3 INPUT_PULLUP switches into a binary mode index. Non-inverted
  // read per the documented design (CLAUDE/hardware.md + patterns.md): an open
  // switch reads 1, a grounded switch reads 0, pin 0 is the LSB, so all-open =
  // mode 7 = galaxy. switchPins = {0, 1, 23}.
  int switchMode = 0;
  for (int i = 0; i < NUM_SWITCH_PINS; i++) {
    switchMode |= digitalRead(switchPins[i]) << i;
  }

  // Buttons always win. With the web override gated off (WEB_OVERRIDE_ENABLED),
  // the ESP bridge cannot hijack or glitch the operator patterns — the switches
  // are the sole authority. When re-enabled, any switch change reclaims control
  // from the web instantly.
  int chosen = switchMode;
  if (WEB_OVERRIDE_ENABLED) {
    static int overrideBaselineSwitch = -1;
    if (webcontrol::overrideActive()) {
      if (overrideBaselineSwitch < 0) {
        overrideBaselineSwitch = switchMode;          // snapshot at engage
      } else if (switchMode != overrideBaselineSwitch) {
        webcontrol::cancelOverride();                 // operator took the panel back
        overrideBaselineSwitch = -1;
      }
    } else {
      overrideBaselineSwitch = -1;
    }
    if (webcontrol::overrideActive()) chosen = webcontrol::overridePattern();
  }

  if (DEBUG_MODE) {
    static unsigned long _lastDbg = 0;
    if (millis() - _lastDbg >= 500) {
      _lastDbg = millis();
      Serial.print("switchMode=");
      Serial.print(switchMode);
      Serial.print(" chosen=");
      Serial.print(chosen);
      Serial.print(" override=");
      Serial.print(webcontrol::overrideActive());
      Serial.print(" overridePattern=");
      Serial.println(webcontrol::overridePattern());
    }
  }

  FastLED.clear();
  FastLED.setBrightness(255);  // mic-driven dimming re-enables with beatdetect
  webcontrol::setCurrentPattern(chosen);
  runPattern(chosen);
}

void loop() {
  webcontrol::poll();
  // beatdetect::poll();   // re-enable when the mic is wired
  selectMode();
}
