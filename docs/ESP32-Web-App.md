# ESP32 Web App - Build Spec

**Scope of this addition:** ESP32-side firmware + Teensy-side Serial bridge
for the festival web-app proof-of-concept.

This document is the build spec for branch `feat/esp32-web-app`. It is the
source of truth when this branch is the one being developed against —
self-contained, with no external references required to build, flash, or
operate.

> **Deferred work lives in a sister doc.** The MAX4466 microphone,
> Teensy-side beat detection, and the riddle activation flow are designed
> but parked until the mic is physically wired. See
> [ESP32-Web-App-Mic-Riddle.md](./ESP32-Web-App-Mic-Riddle.md) for that
> spec + revival recipe. The codebase has the supporting hooks
> (`beatdetect.{h,cpp}` on disk, `_disabled_questions[]` slot in
> `questions.json`, parsed-but-ignored `BEAT:` commands in
> `webcontrol.cpp`) so the revival is uncommenting three sites + a JSON
> move + a solder, not a rewrite.
>
> **`DEBUG_BRIDGE` is currently `1`** in
> [`../esp32/src/config.h`](../esp32/src/config.h) (dev). Flip to `0` per
> §9.3 before festival.

---

## If you're new here

Read these five sections first — together they're maybe 10 minutes and
cover everything you need before touching code:

1. **§0** — what hardware this firmware drives (the installation overview).
2. **§0.1** — what existed on the Teensy *before* this branch.
3. **§1** — the headline table: what this branch adds.
4. **§3.2.1 + §3.2.2** — why the QR sign is the canonical UX, not the
   captive-portal popup. Crucial context before any web-app changes.
5. **§8.4** — the gotcha catalogue. Reading it once saves hours of
   re-discovery (pyserial DTR pulse, Pin 18 mod, CH340 baud cap, Android
   Private DNS, etc.).

Common tasks have step-by-step recipes in **§9**:
- Adding a new question to the pool → §9.1
- Adding a new LED pattern → §9.2
- Pre-festival hardening checklist → §9.3

For mic + riddle work see
[ESP32-Web-App-Mic-Riddle.md](./ESP32-Web-App-Mic-Riddle.md).

### High-level data flow

```
        WiFi (open AP)        Serial4 @ 115200 8-N-1        WS2811 @ 800 kHz
┌──────┐   GET /api/answer  ┌─────────┐   P:5\n  PING (5 s) ┌────────┐   parallel × 6 pins  ┌──────────┐
│ Phone│ ─────────────────→ │ ESP32   │ ──────────────────→ │ Teensy │ ───────────────────→ │  2,130   │
│      │                    │ ·SoftAP │                     │  4.1   │                      │  LEDs    │
│ tap! │ ←───────────────── │ ·DHCP   │ ←────────────────── │        │                      └──────────┘
└──────┘   response.msg     │ ·DNS    │   MODE:web\n        │        │
                            │ ·web    │   PATTERN:5\n       │        │
                            │  routes │   (every 500 ms)    │        │
                            │ ·bridge │                     │        │
                            └─────────┘                     └────────┘
```

The ESP32 owns the user-facing experience (WiFi, UI, question pool).
The Teensy owns the lights. The Serial4 bridge is the only coupling —
plain text, easy to debug with a monitor on either side. (When the
parked mic path is revived, the mic and beat detection also live on the
Teensy because they need to run inside the LED render loop with no
bus-crossing latency.)

---

## 0. About the installation

The firmware in this repository drives **Infinity / Meta Sonic Flux**, a 12-faced
dodecahedron LED installation built for festival deployment.

| Property | Value |
|---|---|
| Faces | 12 pentagonal faces |
| Strips | 30 inter-vertex "runs" (5 per dodecahedron segment × 6 segments) |
| LEDs per strip | 71 |
| Total LEDs | 2,130 |
| Driver MCU | Teensy 4.1 |
| LED driver | OctoWS2811 adapter, 6 parallel pins (8 available) |
| LED protocol | WS2811 @ 800 kHz, RGB byte order |
| Power | 12V external for LED strips (isolated). 5V via barrel jack shared by Teensy + ESP32 + fans. |

Per-pin/segment mapping (fixed by OctoWS2811 DMA hardware - `pinList[] = {2, 14, 7, 8, 6, 20}` in [`../src/main.cpp`](../src/main.cpp)):

| Index | Teensy pin | Segment array | Covers strips |
|---|---|---|---|
| 0 | 2  | `leds1` | 1–5   |
| 1 | 14 | `leds2` | 6–10  |
| 2 | 7  | `leds3` | 11–15 |
| 3 | 8  | `leds4` | 16–20 |
| 4 | 6  | `leds5` | 21–25 |
| 5 | 20 | `leds6` | 26–30 |

Each segment = 355 LEDs (5 strips × 71). Flat `CRGB leds[NUM_LEDS]` is the source
of truth; `leds1..leds6` are CRGBSet views into it.

### 0.1 Existing pre-branch state

Before this branch, the Teensy reads three switches on pins 0/1/23 with
`INPUT_PULLUP` and uses the 3-bit binary value as a flat mode index 0–7.
`selectMode()` in [`../src/main.cpp`](../src/main.cpp) dispatches via `switch (mode)`
to one pattern function per case (rainbowChase, fireFlies, twinkle+comets,
waveVerticalsOverwards, freePalestineFullBlink, rainbowFade, showLights,
spencerSparkle). There was no web bridge, no microphone, no resolver - just
switches → mode → pattern.

### 0.2 The festival web app

A self contained WiFi AP with a captive portal web app lets any phone in
range tap into the lights without an app install. The interaction is
binary Yes/No questions paired with light reactions — memorable,
low-friction, shareable, naturally varied, hard to grief (binary choices
are harder to abuse than an open colour picker). After every question has
been seen, the app transitions into a **fire-ritual finale**
(`dim_ambient` → `fire_full` → `idle_ambient`).

A future expansion adds a **mic + riddle** layer — one user solves a
riddle, the lights start breathing with the music for everyone, all at
once. That whole path is parked until the mic is wired; design lives in
[ESP32-Web-App-Mic-Riddle.md](./ESP32-Web-App-Mic-Riddle.md).

---

## 1. What this branch adds

