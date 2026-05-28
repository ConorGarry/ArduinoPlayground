#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <set>
#include "config.h"
#include "bridge.h"
#include "questions.h"
#include "wifi_ap.h"

// HTTP routes. Single shared sync WebServer. Captive-portal probes are caught
// by explicit endpoints AND an onNotFound redirect to root. /api/* → JSON.
// Question dispatch: binary → P:<n>, riddle → matches q.answers[] and on
// success forwards q.success.action (e.g. "BEAT:on") + flips beatActivated.

namespace web_routes {

inline WebServer server(80);

inline String  lastIp;
inline unsigned long lastAnswerMs = 0;
inline unsigned long totalYes = 0;
inline unsigned long totalNo  = 0;

// Set true when the riddle is solved; sticky. Filters riddles in
// questions::pickRandom and exposed via /api/state + /api/stats.
inline bool beatActivated = false;
inline unsigned long beatActivations = 0;

inline String contentTypeFor(const String& path) {
  if (path.endsWith(".html")) return "text/html";
  if (path.endsWith(".css"))  return "text/css";
  if (path.endsWith(".js"))   return "application/javascript";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".svg"))  return "image/svg+xml";
  if (path.endsWith(".ico"))  return "image/x-icon";
  return "text/plain";
}

inline bool serveStatic(const String& path) {
  if (!LittleFS.exists(path)) return false;
  File f = LittleFS.open(path, "r");
  if (!f) return false;
  server.streamFile(f, contentTypeFor(path));
  f.close();
  return true;
}

// Captive-portal detection on iOS/Android caches the verdict aggressively;
// a stale "Success" from a previous network suppresses the popup.
inline void sendNoCacheHeaders() {
  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
}

inline void redirectToRoot() {
  sendNoCacheHeaders();
  server.sendHeader("Location", String("http://") + wifi_ap::apIp().toString() + "/", true);
  server.send(302, "text/plain", "");
}

// ---------- captive portal probes ----------
// 302 (rather than serving index inline) is the most reliable trigger for
// iOS "Sign in to Wi-Fi" and Android full-screen captive UI.
inline void handleCaptiveProbe() {
  Serial.print("[captive] probe: ");
  Serial.print(server.uri());
  Serial.print(" host=");
  Serial.print(server.hostHeader());
  Serial.print(" ip=");
  Serial.println(server.client().remoteIP().toString());
  redirectToRoot();
}

// "1,2,99" → {1, 2, 99}
inline std::set<int> parseSeenParam(const String& s) {
  std::set<int> result;
  int start = 0;
  while (start < (int) s.length()) {
    int comma = s.indexOf(',', start);
    String token = (comma < 0) ? s.substring(start) : s.substring(start, comma);
    token.trim();
    if (token.length() > 0) result.insert(token.toInt());
    if (comma < 0) break;
    start = comma + 1;
  }
  return result;
}

// ---------- /api/question ----------
// ?seen=1,2,99 — qids already answered, excluded. All eligible excluded →
// {"exhausted":true} signals client to trigger the finale.
inline void handleApiQuestion() {
  std::set<int> seen = parseSeenParam(server.arg("seen"));
  JsonObject q = questions::pickRandom(beatActivated, seen.empty() ? nullptr : &seen);
  if (q.isNull()) {
    if (questions::count() == 0) {
      server.send(503, "application/json", "{\"error\":\"no questions loaded\"}");
    } else {
      server.send(200, "application/json", "{\"exhausted\":true}");
    }
    return;
  }
  String body;
  serializeJson(q, body);
  server.send(200, "application/json", body);
}

// ---------- /api/answer (binary branch) ----------
inline void answerBinary(JsonObject q, const char* answer) {
  JsonObject branch = (strcmp(answer, "yes") == 0) ? q["yes"].as<JsonObject>()
                                                    : q["no"].as<JsonObject>();
  int pattern = branch["pattern"] | -1;
  const char* msg = branch["msg"] | "";

  if (pattern >= 0) bridge::sendPattern(pattern);
  else              bridge::sendAnswer(answer);

  if (strcmp(answer, "yes") == 0) totalYes++; else totalNo++;

  JsonDocument resp;
  resp["ok"]      = true;
  resp["msg"]     = msg;
  resp["pattern"] = pattern;
  String out;
  serializeJson(resp, out);
  server.send(200, "application/json", out);
}

