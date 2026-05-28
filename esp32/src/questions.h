#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <set>

// Serves random questions from /questions.json with type-aware weighting:
// riddles are preferred until beatActivated, then filtered out entirely.
// Schema: ../../docs/ESP32-Web-App.md

namespace questions {

inline JsonDocument doc;
inline bool loaded = false;

// 70% per call → ~91% within 2 pulls, ~97% within 3.
static const int RIDDLE_PREFER_PCT = 70;

inline bool load() {
  File f = LittleFS.open("/questions.json", "r");
  if (!f) {
    Serial.println("[questions] /questions.json missing");
    return false;
  }
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) {
    Serial.print("[questions] parse error: ");
    Serial.println(err.c_str());
    return false;
  }
  loaded = true;
  Serial.print("[questions] loaded ");
  Serial.print(doc["questions"].as<JsonArray>().size());
  Serial.println(" entries");
  return true;
}

inline JsonArray all()   { return doc["questions"].as<JsonArray>(); }
inline int       count() { return loaded ? all().size() : 0; }

inline bool isRiddle(JsonObject q) {
  return strcmp(q["type"] | "binary", "riddle") == 0;
}

// Recently-served ring — prevents back-to-back repeats.
inline int recent[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
inline int recentIdx = 0;

inline bool wasRecent(int id) {
  for (int i = 0; i < 8; ++i) if (recent[i] == id) return true;
  return false;
}

inline void markServed(int id) {
  recent[recentIdx] = id;
  recentIdx = (recentIdx + 1) % 8;
}

// `exclude` = qids the caller has already seen. Returns empty JsonObject if
// every eligible question is excluded (→ client fires ritual finale).
inline JsonObject pickRandom(bool beatActivated, const std::set<int>* exclude = nullptr) {
  if (!loaded || count() == 0) return JsonObject();
  JsonArray arr = all();

  auto isExcluded = [&](int id) {
    return exclude && exclude->count(id) > 0;
  };

  // Phase 1 — riddle preference window
  if (!beatActivated && (long) random(0, 100) < RIDDLE_PREFER_PCT) {
    for (JsonObject q : arr) {
      if (isRiddle(q)) {
        int id = q["id"] | -1;
        if (isExcluded(id)) break;
        markServed(id);
        return q;
      }
    }
  }

  // Phase 2 — random pick within eligible set
  for (int attempt = 0; attempt < 12; ++attempt) {
    int i = (int) random(0, arr.size());
    JsonObject q = arr[i];
    if (beatActivated && isRiddle(q)) continue;
    int id = q["id"] | -1;
    if (isExcluded(id)) continue;
    if (id < 0 || !wasRecent(id)) {
      markServed(id);
      return q;
    }
  }

  // Phase 3 — deterministic fallback
  for (JsonObject q : arr) {
    if (beatActivated && isRiddle(q)) continue;
    int id = q["id"] | -1;
    if (isExcluded(id)) continue;
    markServed(id);
    return q;
  }

  return JsonObject();
}

inline JsonObject byId(int id) {
  if (!loaded) return JsonObject();
  for (JsonObject q : all()) {
    if ((q["id"] | -1) == id) return q;
  }
  return JsonObject();
}

}  // namespace questions