| Area | Result on this branch |
|---|---|
| ESP32 firmware | New self-contained PlatformIO project at [`../esp32/`](../esp32/). SoftAP + 4-layer captive-portal stack (RFC 8910 DHCP-114, DNS catch-all, HTTP redirects, SSID-as-hint) + HTTP web server + Serial2 bridge to Teensy. |
| Festival UX | **Primary**: printed QR code at the installation (§3.2.2). The firmware captive-portal mechanisms are best-effort backup for devices that honor them - see §3.2.1 for why no firmware fix is universal. |
| Web app | Vanilla HTML/CSS/JS in `esp32/data/`, served from LittleFS. Takeover gate + binary Yes/No questions + fire-ritual finale. Free-text **riddle** type is implemented in firmware but parked (see [mic+riddle doc](./ESP32-Web-App-Mic-Riddle.md)). |
| Question pool | JSON file in LittleFS, easy to expand without firmware rebuild. Two entry types: `binary` (Yes/No, live) and `riddle` (free text, parked). The pool also supports a parallel `_disabled_questions[]` array for parked entries — firmware reads only `questions[]`. |
| Teensy bridge | New [`webcontrol.h`/`.cpp`](../src/) module that listens on Serial4, selects a pattern via 30 s timeout override, gates overrides on a **5 s PING liveness signal** from the ESP, and broadcasts `MODE:` + `PATTERN:` to the ESP every 500 ms. `BEAT:on/off` is parsed but silently ignored (mic parked). |
| Resolver | **Minimal** - see §7 for the full priority model this branch defers. Switches still drive operator mode; web override expires after 30 s of no commands OR after the bridge link goes silent for ~12 s OR the moment any switch moves while an override is engaged. |
| Pattern split | App-only patterns now live in [`../src/app/app_patterns.{h,cpp}`](../src/app/) with named constants (`DIM_AMBIENT` … `FOLD`, `CALIBRATE`) so `runPattern()` and `webcontrol.cpp`'s named-command parser can't drift. Operator (0–7) vs app (8–15) banks share the same flat index space; full operator/public bank refactor still deferred (see §7). |

---

## 2. Hardware required for this branch

| Item | Purpose | Status |
|---|---|---|
| Teensy 4.1 | Existing - drives LEDs | Already on bench |
| OctoWS2811 adapter | Existing - 6 of 8 parallel pins used | Already on bench |
| ESP32-WROOM-32 dev board | WiFi + web serving | Required on the bench |
| 3 jumper wires | Serial4 bridge (TX, RX, GND) | Required |
| 5V supply (shared) | Optional during bench test (USB on each board is fine if you obey the §2.3 warning) | - |
| **Hardware mod** | OctoWS2811 Pin 18 trace cut + Pin 16 jumper (§2.1) | Required to receive web commands |

The MAX4466 mic for beat detection is *not* part of this branch's BOM —
see [ESP32-Web-App-Mic-Riddle.md](./ESP32-Web-App-Mic-Riddle.md) for that
hardware once the path is revived.

### 2.1 Required Teensy hardware modification

The OctoWS2811 adapter board does not break Teensy Pin 16 out to its own labelled
header pad. Serial4 RX lives on Pin 16. To make Serial4 RX reachable on a
stacked Teensy 4.1 + OctoWS2811 assembly:

1. On the **underside** of the OctoWS2811 board, cut the trace running from the
   "Pin 18" breakout pad to the Teensy Pin 18 pin.
2. Run a short jumper wire from **Teensy Pin 16** to the now-isolated "Pin 18"
   silkscreen pad.

After the mod the silkscreen still says "Pin 18" but the pad is electrically
**Teensy Pin 16 (Serial4 RX)**. **Pin 18 itself becomes unusable as I/O on this
assembly.**

Verified safe for this installation: the current `pinList = {2, 14, 7, 8, 6, 20}`
in [`../src/main.cpp`](../src/main.cpp) does not use Pin 18, so no pattern is
affected. Confirm before cutting if pattern code has been changed.

**You can defer the trace cut and still test most of this branch:**

- ESP32 firmware + web app: works standalone — connect a phone, see the
  captive portal, tap Yes/No. The ESP32 will log the commands it tried to
  send to the Teensy over its USB serial.
- Teensy receive path: needs the trace cut. Without it, the Teensy's status
  broadcasts on TX (Pin 17) still work, but it cannot receive web commands.

### 2.2 Wiring (this branch)

| Teensy 4.1 | ESP32-WROOM-32 | Note |
|---|---|---|
| Pin 17 (TX4) | ESP32 GPIO16 (RX2) | Teensy → ESP32 status. Always cross TX↔RX. |
| Pin 16 (RX4, via Pin 18 pad) | ESP32 GPIO17 (TX2) | ESP32 → Teensy commands. Requires §2.1 mod. |
| GND | ESP32 GND | Common ground required (Serial bridge) |

Mic wiring (A1 + 3.3V + GND) is in
[ESP32-Web-App-Mic-Riddle.md §3.2](./ESP32-Web-App-Mic-Riddle.md).

Never connect TX→TX or RX→RX - always cross. Never use ESP32 GPIO1/GPIO3
(TX0/RX0) for the bridge - those are shared with the USB programming chip.

Powering: bench-test with each board on its own USB. Festival deployment uses
the shared 5V rail described in §2.3.

### 2.3 Power architecture (operational deployment)

LED power and logic power are physically isolated supplies that share only a
single common-ground tie point at one location in the LED strip.

| Supply | Powers | Notes |
|---|---|---|
| 12V external | WS2811 LED strips only | Sized for full LED load. Never enters the controller box. |
| 5V via barrel jack | Teensy VIN, ESP32 VIN, 2× fans | Shared rail; combined load ~420 mA |

> **If using a separate power source**
> 
> ⚠ **Cut the VIN-VUSB pad on the underside of the Teensy 4.1 before connecting
> external 5V.** This prevents back-feeding power into a USB port during
> programming. The ESP32 dev board has its own onboard diode protection.
>
> ⚠ **Never connect USB and external 5V at the same time** unless the
> VIN-VUSB pad has been cut. Two power sources fighting on the same rail can
> damage the Teensy, the laptop USB port, or both. When programming,
> disconnect the external 5V supply first. When the external supply is
> connected, leave USB unplugged.
>
> ⚠ **Don't connect both USB cables (Teensy Micro-USB + ESP32 USB-C) at the
> same time** while the boards share a common 5V/GND rail. Risks ground-loop
> currents through your laptop's USB ports and programming failures. Flash
> one board, unplug, flash the other.

