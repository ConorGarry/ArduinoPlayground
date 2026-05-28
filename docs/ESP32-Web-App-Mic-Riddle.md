# ESP32 Web App - Mic + Riddle (deferred)

This is the parked design + recipe for the **MAX4466 microphone**, the
**Teensy-side beat detection**, and the **riddle activation flow** layered on
top of the festival web app. None of this is live in the codebase today —
this document is the spec for what should happen once the mic is physically
wired and the relevant firmware lines are uncommented.

For the architecture that **is** live today (SoftAP, captive portal, takeover
gate, binary Yes/No questions, fire-ritual finale, Serial4 bridge), see
[ESP32-Web-App.md](./ESP32-Web-App.md).

---

## 1. Why this isn't live today

Two things keep this path parked:

1. **No mic wired.** The MAX4466 module isn't physically connected to the
   Teensy yet. Calling `analogRead()` on a floating analog pin produces
   flickery brightness once `BEAT:on` fires, which would make the lights
   look broken whenever a phone solved the riddle.
2. **Three firmware sites are explicitly commented out** so the build is
   clean even with the mic absent. See §2 for the exact restore steps.

When the MAX4466 arrives and gets soldered onto the rig, follow §2 to bring
all of this online. Nothing here is half-built — the supporting code
(`beatdetect.{h,cpp}`, the riddle dispatch in `web_routes.h`, the
`_disabled_questions[]` slot in `questions.json`) all stays compiled or
loaded, just sidelined.

---

## 2. Revival checklist

Three code sites + one JSON move + a hardware solder. Total: ~10 minutes once
the mic is in hand.

### 2.1 Hardware

Wire the MAX4466 per §6 below. Don't power-on the rest until the wiring is
verified — see §6 for the "if nothing pulses" diagnostic.

### 2.2 Teensy firmware

In [`../src/main.cpp`](../src/main.cpp):

- Uncomment `#include "beatdetect.h"` near the other includes.
- Uncomment `beatdetect::begin()` in `setup()`.
- Uncomment `beatdetect::poll()` in `loop()`.
- Restore the mic-driven brightness-decay body in `selectMode()` (or in a
  dedicated helper). Previous body lives in git history; the spec for what
  it should compute is in §5.

In [`../src/webcontrol.cpp`](../src/webcontrol.cpp):

- Uncomment `#include "beatdetect.h"` near the other includes.
- Restore the `BEAT:on` / `BEAT:off` branches inside `handleLine()` so they
  call `beatdetect::setEnabled(true)` / `beatdetect::setEnabled(false)`
  (these are currently silently ignored).
- Restore the `BEAT:` line in `broadcastStatus()` so the ESP can see
  `beatdetect::enabled()` echoed back every 500 ms.

### 2.3 Question pool

In [`../esp32/data/questions.json`](../esp32/data/questions.json):

- Move the riddle entry from `_disabled_questions[]` back into `questions[]`.
- `id` 99 is conventional for the riddle but any unused id works.

### 2.4 Reflash

```bash
pio run -t upload                         # Teensy
pio run -d esp32 -t uploadfs -t upload    # ESP32 data + firmware
```

### 2.5 Verify

Per §8 (manual test flow): solve the riddle, clap near the mic, watch for
brightness pulses and `[bridge ← teensy] BEAT:1` in `tools/monitor.py`.

---

## 3. Hardware - the MAX4466 mic

### 3.1 Bill of materials

| Item | Purpose |
|---|---|
| MAX4466 module (eBay / Adafruit / AliExpress ~€3) | Analog mic + envelope-following preamp. Small breakout with the mic on top, gain trim pot on the back, three pins on the bottom edge labelled `OUT`, `GND`, `VCC`. |
| 3 jumper wires (or solid-core for protoboard) | Connect OUT / VCC / GND. |

### 3.2 Wiring

| MAX4466 pin | Teensy 4.1 pin | Note |
|---|---|---|
| `OUT` | **A1 / Pin 15** | Analog audio output. **Not A0** — see §3.3. |
| `VCC` | **3.3V** | Do NOT use 5V — the MAX4466 will work but output will swing above 3.3V and damage Teensy's ADC over time. |
| `GND` | **GND** | Any ground pin. Must share ground with everything else (Teensy 5V GND + ESP32 GND already common via bridge). |

If the mic and ESP32 are both physically far from the Teensy, run the mic's
GND wire back to the *same* ground point as the rest — avoid star-ground
loops, they show up as 50/60 Hz hum riding on the envelope.

### 3.3 Pin choice: A1 (Pin 15), not A0