// ---------- /api/answer (riddle branch) ----------
inline void answerRiddle(JsonObject q, String userAns) {
  userAns.toLowerCase();
  userAns.trim();

  bool correct = false;
  JsonArray ans = q["answers"].as<JsonArray>();
  for (JsonVariant v : ans) {
    String candidate = v.as<String>();
    candidate.toLowerCase();
    candidate.trim();
    if (userAns == candidate) { correct = true; break; }
  }

  if (correct) {
    const char* action = q["success"]["action"] | "";
    if (action[0] != '\0') bridge::sendLine(String(action));
    int pattern = q["success"]["pattern"] | -1;
    if (pattern >= 0) bridge::sendPattern(pattern);

    if (!beatActivated) {
      beatActivated = true;
      beatActivations++;
    }

    JsonDocument resp;
    resp["ok"]            = true;
    resp["msg"]           = q["success"]["msg"] | "you've activated the beat";
    resp["beatActivated"] = true;
    String out;
    serializeJson(resp, out);
    server.send(200, "application/json", out);
  } else {
    // 200 + ok:false so the client can track attempt count.
    JsonDocument resp;
    resp["ok"]  = false;
    resp["msg"] = q["fail_msg"] | "not quite…";
    String out;
    serializeJson(resp, out);
    server.send(200, "application/json", out);
  }
}

// ---------- /api/answer ----------
inline void handleApiAnswer() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"error\":\"method\"}");
    return;
  }

  String ip = server.client().remoteIP().toString();
  unsigned long now = millis();
  if (ip == lastIp && (now - lastAnswerMs) < PER_IP_COOLDOWN_MS) {
    server.send(429, "application/json", "{\"error\":\"slow down\"}");
    return;
  }

  JsonDocument req;
  if (deserializeJson(req, server.arg("plain"))) {
    server.send(400, "application/json", "{\"error\":\"bad json\"}");
    return;
  }
  int qid = req["qid"] | -1;
  String answer = req["answer"] | "";
  if (qid < 0 || answer.length() == 0) {
    server.send(400, "application/json", "{\"error\":\"need qid + answer\"}");
    return;
  }

  JsonObject q = questions::byId(qid);
  if (q.isNull()) {
    server.send(404, "application/json", "{\"error\":\"unknown qid\"}");
    return;
  }

  lastIp = ip;
  lastAnswerMs = now;

  if (questions::isRiddle(q)) {
    answerRiddle(q, answer);
    return;
  }
  if (!(answer == "yes" || answer == "no")) {
    server.send(400, "application/json", "{\"error\":\"need yes/no\"}");
    return;
  }
  answerBinary(q, answer.c_str());
}

// ---------- /api/state ----------
inline void handleApiState() {
  JsonDocument resp;
  resp["mode"]    = bridge::lastTeensyMode;
  resp["pattern"] = bridge::lastTeensyPattern;
  resp["bridge_ms_since"] = bridge::lastInboundMs == 0
      ? -1
      : (long)(millis() - bridge::lastInboundMs);
  resp["beatActivated"] = beatActivated;
  resp["teensyBeat"]    = bridge::lastTeensyBeat;
  String out;
  serializeJson(resp, out);
  server.send(200, "application/json", out);
}

// ---------- /command ----------
// Strict allowlist (not a charset filter) — only the three ritual stages plus
// bench calibration are valid. New control flows get their own endpoint.
static const char* const FIRE_RITUAL_COMMANDS[] = {
  "P:dim_ambient",
  "P:fire_full",
  "P:idle_ambient",
  "P:calibrate",     // bench-only
};