---

## 3. ESP32 firmware architecture

```
ArduinoPlayground/esp32/
├── platformio.ini       # env:esp32dev, framework=arduino, LittleFS image
├── src/
│   ├── main.cpp         # setup() + loop() - wires modules
│   ├── wifi_ap.h        # SoftAP + DNSServer captive portal
│   ├── web_routes.h     # HTTP routes (UI + /api/*) — riddle handler present but parked
│   ├── bridge.h         # Serial2 framing (send commands, parse status)
│   ├── questions.h      # Loads questions.json into RAM; type-aware picker
│   └── config.h         # AP name, IPs, baud, timeouts, etc.
└── data/                # uploaded to LittleFS via `pio run -t uploadfs`
    ├── index.html
    ├── style.css
    ├── app.js
    └── questions.json
```

### 3.1 Boot sequence

1. `Serial.begin(115200)` - USB-serial debug (free for monitoring)
2. `Serial2.begin(115200)` on GPIO16/17 - bridge to Teensy
3. `LittleFS.begin()` - mount filesystem
4. Load `/questions.json` into a small in-RAM array
5. `WiFi.softAP("Enter Infinity", nullptr)` - open AP, no password.
6. `dnsServer.start(53, "*", AP_IP)` - wildcard DNS captive portal
7. Register HTTP routes (§3.3), `server.begin()`
8. `loop()`: `dnsServer.processNextRequest(); server.handleClient(); bridge.poll();`

### 3.2 Captive portal endpoints

iOS, Android, and modern Windows all probe a known URL on AP-join to decide
whether to show a "Sign in to network" prompt. Catch all of them and respond
with a **302 redirect to root + `Cache-Control: no-store` headers**. This is
the most universal trigger pattern across modern devices - more reliable than
serving HTML inline, and the no-cache headers stop the OS from reusing a
"no captive portal" verdict it cached on a previous network.

| OS probe | Endpoint to catch |
|---|---|
| Apple | `/hotspot-detect.html`, `/library/test/success.html` |
| Android | `/generate_204`, `/gen_204` |
| Microsoft | `/ncsi.txt`, `/connecttest.txt`, `/redirect` |
| Firefox / Ubuntu | `/success.txt`, `/canonical.html` |
| Catch-all (`onNotFound`) | Any unknown host → redirect to `http://192.168.4.1/` |

Each probe handler logs the request (`[captive] probe: <uri> host=<x> ip=<y>`)
to USB serial so it's possible to see during testing whether the phone is
actually reaching us.

### 3.2.1 Captive-portal auto-open: best-effort, not guaranteed

**There is no firmware-only solution that makes the captive popup open on
100% of devices.** This is a well known OS vendor mess; every
commercial captive portal (hotels, airports, coffee shops) has the same
problem and works around it with signage. We've stacked four mechanisms so
the popup *does* fire on as many devices as possible, but the **printed QR
code on the installation is the canonical UX** - see §3.2.2.

Mechanisms in the firmware, in modern → legacy order:

1. **RFC 8910 - DHCP option 114** (`ESP_NETIF_CAPTIVEPORTAL_URI`, in
   `wifi_ap.h`). The DHCP lease itself carries the portal URL, so the OS
   never has to do an HTTP probe. **No DNS involved - bypasses Private DNS
   entirely.** Honored by iOS 14+, macOS 12+, Android 11+ (manufacturer
   skin permitting), Windows 11. This is the modern correct way to do this.
2. **Wildcard DNS catch-all** (`DNSServer` in `wifi_ap.h`) - every name
   lookup resolves to 192.168.4.1, so the next-fallback HTTP-based captive
   probe also lands on us.
3. **HTTP 302 redirect with no-cache headers** on the classic probe paths
   (`web_routes.h`) - triggers OSes that didn't honor DHCP-114 and
   completed the DNS step.
4. **Printed QR sign at the installation** (§3.2.2). The very last fallback
   for anyone who got past all three above with no popup: a plain-URL QR
   code pointing at `http://192.168.4.1/`. Typing/pasting an IP into a
   browser doesn't touch DNS, so it works under any OS condition.

What can still defeat all four:

- **Android with manufacturer-specific captive-portal handling** (some Samsung
  / Xiaomi / OPPO skins ignore DHCP-114 *and* require successful HTTPS
  verification before opening the popup; Private DNS breaks that verification
  step → no popup). Affected users won't see any prompt. They CAN still join
  the AP manually and tap the URL/SSID.
- **Older devices** (pre-iOS 14, pre-Android 11) that predate DHCP-114 and
  may have other quirks.
- **Anyone who's joined our AP before and Android cached "no captive portal"**
  for the SSID until they "Forget network".

For these users, the QR sign is the answer.

### 3.2.2 Festival UX - printed QR + sign

This is the canonical interaction we expect 90%+ of festival-goers to use:

```
┌─────────────────────────────────────────┐
│                                         │
│  ╔════════════╗     ╔════════════╗      │
│  ║            ║     ║            ║      │
│  ║   WiFi     ║     ║    Open    ║      │
│  ║   QR       ║     ║    QR      ║      │
│  ║            ║     ║            ║      │
│  ╚════════════╝     ╚════════════╝      │
│                                         │
│   1. Scan ←          2. Scan ←          │
│                                         │
│  No popup? Open  192.168.4.1  manually  │
│                                         │
└─────────────────────────────────────────┘
```

**Left QR - WiFi join.** Encodes the SSID using the standard `WIFI:` schema.
Every phone camera since ~2018 recognises it and prompts "Join Wi-Fi?".

```
WIFI:T:nopass;S:Enter Infinity;;
```

**Right QR - open the portal.** Plain URL. After WiFi join, this opens the
portal in the user's default browser, bypassing all captive-portal mechanisms.

```
http://192.168.4.1/
```

Sign size: A5 or larger so the QRs are scannable from ~1 m. Print on
weatherproof vinyl, mount on the installation at chest height.

