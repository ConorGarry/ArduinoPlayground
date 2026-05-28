// ESP32 firmware for the Infinity / Meta Sonic Flux web-app bridge.
// See ../../docs/ESP32-Web-App.md.

#include <Arduino.h>
#include <LittleFS.h>

#include "config.h"
#include "bridge.h"
#include "questions.h"
#include "wifi_ap.h"
#include "web_routes.h"

static unsigned long lastPingMs = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("=== InfinityFlux ESP32 boot ===");

  if (!LittleFS.begin(true)) {
    Serial.println("[main] LittleFS mount failed — formatted on first boot. Re-flash data partition.");
  }

  questions::load();
  bridge::begin();
  wifi_ap::begin();
  web_routes::begin();

  randomSeed((unsigned long) esp_random());

  // The Teensy only honours web overrides while PINGs are arriving. Without
  // this initial ping, a phone tapping in the first 5 s after boot has its
  // command silently dropped.
  bridge::sendPing();

  Serial.println("[main] ready");
}

void loop() {
  wifi_ap::poll();
  web_routes::poll();
  bridge::poll();

  unsigned long now = millis();
  if (now - lastPingMs > 5000) {
    bridge::sendPing();
    lastPingMs = now;
  }
}