Earlier hardware planning documents listed the MAX4466 on Teensy **A0** — but
A0 is **Pin 14**, and Pin 14 is already in `pinList` driving `leds2` (strips
6–10). Connecting the mic OUT to that pin would short the analog signal
against an OctoWS2811 DMA digital output.

The branch uses **A1 (Pin 15)** instead — the next analog pin over, clear of
the OctoWS2811 pin map, easy reroute on the protoboard.

Pin choice is a single `#define MIC_PIN 15` at the top of
[`../src/beatdetect.cpp`](../src/beatdetect.cpp) — change once if your
physical wiring differs.

---

## 4. Bridge framing additions

The bridge protocol stays mostly as documented in
[ESP32-Web-App.md §3.4](./ESP32-Web-App.md). Reviving the mic + riddle adds
three lines back into circulation.

**ESP32 → Teensy** (revived):

```
BEAT:on\n      Enable beat-synced brightness on the Teensy
BEAT:off\n     Disable beat-synced brightness
```

These were already being **parsed and silently ignored** in
`webcontrol.cpp` while the mic was parked; restoring §2.2 makes them
actually toggle `beatdetect::setEnabled()`.

**Teensy → ESP32** (revived):

```
BEAT:<0|1>     Current beatdetect enabled state (echoed every 500 ms)
```

The ESP-side `bridge::handleLine()` already parses `BEAT:` into
`bridge::lastTeensyBeat` regardless — the Teensy just stopped broadcasting
it. Restoring the `BEAT:` line in `broadcastStatus()` revives the echo.

---

## 5. Beat detection algorithm

### 5.1 Why this algorithm

Cheap envelope follower + adaptive baseline + threshold gate. No FFT, no
brightness math beyond `FastLED.setBrightness()`. Runs in the main loop
without an ISR. Picked for predictability under WS2815 DMA timing budget,
not for transcription accuracy.

### 5.2 Core loop

```
sample      = analogRead(A1)                  // 0..4095 on Teensy 4.1
amplitude   = |sample - 2048|                 // DC mid-rail for AC-coupled mic
envelope    = max(amplitude, envelope * 0.93) // fast attack, slow decay
baseline    = baseline * 0.999 + envelope * 0.001   // very slow tracking
if (envelope > baseline * 1.5 && now >= beatLockoutUntil) {
  beatStartMs       = now;
  beatLockoutUntil  = now + 200ms;
}
```

This lives in [`../src/beatdetect.cpp::poll()`](../src/beatdetect.cpp) and
is gated by `_enabled` — `beatdetect::poll()` is a no-op until
`setEnabled(true)` (triggered by the riddle solve).

### 5.3 Brightness modulation

Restored in [`../src/main.cpp`](../src/main.cpp) once the mic is wired:

```
if (beatdetect::enabled()) {
  unsigned long since = millis() - beatdetect::lastBeatMs();
  uint8_t b;
  if (since < 200) {
    b = 255 - ((255 - 153) * since / 200);  // decay 255 → 153 over 200 ms
  } else {
    b = 153;                                  // 60% baseline
  }
  FastLED.setBrightness(b);
} else {
  FastLED.setBrightness(255);
}
```

The else-branch is what runs today. The if-branch is the bit that's
commented out alongside `beatdetect::poll()` and `#include "beatdetect.h"`.

### 5.4 Tuning constants

Knobs live as `static const` at the top of
[`../src/beatdetect.cpp`](../src/beatdetect.cpp):

| Const | Meaning | Default |
|---|---|---|
| `BEAT_THRESHOLD` | Envelope must exceed `baseline * THRESHOLD` to fire a beat | `1.5` |
| `BEAT_LOCKOUT_MS` | Minimum gap between beats (prevents double-fire on a long transient) | `200` |
| `ENVELOPE_DECAY` | Per-iteration envelope decay when sample is below current envelope | `0.93` |
| `BASELINE_ALPHA` | Baseline IIR smoothing constant (very slow) | `0.001` |
| `BASELINE_FLOOR` | Minimum baseline; prevents retrigger in silent rooms | `40` counts |

The MAX4466 itself has a physical gain trim pot — set venue sensitivity
there first; only tune software thresholds if the trim isn't enough.

---

## 6. Recipe: wire the MAX4466

Steps once the mic is in hand:

1. Power both boards off. Wire OUT → A1, VCC → 3.3V, GND → GND (§3.2).
2. Power on. The mic's onboard red LED should light.
3. Apply the firmware changes from §2.2 + JSON move from §2.3. Reflash.
4. Join phone to AP. Solve the riddle (type `mic`).
5. Snap your fingers or clap near the mic. Lights should pulse brighter on
   each transient and settle back to ~60% brightness between hits.