Two QRs (rather than one combined) because no QR schema natively combines
"join WiFi + then open URL" with reliable cross-platform support. Two scans,
two seconds - works on every device.

### 3.3 HTTP routes

| Method | Path | Purpose |
|---|---|---|
| GET | `/` | Serve `index.html` from LittleFS |
| GET | `/style.css`, `/app.js` | Static assets |
| GET | `/api/question` | Return a random question. Accepts `?seen=1,2,…` (comma-separated qids the client has already answered) to exclude them. Server-side weight prefers a riddle while `beatActivated` is false. Returns `{"exhausted":true}` when every eligible question has been seen — client triggers the fire-ritual finale. |
| POST | `/api/answer` | Body: `{"qid":N,"answer":"yes"\|"no"\|"<free text>"}`. Dispatches by question type - see §3.5. Per-IP rate-limited (`PER_IP_COOLDOWN_MS = 2000` in `esp32/src/config.h`). |
| GET | `/api/state` | Returns `{mode, pattern, bridge_ms_since, beatActivated, teensyBeat}`. `bridge_ms_since` = ms since the last bridge frame from the Teensy (-1 if none yet). |
| GET | `/api/stats` | Cumulative yes/no tallies + `beatActivated` + `beatActivations` count. |
| POST | `/api/takeover` | Phone "take over the lights" gate. Sends `P:<lastTeensyPattern>` to engage the web override at the currently-showing pattern (nothing visibly changes). Fired by the app after a client-side 3 s countdown. If the Teensy hasn't reported a pattern yet (-1), nothing is sent and the override engages on the first answer instead. |
| GET | `/command?cmd=<allowed>` | Strict allowlist for fire-ritual stage commands. Accepts only `P:dim_ambient`, `P:fire_full`, `P:idle_ambient`, `P:calibrate` (bench-only). Anything else → 400. New control flows get their own endpoint, not this one. |

**Takeover + instant reclaim.** The app boots into a "Take over the lights" gate;
tapping it runs a 3 s countdown (client-side; the lights keep their current
pattern) then POSTs `/api/takeover` to claim control and starts the question
flow. The operator reclaims **instantly**: `selectMode()` snapshots the switch
position when an override engages and calls `webcontrol::cancelOverride()` the
moment the switches change. The 30 s timeout remains only as a safety backstop.

State sync is via short HTTP polling (~10 s for stats; clients don't poll
state today), not SSE - simpler with the built-in sync `WebServer`, adequate
for the bench test, and easier to debug. SSE is on the deferred list (§7).

### 3.4 Serial bridge framing

Plain-text, newline-delimited. Easy to debug with a serial monitor on either
side. This branch implements a subset of the framing.

**ESP32 → Teensy** (`P:` accepts either an integer or a named app-pattern token):

```
P:<n>\n             Pattern index (web override, 30 s timeout on Teensy)
P:dim_ambient\n     Named alias for app_patterns::DIM_AMBIENT  (= 8)
P:fire_full\n       Named alias for app_patterns::FIRE_FULL    (= 9)
P:idle_ambient\n    Named alias for app_patterns::IDLE_AMBIENT (= 10)
P:calibrate\n       Named alias for app_patterns::CALIBRATE    (= 15)
Q:yes\n             Shorthand: trigger the "yes" reaction (mapped server-side)
Q:no\n              Shorthand: trigger the "no" reaction
PING\n              Liveness probe — sent every 5 s; required for overrides
```

Indices outside `0..app_patterns::CALIBRATE` (currently 15) and non-numeric,
non-named tokens are rejected on the Teensy side — that keeps stray bytes on
a floating Serial4 RX from hijacking the switches when the ESP is unplugged.

**Teensy → ESP32** (broadcast every 500 ms):

```
MODE:operator   Switches drive the pattern (no live web override)
MODE:web        Web override is active and not timed out
PATTERN:<n>     Currently running pattern index
```

**Link liveness.** The Teensy only honours web overrides while the ESP's
PINGs keep arriving — last PING within `LINK_ALIVE_MS` (12 s, ~2 missed
5 s pings). Without this gate the Serial4 RX pin floats when the ESP is
unplugged and reads noise, and any noise byte sequence that happened to
parse as a valid command would hijack the operator switches.

`BEAT:on/off` (ESP → Teensy) and `BEAT:<0|1>` (Teensy → ESP) are reserved
for the parked mic + riddle path — see
[ESP32-Web-App-Mic-Riddle.md §4](./ESP32-Web-App-Mic-Riddle.md). The Teensy
parses-and-ignores the inbound forms today; the outbound broadcast is
suppressed.

Reserved for future commands (not implemented on this branch): `C:#RRGGBB`,
`S:<0-100>` (speed), `B:<0-100>` (brightness). They will be added as the
web app grows; the bridge module is structured so new commands are
one-line additions.

### 3.5 `/api/answer` dispatch by type

The endpoint reads the question's `type` field and routes:

- **binary** (default if `type` is absent): expects `"yes"` or `"no"` as
  the answer. Looks up `q.yes` / `q.no` branch, forwards `P:<n>` to the
  Teensy, responds with the matching `msg`. This is the live path.
- **riddle**: free-text question type. Implemented in `web_routes.h` but
  parked — no `type: "riddle"` entries exist in the live `questions[]`
  array. Full description in
  [ESP32-Web-App-Mic-Riddle.md §9](./ESP32-Web-App-Mic-Riddle.md).

### 3.6 Question pool format

`data/questions.json` (current `version: 4`):

```json
{
  "version": 4,
  "_note": "Pattern indices map to runPattern() in main.cpp.",
  "_disabled_note": "Parked entries live here; firmware reads only questions[].",
  "_disabled_questions": [ /* riddle entry lives here while parked */ ],
  "questions": [
    {
      "id": 1,
      "type": "binary",
      "q": "Would you still love me if I was a worm?",
      "yes": { "msg": "Love is all the colours! Slurp the rainbow.", "pattern": 11 },
      "no":  { "msg": "You broke my heart. Bye.",                    "pattern": 12 }
    },
    {
      "id": 2,
      "type": "binary",
      "q": "Do you fold or scrunch?",
      "yes": { "label": "Fold",    "msg": "The correct answer.", "pattern": 14 },
      "no":  { "label": "Scrunch", "msg": "You dirty turd!",     "pattern": 13 }
    }
  ]
}
```

