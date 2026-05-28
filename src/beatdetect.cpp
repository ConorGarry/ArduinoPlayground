#include "beatdetect.h"

namespace beatdetect {

// ---- Hardware ----
// A1 on Teensy 4.1 = Pin 15. Change here if mic is wired elsewhere.
static const int   MIC_PIN          = 15;
static const int   ADC_RESOLUTION   = 12;     // 0..4095
static const float ADC_MID_RAIL     = 2048.0f; // AC-coupled mic biased at Vcc/2

// ---- Tuning (see ../docs/ESP32-Web-App.md §6.3) ----
static const float ENVELOPE_DECAY   = 0.93f;  // each iteration when sample <  envelope
static const float BASELINE_ALPHA   = 0.001f; // baseline = (1-α)*baseline + α*envelope
static const float BEAT_THRESHOLD   = 1.5f;   // envelope > baseline * THRESHOLD → beat
static const float BASELINE_FLOOR   = 40.0f;  // counts; prevents instant retrigger when silent
static const unsigned long BEAT_HOLD_MS    = 100;
static const unsigned long BEAT_LOCKOUT_MS = 200;

// ---- State ----
static bool          _enabled       = false;
static float         _envelope      = 0.0f;
static float         _baseline      = 100.0f;
static unsigned long _beatStartMs   = 0;
static unsigned long _beatLockedUntilMs = 0;
static unsigned long _lastSampleMs  = 0;

void begin() {
  pinMode(MIC_PIN, INPUT);
  analogReadResolution(ADC_RESOLUTION);
  // Default averaging on Teensy 4.1 is fine; tweak via analogReadAveraging()
  // if signal looks too noisy at venue.
}

void poll() {
  if (!_enabled) return;

  unsigned long now = millis();
  // Sample at ~1 kHz. analogRead on Teensy 4.1 is fast (~3 µs) — the millis
  // gate is just to prevent floods if the main loop is short.
  if (now == _lastSampleMs) return;
  _lastSampleMs = now;

  int   raw = analogRead(MIC_PIN);
  float amp = fabsf((float)raw - ADC_MID_RAIL);

  // Envelope follower: instant attack, exponential decay.
  if (amp > _envelope) {
    _envelope = amp;
  } else {
    _envelope *= ENVELOPE_DECAY;
  }

  // Adaptive baseline — slow IIR. Floored so silent rooms don't trip on noise.
  _baseline = (1.0f - BASELINE_ALPHA) * _baseline + BASELINE_ALPHA * _envelope;
  if (_baseline < BASELINE_FLOOR) _baseline = BASELINE_FLOOR;

  // Beat decision: envelope clearly exceeds baseline AND we're past the
  // last beat's lockout window.
  if (_envelope > _baseline * BEAT_THRESHOLD && now >= _beatLockedUntilMs) {
    _beatStartMs       = now;
    _beatLockedUntilMs = now + BEAT_LOCKOUT_MS;
  }
}

void setEnabled(bool on) {
  if (on == _enabled) return;
  _enabled = on;
  if (on) {
    // Reset state so we don't reuse stale envelopes from a previous session.
    _envelope          = 0.0f;
    _baseline          = 100.0f;
    _beatStartMs       = 0;
    _beatLockedUntilMs = 0;
  }
}

bool enabled() {
  return _enabled;
}

bool beatActive() {
  if (!_enabled || _beatStartMs == 0) return false;
  return (millis() - _beatStartMs) < BEAT_HOLD_MS;
}

unsigned long lastBeatMs() {
  return _beatStartMs;
}

}  // namespace beatdetect