**You're done when** clapping causes a visible brightness pulse AND the
`tools/monitor.py` output (with `DEBUG_BRIDGE=1`) shows
`[bridge ← teensy] BEAT:1` after the riddle solve.

**If it's flickering with no audio**: the envelope-vs-baseline threshold is
too low for your venue. Tune via §7.

**If nothing pulses**: turn the MAX4466 gain trim pot clockwise to increase
gain. If still nothing, check that A1 reads non-zero with a serial-print
test, or that the wiring is to **Pin 15** (counted from one end of the long
Teensy header — easy to off-by-one).

---

## 7. Recipe: tune beat sensitivity at a venue

Defaults in [`../src/beatdetect.cpp`](../src/beatdetect.cpp) were chosen for
bench-room ambient. Real venues (DJ-set volume, crowd noise, wind) need
re-tuning. Tune in this order, *cheapest* first:

1. **MAX4466 trim pot** — small Phillips screw on the back of the module.
   Clockwise = more gain. Set so that quiet ambient gives ~25% envelope and
   transients (snap, kick drum) give 80%+. Watch lights — you want a
   visible-but-not-frantic pulse.
2. **`BEAT_THRESHOLD` in `beatdetect.cpp`** — default `1.5` means envelope
   must exceed 1.5× baseline to fire. Lower (e.g. `1.3`) = more sensitive;
   higher (e.g. `1.8`) = only fires on clear transients. Re-flash to apply.
3. **`BEAT_LOCKOUT_MS`** — minimum gap between beats. Default `200` ms ≈ 5
   beats/sec max. Lower for fast/double-time music, higher to ignore
   re-fires.