- `type` defaults to `binary` if missing (backwards-compatible).
- `pattern` is an int index into the Teensy's `runPattern(n)` table
  (see §4.4 for the index → function mapping).
- Binary branches accept an optional `label` (defaults to "Yes" / "No") so
  a question can show e.g. "Fold" / "Scrunch" on the buttons.
- `_disabled_questions[]` is a parked-entries holding pen. The firmware
  ignores it; move an entry back into `questions[]` to re-enable it (and
  re-run `pio run -t uploadfs`). The riddle currently lives there — see
  [ESP32-Web-App-Mic-Riddle.md](./ESP32-Web-App-Mic-Riddle.md) for the
  full entry + dispatch semantics.

Target pool: 40–60 questions at festival time, weighted ~70 % absurd, ~20 %
festival-specific, ~10 % wholesome. Adjustable without firmware rebuild -
just edit `data/questions.json` and re-run `pio run -t uploadfs`.

---

## 4. Teensy-side changes (this branch)

### 4.1 New files

- [`../src/webcontrol.h`/`.cpp`](../src/) — Serial4 read loop, fixed-buffer
  line parser (128-byte `char[]`, no `String` heap churn), 30 s
  pattern-override timeout, PING-gated link liveness, 500 ms status
  broadcast. `BEAT:on/off` is parsed-and-ignored today (see
  [mic+riddle doc](./ESP32-Web-App-Mic-Riddle.md)).
- [`../src/app/app_patterns.{h,cpp}`](../src/app/) — App-only patterns,
  separate from the operator/button bank. Holds the fire-ritual stages
  (`dimAmbient` / `fireFull` / `idleAmbient`), the question reactions
  (`wormYes` / `wormNo` / `scrunch` / `fold`), and the bench-only
  `calibrate()`. The pattern-index constants
  (`app_patterns::DIM_AMBIENT` … `FOLD`, `CALIBRATE`) are the single
  source of truth — both `runPattern()` and `webcontrol.cpp`'s
  named-command parser reference them.
- [`../src/beatdetect.h`/`.cpp`](../src/) — A1/Pin 15 analog amplitude
  detector. **On disk but disabled** — `#include` + `begin()` + `poll()`
  in `main.cpp` are commented out. See
  [ESP32-Web-App-Mic-Riddle.md](./ESP32-Web-App-Mic-Riddle.md) for the
  algorithm + revival recipe.

### 4.2 Modified files

