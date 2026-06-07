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

inline unsigned long totalYes = 0;
inline unsigned long totalNo  = 0;

// Set true when the riddle is solved; sticky. Filters riddles in
// questions::pickRandom and exposed via /api/state + /api/stats.
inline bool beatActivated = false;
inline unsigned long beatActivations = 0;

// ---------- per-IP answer cooldown (#7) ----------
// Fixed table sized to the AP client cap — no heap, no single-slot aliasing.
// Each IP gets its own PER_IP_COOLDOWN_MS window; under multi-phone load one
// phone's rapid taps can't reset another's, and a busy table evicts the oldest.
struct IpStamp { uint32_t ip; unsigned long ms; };
inline IpStamp ipStamps[8] = {};

inline bool rateLimited(uint32_t ip, unsigned long now) {
  int freeSlot = -1, oldest = 0;
  for (int i = 0; i < 8; ++i) {
    if (ipStamps[i].ip == ip) {
      if (now - ipStamps[i].ms < PER_IP_COOLDOWN_MS) return true;
      ipStamps[i].ms = now;
      return false;
    }
    if (ipStamps[i].ip == 0 && freeSlot < 0) freeSlot = i;
    if (ipStamps[i].ms < ipStamps[oldest].ms) oldest = i;
  }
  int slot = (freeSlot >= 0) ? freeSlot : oldest;   // evict oldest if full
  ipStamps[slot] = { ip, now };
  return false;
}

// ---------- single active controller (#1 / #8) ----------
// Exactly one phone drives the LEDs at a time. Everyone else gets the holding
// screen. The slot frees on explicit release, or when activity/heartbeat goes
// quiet for SESSION_IDLE_MS (walk-away / locked phone). "" = free.
inline String        activeIp;
inline unsigned long activeLastSeenMs = 0;

inline bool sessionOccupied() {
  return activeIp.length() > 0 && (millis() - activeLastSeenMs) <= SESSION_IDLE_MS;
}
inline bool isActiveController(const String& ip) {
  return ip == activeIp && (millis() - activeLastSeenMs) <= SESSION_IDLE_MS;
}
// Caller already holds the slot → refresh liveness and return true. Otherwise
// send 409 busy and return false. Gates the experience endpoints.
inline bool requireActiveController() {
  String ip = server.client().remoteIP().toString();
  if (isActiveController(ip)) {
    activeLastSeenMs = millis();
    return true;
  }
  server.send(409, "application/json", "{\"error\":\"busy\",\"busy\":true}");
  return false;
}

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
  // Prefer a pre-compressed sibling (tools/gzip_assets.py builds <file>.gz into
  // data/). Cuts the connect-storm transfer ~3-4x. Falls back to the raw file.
  String fsPath = path;
  bool gzipped = false;
  if (LittleFS.exists(path + ".gz")) {
    fsPath = path + ".gz";
    gzipped = true;
  } else if (!LittleFS.exists(path)) {
    return false;
  }

  File f = LittleFS.open(fsPath, "r");
  if (!f) return false;

  // Immutable assets (js/css/svg/ico) get a long cache so returning phones and
  // OS re-probes skip the re-download. index.html is NOT cached here — its
  // callers set no-store so the captive flow always re-checks.
  if (path.endsWith(".js") || path.endsWith(".css") ||
      path.endsWith(".svg") || path.endsWith(".ico")) {
    server.sendHeader("Cache-Control", "public, max-age=31536000, immutable");
  }
  if (gzipped) server.sendHeader("Content-Encoding", "gzip");
  server.streamFile(f, contentTypeFor(path));   // MIME from the logical path
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
#if DEBUG_HTTP
  Serial.print("[captive] probe: ");
  Serial.print(server.uri());
  Serial.print(" host=");
  Serial.print(server.hostHeader());
  Serial.print(" ip=");
  Serial.println(server.client().remoteIP().toString());
#endif
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
  if (!requireActiveController()) return;
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