4. **`BASELINE_FLOOR`** — prevents false fires in dead silence (the envelope
   would otherwise track *down* to zero, and any noise > zero is "much
   greater than baseline"). Raise if silent rooms still flicker.

**You're done when** sustained music produces consistent pulses without
either drifting to "always on" or "never fires".

---

## 8. The riddle activation flow

### 8.1 Why a riddle

The mic is a hardware capability the crowd doesn't know exists. A riddle
turns discovering it into a moment: one user solves it, the lights start
breathing with the music, and everyone notices at once. Same UX principles
as the binary questions — memorable, low-friction, shareable — with the
bonus of an unlock-style payoff.

### 8.2 The riddle

> I have no mouth, but I hear every word.
> I have no ears, but I catch every sound.
> Speak to me, and the lights will dance.
> What am I?

Accepted answers (case-insensitive, trimmed): `mic`, `microphone`, `mike`,
`the mic`, `a mic`. Curatable via `data/questions.json` without rebuilding
firmware.

### 8.3 UX flow

```
Question card shows the riddle (free text input, lowercased on submit)
       │
       ▼
Answer matches q.answers[]?
       │
       ├── No  → show q.fail_msg + "N attempts left"
       │        Client tracks attempts; after q.max_attempts fails →
       │        show q.reveal_msg, auto-advance to next question
       │
       └── Yes → show q.success.msg + 🎤 emoji
                 Server sends `BEAT:on` to Teensy via Serial4
                 Server sets beatActivated = true (sticky)
                 Lights start pulsing to the mic input
                 Continue with normal questions (riddle filtered from pool)
```

### 8.4 When the riddle appears

Server-side weight: while `beatActivated == false`, `/api/question` returns
the riddle with probability **70%** on each call (and a normal binary
question on the other 30%). Within 2–3 question pulls almost everyone will
encounter it. The constant lives in
[`../esp32/src/questions.h`](../esp32/src/questions.h) as
`RIDDLE_PREFER_PCT`.

Once `beatActivated == true`, the riddle is filtered out of the pool. New
users joining the AP after activation see only normal binary questions —
the magic moment already happened, the beat is on for the rest of the
session.

### 8.5 Multi-user (sticky-on)

Beat activates once and stays on. There is intentionally **no per-user
gating**:

- User A solves the riddle → `BEAT:on` fires → flag set true.
- User B joins later → never sees the riddle → enjoys the beat anyway.
- Reset only on ESP32 reboot (or explicit `BEAT:off` from a future operator
  endpoint, not exposed in the current UI).

The `/api/stats` endpoint exposes `beatActivations` count so the crew can
see "the beat has been awoken N times today" across reboots.

### 8.6 Response screen

```
┌─────────────────────────────┐
│                             │
│            🎤               │
│                             │
│  YOU'VE ACTIVATED THE BEAT  │
│                             │
│   the lights now hear       │
│       what you hear         │
│                             │
│       (look up ↑)           │
│                             │
└─────────────────────────────┘
```

The phrase "the lights now hear what you hear" carries the magic without
explaining the technical reality.

---

## 9. /api/answer riddle dispatch

The endpoint already routes by `q.type`:

- **binary** (default): expects `"yes"` or `"no"`, forwards `P:<n>` and
  responds with the matching `msg`. This is the live path.
- **riddle**: lowercases + trims the free-text answer, checks it against
  `q.answers[]`.
  - **Match**: forwards `q.success.action` raw (e.g. `BEAT:on`), optionally
    also forwards `P:<n>` if `q.success.pattern` is set, sets the
    server-side `beatActivated` flag, returns `{ok:true,
    msg:q.success.msg, beatActivated:true}`.
  - **Miss**: returns `{ok:false, msg:q.fail_msg}` (200 OK — wrong is
    expected). Client tracks attempts and shows `reveal_msg` after
    `max_attempts`.

The riddle branch is already implemented in
[`../esp32/src/web_routes.h::answerRiddle()`](../esp32/src/web_routes.h) —
it just doesn't get reached today because no `type: "riddle"` entry exists
in the live `questions[]` array.

### 9.1 Riddle JSON shape

```json
{
  "id": 99,
  "type": "riddle",
  "q": "I have no mouth, but I hear every word. I have no ears, but I catch every sound. Speak to me, and the lights will dance. What am I?",
  "answers": ["mic", "microphone", "mike", "the mic", "a mic"],
  "success": {
    "msg": "You've activated the beat!",
    "action": "BEAT:on",
    "pattern": 7
  },
  "fail_msg": "Not quite… listen closer.",
  "max_attempts": 3,
  "reveal_msg": "The answer was the mic. 🎤"
}
```

- `q.success.pattern` is optional — if absent, only the action fires.
- `q.success.action` is forwarded verbatim to the Teensy as a bridge line,
  so it could be anything the Teensy handles. Today the only useful value
  is `BEAT:on`.
- Numbers above 90 are reserved for riddles by convention; binary questions
  take 1+.

---

## 10. Behaviour matrix with beat on

Extends the live-today matrix in [ESP32-Web-App.md §5.3](./ESP32-Web-App.md)
with the BEAT-on rows:

| Switch state | Last web command | Link alive? | BEAT state | Result |
|---|---|---|---|---|
| any | none received | n/a | **on** (after mic re-enable) | 3-switch behaviour, brightness pulses to mic |
| unchanged since override engaged | within last 30 s | yes | **on** (after mic re-enable) | Web-selected pattern, brightness pulses to mic |

Beat state is **independent of all three override-down gates** (30 s
timeout, link-liveness, switch change): once `BEAT:on` has fired (after the
mic is wired and the riddle solved), it stays on until reboot.

---

## 11. Manual test flow (extended)

After §2 (revival):

1. Wire MAX4466 OUT → A1, VCC → 3.3V, GND → GND. Reflash Teensy with the
   `beatdetect` lines uncommented.
2. Move the riddle entry from `_disabled_questions[]` back into
   `questions[]` in `data/questions.json`. Run
   `pio run -d esp32 -t uploadfs`.
3. Join the AP, take over, type `mic` when the riddle shows → "you've
   activated the beat 🎤". Clap near the mic — brightness pulses.
4. Continue with binary questions; riddle no longer appears.
5. After 30 s of no taps, switch pattern resumes but brightness keeps
   pulsing to the mic (beat is sticky).
6. Repeat on a second phone — riddle is gone for them; beat is on.

Reset beat by power-cycling the ESP32 (and Teensy — `beatdetect::enabled`
lives in Teensy RAM).

---

## 12. Open decisions (for this path)

1. **Riddle weight** — 70% during pre-activation may be too dominant if the
   crowd is large and turning over quickly. Drop to 50% (in
   `RIDDLE_PREFER_PCT`) if everyone gets the same first card.
2. **Beat-detection tuning** — envelope decay (0.93) and threshold (1.5×)
   are bench guesses. Validate at venue volume; tune `BEAT_THRESHOLD` in
   [`../src/beatdetect.cpp`](../src/beatdetect.cpp) and the MAX4466 trim
   pot per §7.

---

## 13. Future bridge commands (deferred even further)

Reserved tokens that aren't in this branch but make sense alongside the
mic + riddle work:

| Command | Purpose |
|---|---|
| `BEAT:mode:pulse` / `BEAT:mode:strobe` | Per-pattern beat reactions (soft pulse vs hard strobe). |
| `BEAT:sens:<0-100>` | Runtime sensitivity adjustment from the web UI without a reflash. |

Single on/off is enough until the crew wants per-pattern variation. The
bridge module is structured so new commands are one-line additions.