inline void handleCommand() {
  String cmd = server.arg("cmd");
  if (cmd.length() == 0) {
    server.send(400, "application/json", "{\"error\":\"need cmd\"}");
    return;
  }
  for (const char* allowed : FIRE_RITUAL_COMMANDS) {
    if (cmd == allowed) {
      bridge::sendLine(cmd);
      server.send(200, "application/json", "{\"ok\":true}");
      return;
    }
  }
  server.send(400, "application/json", "{\"error\":\"bad cmd\"}");
}

// ---------- /api/takeover ----------
// Phone claims control from operator switches. Engages the Teensy web override
// at the *current* pattern so nothing visibly changes. Held until an operator
// switch moves (instant reclaim) or the safety-backstop timeout fires. If
// pattern is unknown (-1), nothing sent — override engages on first answer.
inline void handleApiTakeover() {
  int p = bridge::lastTeensyPattern;
  if (p >= 0) bridge::sendPattern(p);
  String out = String("{\"ok\":true,\"pattern\":") + p + "}";
  server.send(200, "application/json", out);
}

// ---------- /api/stats ----------
inline void handleApiStats() {
  JsonDocument resp;
  resp["yes"]             = totalYes;
  resp["no"]              = totalNo;
  resp["beatActivated"]   = beatActivated;
  resp["beatActivations"] = beatActivations;
  String out;
  serializeJson(resp, out);
  server.send(200, "application/json", out);
}

// ---------- root ----------
inline void handleRoot() {
  Serial.print("[http] GET / host=");
  Serial.print(server.hostHeader());
  Serial.print(" ip=");
  Serial.println(server.client().remoteIP().toString());
  sendNoCacheHeaders();
  if (!serveStatic("/index.html")) {
    server.send(500, "text/plain", "index.html missing — did you run pio run -t uploadfs?");
  }
}

// ---------- catch-all (captive portal fallback) ----------
inline void handleNotFound() {
  String uri = server.uri();
  if (serveStatic(uri)) return;
  // Almost certainly an unregistered OS captive-portal probe.
  Serial.print("[captive] notFound: ");
  Serial.print(uri);
  Serial.print(" host=");
  Serial.print(server.hostHeader());
  Serial.print(" ip=");
  Serial.println(server.client().remoteIP().toString());
  redirectToRoot();
}

inline void begin() {
  server.on("/",            HTTP_GET, handleRoot);
  server.on("/index.html",  HTTP_GET, handleRoot);

  server.on("/api/question", HTTP_GET,  handleApiQuestion);
  server.on("/api/answer",   HTTP_POST, handleApiAnswer);
  server.on("/api/state",    HTTP_GET,  handleApiState);
  server.on("/api/stats",    HTTP_GET,  handleApiStats);
  server.on("/command",      HTTP_GET,  handleCommand);
  server.on("/api/takeover", HTTP_POST, handleApiTakeover);

  // Captive-portal probes — listed for logging; handleNotFound catches the rest.
  server.on("/generate_204",                       HTTP_GET, handleCaptiveProbe);  // Android
  server.on("/gen_204",                            HTTP_GET, handleCaptiveProbe);  // Android (older)
  server.on("/hotspot-detect.html",                HTTP_GET, handleCaptiveProbe);  // iOS / macOS
  server.on("/library/test/success.html",          HTTP_GET, handleCaptiveProbe);  // iOS
  server.on("/success.txt",                        HTTP_GET, handleCaptiveProbe);  // Firefox
  server.on("/canonical.html",                     HTTP_GET, handleCaptiveProbe);  // Ubuntu
  server.on("/ncsi.txt",                           HTTP_GET, handleCaptiveProbe);  // Windows
  server.on("/connecttest.txt",                    HTTP_GET, handleCaptiveProbe);  // Windows 10+
  server.on("/redirect",                           HTTP_GET, handleCaptiveProbe);  // Windows
  server.on("/check_network_status.txt",           HTTP_GET, handleCaptiveProbe);
  server.on("/chrome-variations/seed",             HTTP_GET, handleCaptiveProbe);  // Chrome
  server.on("/service/update2/json",               HTTP_GET, handleCaptiveProbe);

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("[web_routes] HTTP server listening on :80");
}

inline void poll() {
  server.handleClient();
}

}  // namespace web_routes