// ---------- /api/answer (choice branch) ----------
// type:choice questions carry an options[] array of { id, label, msg, pattern }.
// The client posts `answer:"<id>"`; we look it up case-insensitively against
// the option ids declared in questions.json, and dispatch its pattern.
inline void answerChoice(JsonObject q, const String& choiceId) {
  JsonArray opts = q["options"].as<JsonArray>();
  if (opts.isNull() || opts.size() == 0) {
    server.send(500, "application/json", "{\"error\":\"no options\"}");
    return;
  }

  String wanted = choiceId;
  wanted.toLowerCase();
  wanted.trim();

  for (JsonObject opt : opts) {
    String oid = opt["id"] | "";
    oid.toLowerCase();
    oid.trim();
    if (oid != wanted) continue;

    int pattern = opt["pattern"] | -1;
    const char* msg = opt["msg"] | "";
    if (pattern >= 0) bridge::sendPattern(pattern);

    JsonDocument resp;
    resp["ok"]      = true;
    resp["msg"]     = msg;
    resp["pattern"] = pattern;
    String out;
    serializeJson(resp, out);
    server.send(200, "application/json", out);
    return;
  }

  server.send(400, "application/json", "{\"error\":\"unknown choice\"}");
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
  if (!requireActiveController()) return;   // only the active phone drives LEDs

  uint32_t ip = (uint32_t) server.client().remoteIP();
  if (rateLimited(ip, millis())) {
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

  if (questions::isRiddle(q)) {
    answerRiddle(q, answer);
    return;
  }
  if (questions::isChoice(q)) {
    answerChoice(q, answer);
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
  // Fixed-shape response — built on the stack to avoid per-request heap churn
  // (this and /api/stats are polled by every phone, all night).
  long since = bridge::lastInboundMs == 0
      ? -1
      : (long) (millis() - bridge::lastInboundMs);
  char buf[160];
  snprintf(buf, sizeof(buf),
           "{\"mode\":\"%s\",\"pattern\":%d,\"bridge_ms_since\":%ld,"
           "\"beatActivated\":%s,\"teensyBeat\":%s}",
           bridge::lastTeensyMode, bridge::lastTeensyPattern, since,
           beatActivated ? "true" : "false",
           bridge::lastTeensyBeat ? "true" : "false");
  server.send(200, "application/json", buf);
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
  if (!requireActiveController()) return;   // only the active phone drives LEDs
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
// Acquire the single controller slot. Granted if the slot is free, expired, or
// already ours → {ok:true}; otherwise {ok:false,busy:true} and the client shows
// the holding screen. On a fresh grant we engage the Teensy override at the
// *current* pattern so nothing visibly jumps (unknown pattern -1 → nothing sent,
// engages on first answer). Operator switches still reclaim instantly Teensy-side.
inline void handleApiTakeover() {
  String ip = server.client().remoteIP().toString();
  if (sessionOccupied() && !isActiveController(ip)) {
    server.send(200, "application/json", "{\"ok\":false,\"busy\":true}");
    return;
  }
  bool fresh = !isActiveController(ip);
  activeIp = ip;
  activeLastSeenMs = millis();
  int p = bridge::lastTeensyPattern;
  if (fresh && p >= 0) bridge::sendPattern(p);
  char buf[48];
  snprintf(buf, sizeof(buf), "{\"ok\":true,\"pattern\":%d}", p);
  server.send(200, "application/json", buf);
}

// ---------- /api/heartbeat ----------
// Active phone pings (~8 s) to keep its slot alive; returns {mine:true}.
// Waiting phones poll it to learn when the slot frees ({mine:false,occupied:…}).
inline void handleApiHeartbeat() {
  String ip = server.client().remoteIP().toString();
  if (isActiveController(ip)) {
    activeLastSeenMs = millis();
    server.send(200, "application/json", "{\"mine\":true}");
    return;
  }
  server.send(200, "application/json",
              sessionOccupied() ? "{\"mine\":false,\"occupied\":true}"
                                : "{\"mine\":false,\"occupied\":false}");
}

// ---------- /api/release ----------
// Active phone hands the slot back (end of fire ritual, or on leaving) so the
// next person isn't waiting out the idle timeout. No-op from anyone else.
inline void handleApiRelease() {
  String ip = server.client().remoteIP().toString();
  if (isActiveController(ip)) activeIp = "";
  server.send(200, "application/json", "{\"ok\":true}");
}

// ---------- /api/stats ----------
inline void handleApiStats() {
  char buf[160];
  snprintf(buf, sizeof(buf),
           "{\"yes\":%lu,\"no\":%lu,\"beatActivated\":%s,\"beatActivations\":%lu}",
           totalYes, totalNo, beatActivated ? "true" : "false", beatActivations);
  server.send(200, "application/json", buf);
}

// ---------- root ----------
inline void handleRoot() {
#if DEBUG_HTTP
  Serial.print("[http] GET / host=");
  Serial.print(server.hostHeader());
  Serial.print(" ip=");
  Serial.println(server.client().remoteIP().toString());
#endif
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
#if DEBUG_HTTP
  Serial.print("[captive] notFound: ");
  Serial.print(uri);
  Serial.print(" host=");
  Serial.print(server.hostHeader());
  Serial.print(" ip=");
  Serial.println(server.client().remoteIP().toString());
#endif
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
  server.on("/api/heartbeat",HTTP_GET,  handleApiHeartbeat);
  server.on("/api/release",  HTTP_POST, handleApiRelease);

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