- [`../src/main.cpp`](../src/main.cpp) —
  - `webcontrol::begin()` in `setup()`. `beatdetect::begin()` is commented
    out alongside its include and `beatdetect::poll()` until the mic lands.
  - A master `WEB_OVERRIDE_ENABLED` flag (default **true**) gates the
    whole web-override dispatch. Set it false to lock the web app out of
    pattern selection entirely — the Serial4 status bridge keeps running
    either way. It is **independent of the LED ghost-pixel issue**
    (that's grounding, not this flag — see §8.4).
  - Extracted the existing pattern dispatch into a `runPattern(int idx)`
    function so both the switch-driven operator path and the web-override
    path use the same index → function mapping. Cases 8–15 use the named
    constants from
    [`../src/app/app_patterns.h`](../src/app/app_patterns.h).
  - In `selectMode()`: if `webcontrol::overrideActive()`, dispatch on
    `webcontrol::overridePattern()` instead of the switch-derived mode.
    Otherwise fall through to switch behaviour. The local `switchMode`
    builds from `digitalRead(switchPins[i]) << i`; the old top-level
    `mode` global was removed.
  - `FastLED.setBrightness(255)` is set inline each frame. The mic-driven
    brightness-decay path is removed for now; restore alongside
    re-enabling `beatdetect`.

### 4.3 Behaviour matrix

| Switch state | Last web command | Link alive (PING < 12 s)? | Result |
|---|---|---|---|
| any | none received | n/a | 3-switch behaviour, full brightness |
| **unchanged since override engaged** | within last 30 s | yes | Web-selected pattern overrides, full brightness |
| **changed since override engaged** | within last 30 s | yes | `cancelOverride()` fires; switch pattern resumes instantly |
| any | within last 30 s | **no** (ESP unplugged / silent) | Override ignored — switches drive the pattern |

Three independent gates take the web override down: the 30 s
pattern-override timeout, the ~12 s link-liveness timeout
(`LINK_ALIVE_MS` — gates on the ESP's 5 s PINGs), and any operator
switch change (instant reclaim via `webcontrol::cancelOverride()`). All
three are stand-ins for the full priority resolver (see §6).

For the additional BEAT-on rows that apply once the mic + riddle path is
revived, see
[ESP32-Web-App-Mic-Riddle.md §10](./ESP32-Web-App-Mic-Riddle.md).

### 4.4 Pattern index → function mapping

Single source of truth: `runPattern(int)` in [`../src/main.cpp`](../src/main.cpp).
Indices 0–7 are the operator/button bank (shared with the 3-switch dispatch).
Indices 8–15 are app-only - only reachable from the web app or the
`/command` allowlist. The 8–15 constants live in
[`../src/app/app_patterns.h`](../src/app/app_patterns.h) so the
`webcontrol.cpp` parser and `runPattern()` can't drift. Anything out of range
falls through to `galaxy()` and `webcontrol.cpp` rejects indices > `CALIBRATE`
(currently 15) before the dispatch ever sees them.

| Index | Function | Contributor / Where |
|---|---|---|
| 0 | `rainbowChase()` | conor |
| 1 | `fireFlies()` | conor |
| 2 | `twinkle() + comets()` | denise |
| 3 | `waveVerticalsOverwards()` | denise |
| 4 | `freePalestineFullBlink()` | dee |
| 5 | `rainbowFade()` | conor |
| 6 | `showLights()` | dee |
| 7 (default) | `galaxy()` | spencer |
| 8 (`DIM_AMBIENT`) | `dimAmbient()` | app - fire-ritual transition |
| 9 (`FIRE_FULL`) | `fireFull()` | app - fire-ritual fire |
| 10 (`IDLE_AMBIENT`) | `idleAmbient()` | app - fire-ritual post-finale |
| 11 (`WORM_YES`) | `wormYes()` | app - Q1 "love is all the colours" |
| 12 (`WORM_NO`) | `wormNo()` | app - Q1 blackout |
| 13 (`SCRUNCH`) | `scrunch()` | app - Q2 "you dirty turd" |
| 14 (`FOLD`) | `fold()` | app - Q2 "the correct answer" |
| 15 (`CALIBRATE`) | `calibrate()` | app - **bench-only** per-strip wiring-direction read-off (red→blue HSV gradient). Trigger from a phone via `http://192.168.4.1/command?cmd=P:calibrate`. Read-off instructions in the function body. |

`data/questions.json` references these by integer. Re-curate by editing the
JSON — no firmware rebuild needed. Adding a NEW app pattern: see §9.2.

---

## 5. Build & flash

### 5.1 Teensy

From [`../`](../) (the project root):

```bash
pio run                    # build
pio run -t upload          # flash via Teensy Loader
pio device monitor -b 9600 # debug serial (USB) - only when DEBUG_MODE = true
```

Toolchain: PlatformIO with `platform = teensy`, `board = teensy41`,
`framework = arduino`, libs `fastled/FastLED@^3.6.0`,
`paulstoffregen/OctoWS2811@^1.5`.

### 5.2 ESP32

From [`../esp32/`](../esp32/):

```bash
pio run                          # build firmware
pio run -t uploadfs              # upload LittleFS image (data/)
pio run -t upload                # flash firmware
pio device monitor -b 115200     # USB serial debug
```

**Run `uploadfs` before `upload`** when changing `data/` files - they are
flashed to a separate partition.

> **Gotcha - `questions.json` edits don't ship with a firmware flash.**
> Anything under `esp32/data/` (including `questions.json`, `app.js`,
> `index.html`, `style.css`) lives on the LittleFS partition and is *only*
> updated by `pio run -t uploadfs`. A plain `pio run -t upload` reflashes
> firmware but leaves the data partition untouched, so the device keeps
> serving whatever was on flash from the previous `uploadfs`. Symptom: you
> add/edit a question, flash, and the web app still shows the old pool.
> Fix: `pio run -d esp32 -t uploadfs`.

Toolchain: `platform = pioarduino` (community fork of `espressif32`, required
for ESP-IDF 5.1+ which exposes `ESP_NETIF_CAPTIVEPORTAL_URI` for RFC 8910 -
see §3.2.1), `board = esp32dev`, `framework = arduino`,
`board_build.filesystem = littlefs`, lib `bblanchon/ArduinoJson @ ^7.0.4`,
build flags `-std=gnu++17` (needed for the inline-variable namespaces). The
pioarduino fork pulls in a larger ESP-IDF (firmware ~80% flash vs ~64% on
the stock platform) but stays well within the partition.

### 5.3 Manual test flow

1. Power both boards (USB is fine for bench — see §8.4 ground-loop note).
2. On a phone, join WiFi `Enter Infinity`.
3. Captive portal should auto-open. If not, browse to
   `http://192.168.4.1/`.
4. Tap "Take over the lights" → 3 s countdown → question card appears.
5. Answer Q1 (worm Yes → rainbow worm pattern; No → blackout).
6. Answer Q2 (Fold → soft white breathing; Scrunch → muddy brown).
7. After both qids are seen, the client receives `{"exhausted":true}` and
   transitions into the fire-ritual finale stages (`dim_ambient` →
   `fire_full` → `idle_ambient`), each forwarded via `/command`.
8. Move any operator switch — the web override drops instantly and the
   switch pattern resumes. Or wait 30 s of no taps for the same effect.
9. Unplug the ESP USB — within ~12 s the Teensy stops honouring overrides
   even if a stale command timer is still running (link-liveness gate).

For the extended flow that exercises the mic + riddle path, see
[ESP32-Web-App-Mic-Riddle.md §11](./ESP32-Web-App-Mic-Riddle.md).

---

## 6. Open decisions (resolve before merging to `main`)

1. **Print and mount the QR sign** (§3.2.2). This is the actual primary
   UX. Generate the two QR codes from the strings in §3.2.2, lay out at
   A5+, print on weatherproof vinyl.
2. **Question pool curation** — `data/questions.json` ships with a
   starter set; needs the crew's editorial pass.
3. **Pattern mapping for Yes/No** — the seed mapping is in
   `questions.json`, not hard-coded. Verify the mapped indices match the
   running flat bank when testing.
4. **External antenna** — antenna range inside the (eventually
   aluminium) enclosure cannot be validated on this branch; needs an
   on-enclosure test before festival. If range is insufficient, drill a
   3 mm hole near the ESP32 antenna end and fit a U.FL → SMA pigtail
   with an external stub.

Decisions specific to the parked mic + riddle path (riddle weight, beat
tuning) live in
[ESP32-Web-App-Mic-Riddle.md §12](./ESP32-Web-App-Mic-Riddle.md).

---

## 7. Deferred to later phases

Called out so later work doesn't surprise anyone. The **mic + riddle**
work has its own doc:
[ESP32-Web-App-Mic-Riddle.md](./ESP32-Web-App-Mic-Riddle.md).

| Item | Description | Why deferred |
|---|---|---|
| Full priority resolver | A single `resolver.h` module that arbitrates three input domains (operator buttons, mic-driven beat, web app) into one `(patternIndex, modifiers)` per frame. Operator always wins (priority 1), beat next (2), web lowest (3). | Needs momentary-button hardware change first; today's 30 s timeout + link-liveness + switch-change reclaim are the placeholders. |
| Operator/public pattern bank split | Two separate banks: `operatorPatterns[]` (blackout, full white, slow pulse — for setup/work-light) and `publicPatterns[]` (rainbow, fire, etc. — for the show). | No code benefit until the resolver lands. |
| Switches → momentary buttons | Replace the 3 toggle switches with 3 momentary buttons. Firmware will need debounce + edge detection + the 30 s timeout already in place. | Hardware change; current firmware tolerates either since web override has its own timeout. |
| SSE for real-time state sync | Server-Sent Events from ESP32 to all connected clients so phones see live state without polling. | Polling is adequate for MVP; SSE moves in once the protocol stabilises and we're sure of multi-client behaviour. |
| Per-IP cooldown beyond 2 s | Web app rate-limits per phone IP at 2 s today. May need extending to 5 s+ in a dense crowd. | Tune at venue based on crowd behaviour. |
| Live "73 % of people would…" stat | Shareable live stat under each Yes/No answer. | Easy follow-up; current branch already has the tally endpoint stubbed at `/api/stats`. |
| Antenna pigtail mod | Drilled U.FL → SMA pigtail for an external antenna if the enclosure attenuates too much. | Decided after on-enclosure range test. |

---

## 8. Diagnostics & Testing

This branch was built and validated using a small set of helper scripts in
[`../tools/`](../tools/). They're not part of the firmware - they're bench
tools that capture the gotchas we hit during bring-up so the same time
doesn't get re-spent on the next session.

### 8.1 Tools

| Script | Purpose |
|---|---|
| [`tools/serial_helper.py`](../tools/serial_helper.py) | Shared module: port discovery, passive serial open (won't reset the chip), `hard_reset_esp32()` recovery via esptool |
| [`tools/monitor.py`](../tools/monitor.py) | Timestamped serial monitor, alternative to `pio device monitor`. Opens without disturbing the chip's run state. |
| [`tools/test_bridge.py`](../tools/test_bridge.py) | 20 s capture → PASS/FAIL on the Teensy ↔ ESP32 Serial4 bridge **round-trip**. Requires a phone tap during the window. Use when you want end-to-end proof. |
| [`tools/check_bridge.py`](../tools/check_bridge.py) | 15 s **passive** sniff of bridge traffic (no phone tap needed). Counts Teensy MODE / PATTERN broadcasts + ESP PINGs and flags corruption. Good pre-flight to rule out ground faults before powering LEDs. |
| [`tools/dev_server.py`](../tools/dev_server.py) | Local browser-side mock of every `/api/*` endpoint. Serves the same `esp32/data/` UI on `http://localhost:8080` so the question flow, takeover, and fire-ritual finale can be demoed and tested on a Mac without flashing hardware. Mirrors `RIDDLE_PREFER_PCT` and `PER_IP_COOLDOWN_MS` for parity with the parked riddle path. |
| [`tools/flash_both.sh`](../tools/flash_both.sh) | One command: Teensy firmware, then ESP32 LittleFS, then ESP32 firmware |

All scripts use PlatformIO's bundled Python (`~/.platformio/penv/bin/python3`)
which already has `pyserial` and `esptool` installed - no virtualenv to set up.

### 8.2 Workflows

**Fresh flash + smoke test:**

```bash
bash tools/flash_both.sh                                 # both boards
~/.platformio/penv/bin/python3 tools/monitor.py          # tail ESP32 USB
# Expect: "[wifi_ap] DHCP option 114 captive portal URI = ..."
# Expect: "[main] ready"
```

**Bridge round-trip (after solder rework, or any bridge change):**

```bash
# 1. Flip DEBUG_BRIDGE to 1 in esp32/src/config.h, reflash firmware:
cd esp32 && pio run -t upload --upload-port /dev/cu.usbserial-1430
# 2. Run the test:
~/.platformio/penv/bin/python3 tools/test_bridge.py
# 3. Tap any phone answer during the 20s window.
# 4. Read the PASS/FAIL verdict + RTT.
# 5. Flip DEBUG_BRIDGE back to 0 and reflash before festival.
```

### 8.3 `DEBUG_BRIDGE` flag

Defined in [`../esp32/src/config.h`](../esp32/src/config.h). When `1`, the
ESP32 logs every Serial bridge frame in both directions to USB serial:

```
[bridge → teensy] P:5
[bridge ← teensy] MODE:web
[bridge ← teensy] PATTERN:5
[bridge → teensy] PING
```

(`[bridge ← teensy] BEAT:0` lines would also appear here, but the Teensy
currently suppresses that broadcast — see §3.4.)

**Currently `1` for bench dev.** Flip to `0` for festival deployment — at
500 ms broadcast cadence × 2 lines per tick + a 5 s PING it's ~4 lines/sec of
serial noise that nobody needs. `tools/test_bridge.py` and
`tools/check_bridge.py` both require this flag to be on.

### 8.4 Gotchas catalogue

Painful things we hit during this branch. Reading this saves re-discovering
them.

| Gotcha | Symptom | Fix |
|---|---|---|
| Stock PlatformIO `espressif32` ships ESP-IDF 4.4, not 5.1+ | Boot log says `ESP-IDF < 5.1 - DHCP option 114 not available`; captive popup falls back to legacy DNS-only path | Use `pioarduino` fork - `platformio.ini` is already configured. |
| pyserial asserts DTR on `Serial()` open | ESP32 boots into `waiting for download` after running a Python script that opened its port | Open serial with `dsrdtr=False, rtscts=False, exclusive=True`. Use `serial_helper.open_passive()`. Recover via `serial_helper.hard_reset_esp32()`. |
| Two processes reading the same serial port | Garbled output like `[ridge ← teen` / partial bytes | Close all but one reader. Scripts use `exclusive=True` and report port-locked cleanly. |
| Teensy Loader on macOS can't find firmware on cold launch | `Teensy Loader could not find the file firmware`, upload aborts | Launch `~/.platformio/packages/tool-teensy/teensy.app` once before `pio run -t upload`. `flash_both.sh` does this automatically. |
| Cheap CH340 ESP32 boards can't sustain 921600 baud upload | `Invalid head of packet (0xE0): Possible serial noise or corruption` mid-flash | Set `upload_speed = 460800` in `esp32/platformio.ini` (already done). |
| USB-C-only ESP32 dev boards miss CC pull-down resistors | Board doesn't power on / enumerate with USB-C → USB-C cable (no `/dev/cu.usbserial-*` appears; `system_profiler` shows no CH340) | Use USB-A → USB-C cable. **Confirmed for our specific board** - USB-A is the only working source. Keep a USB-A → USB-C cable in the project box and don't bother trying C-to-C. |
| Both USBs plugged in while boards share GND via bridge wire | Bench is usually fine; in worst case ground loops cause programming failures | Bench OK without external 5V. Festival: shared external 5V rail and no USB connected. |
| Teensy 4.1 + OctoWS2811: Pin 16 (Serial4 RX) not broken out | ESP32 → Teensy commands never arrive; `MODE:web` never appears in bridge logs even after a phone tap | The Pin 18 trace cut + Pin 16 jumper mod from §2.1. If you've done the mod and it still fails, suspect cold solder joint on the jumper - that bit us once. |
| Scattered wrong-colour pixels whenever the bridge harness is wired | Random pixels flicker the wrong colour; present even with the web app idle **and** `WEB_OVERRIDE_ENABLED` off | **Grounding, not code** — don't chase it in software. Almost always a ground loop from the bridge GND wire (worst on the bench with two USB cables *plus* a shared bridge GND). Fix: power both boards from the single shared 5 V rail with one common ground (§2.3); add ~330 Ω in series on the data line; keep the RX jumper short and away from the data lines. `WEB_OVERRIDE_ENABLED` does **not** gate the bridge, so toggling it neither causes nor cures this. |
| Android Private DNS bypasses our DNS catch-all | Captive popup never opens on Android, even with DHCP-114 advertised | No firmware fix. Mitigation: printed QR sign - §3.2.1 / §3.2.2. |
| iOS / Android cache "no captive portal" per network | After fixing firmware, popup still doesn't appear on devices that connected before | Forget the network on the device, then rejoin. |

### 8.5 Quick port-detection cheatsheet

| Device | Path | Detected by |
|---|---|---|
| Teensy 4.1 (USB Serial mode) | `/dev/cu.usbmodem*` | VID:PID `16C0:0483` |
| ESP32 dev board (CH340) | `/dev/cu.usbserial-*` | VID:PID `1A86:7523` |

`serial_helper.find_teensy_port()` and `find_esp32_port()` glob those patterns.

---

## 9. Recipes

Step-by-step for the most common forward-looking jobs. Each recipe ends
with a "you're done when…" check so you can verify rather than hope.

Recipes for the parked mic + riddle path (wire the MAX4466, tune beat
sensitivity) live in
[ESP32-Web-App-Mic-Riddle.md §6–7](./ESP32-Web-App-Mic-Riddle.md).

### 9.1 Add a new question to the pool

1. Open [`../esp32/data/questions.json`](../esp32/data/questions.json).
2. Add an entry under `questions[]`:

   **Binary (Yes / No, or custom labels like Fold / Scrunch):**

   ```json
   {
     "id": 42,
     "type": "binary",
     "q": "Your question here?",
     "yes": { "label": "Yes",  "msg": "what they see on tap-yes", "pattern": 5 },
     "no":  { "label": "No",   "msg": "what they see on tap-no",  "pattern": 3 }
   }
   ```

   `label` is optional — defaults to "Yes" / "No" if omitted. `pattern`
   is an index into the Teensy `runPattern(int)` table — see §4.4.

   For the `riddle` shape, see
   [ESP32-Web-App-Mic-Riddle.md §9.1](./ESP32-Web-App-Mic-Riddle.md).

3. **`id` must be unique** across the file. Numbers above 90 are
   reserved for riddles by convention; binary questions take 1+.
4. Reflash the LittleFS image (firmware unchanged):

   ```bash
   cd esp32
   pio run -t uploadfs --upload-port /dev/cu.usbserial-1430
   ```

5. **Done when** you join the AP, force-refresh the page (or wait for
   the intermission), and the new question can appear.
   `tools/monitor.py` will log it as a `GET /api/question` returning
   your text.

### 9.2 Add a new LED pattern

The full pattern-authoring walkthrough lives in
[`../../CLAUDE/patterns.md`](../../CLAUDE/patterns.md). Pick the right
home for your new pattern:

- **Operator/button pattern** (reachable from the 3-switch dispatch) →
  add it to a contributor folder
  `src/<your-name>/<your-name>_patterns.{h,cpp}` and wire a `case 0..7`
  in `runPattern()`.
- **App-only pattern** (reachable only from the web app, not the
  switches) → add it to
  [`../src/app/app_patterns.{h,cpp}`](../src/app/) with a named
  constant in the `app_patterns::` namespace (e.g.
  `constexpr int MY_PATTERN = 16;` — index 15 is taken by `CALIBRATE`).
  Then add `case app_patterns::MY_PATTERN: myPattern(); break;` in
  `runPattern()` AND bump `MAX_PATTERN_INDEX` in `webcontrol.cpp`
  (currently `app_patterns::CALIBRATE`) if the new index is higher.
  The constant is the single source of truth — the `webcontrol.cpp`
  parser uses the same names for named-command mapping (`P:dim_ambient`
  etc.), so adding a new app pattern means **also** deciding whether to
  give it a named alias or just expose it by integer.

Short version, either path:

1. Function signature: `void myPattern()`, writes to the global
   `CRGB leds[NUM_LEDS]`, calls `FastLED.show()` at the end.
2. Include the right header in `main.cpp`.
3. Wire the `case` in `runPattern()`.
4. Reflash the Teensy.

To make the new pattern fire from the web app, reference its index in
`esp32/data/questions.json` (see §9.1) and re-run
`pio run -t uploadfs`. `runPattern(int)` remains the single source of
truth for "what does index N mean".

### 9.3 Pre-festival hardening checklist

Before deployment:

- [ ] `DEBUG_BRIDGE = 0` in
      [`../esp32/src/config.h`](../esp32/src/config.h) — no serial
      spam.
- [ ] AP SSID matches the printed QR sign
      (`Enter Infinity` by default).
- [ ] `data/questions.json` curated — crew has reviewed for tone /
      typos.
- [ ] Pin 18 trace cut + Pin 16 jumper verified with a multimeter
      (continuity from the silkscreen "Pin 18" pad to Teensy Pin 16,
      *not* Pin 18).
- [ ] Antenna range tested with the enclosure assembled (per §6 item 5).
- [ ] Two printed QR signs at the installation (WiFi join + portal URL).
- [ ] Spare USB-A → USB-C cable in the kit for the ESP32 (some boards
      need it — see §8.4).
- [ ] Spare pre-flashed ESP32 in the kit (drop-in replacement if it
      dies).
- [ ] Both firmwares burned to the production boards, *not* the bench
      boards.
- [ ] Power-cycle test: pull the 5V supply, replug, confirm everything
      boots cleanly within 10 s (DHCP, captive, bridge alive).
