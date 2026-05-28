# Infinity / Meta Sonic Flux

This is the code for **Infinity / Meta Sonic Flux** - a big 12-faced pentagonal
LED sculpture (a dodecahedron) built for festivals. There are **30 LED "runs"**
between the vertices, **2,130 LEDs** in total, and the whole thing is driven by
a **Teensy 4.1** running the FastLED library through an OctoWS2811 adapter.

There's also an optional **ESP32 sidecar** that turns it into something the
crowd can play with: it broadcasts an open WiFi network, and anyone who joins
gets a little web app on their phone - no install needed - where they answer
playful questions and watch the lights react. That whole side of things has
its own deep-dive doc: [docs/ESP32-Web-App.md](docs/ESP32-Web-App.md), with a
sister doc [docs/ESP32-Web-App-Mic-Riddle.md](docs/ESP32-Web-App-Mic-Riddle.md)
for the parked mic + riddle path. This README is the friendly starting point -
what it is, how to build and flash it, how to drive it and how to wire it
without chasing glitches.

## IDE

Use whatever you're comfortable with. The [Arduino IDE](https://www.arduino.cc/en/software)
is great to start with because it ships with good examples, but for real dev
work I find [VS Code](https://code.visualstudio.com/) with the [PlatformIO extension](https://platformio.org/) much nicer - that's what the
project is set up for. [FastLED](https://fastled.io/) is the C++ library doing
all the heavy lifting on the LEDs; PlatformIO pulls it in for you automatically.

## Getting started

You don't strictly need a GitHub account, but I'd recommend it - it's free and
lets you watch the project so you get an email whenever something changes (click
**Watch → All Activity**). Clone the repo, make a branch, and work away; we can
sort out any mess with reverts if needed.

No GitHub? You can download a zip and open it in VS Code, then re-grab a fresh
zip when changes come in. Either way works.

## What you're working with

A quick tour of the hardware so the numbers in the code make sense:

| | |
|---|---|
| Shape | Dodecahedron - **12 pentagonal faces** |
| LED runs | **30** "runs" between the vertices (5 per segment × 6 segments) |
| LEDs per run | **71** |
| **Total LEDs** | **2,130** |
| LED type | WS2815 (12 V strips), driven with the WS2811 800 kHz protocol |
| Brain | **Teensy 4.1** (not an Arduino - see the simulation note below) |
| LED driver | OctoWS2811 adapter, running **6 parallel outputs** |
| Pattern select | **3 toggle switches** on the operator panel → 8 modes |

Because there are so many LEDs, we can't drive them all from one pin - there
isn't enough juice or timing headroom. So we split them into **6 segments and
drive them in parallel** (OctoWS2811 does this with DMA, which is what keeps the
animation smooth). Each segment is 5 runs × 71 LEDs = **355 LEDs**.

The one array you actually write to is the big flat `CRGB leds[NUM_LEDS]` in the
code - that's the source of truth. All the constants (`NUM_LEDS`,
`NUM_LEDS_PER_SEGMENT`, etc.) live in [src/led.h](src/led.h), and there are handy
groupings (`pentagons[]`, `allVerticals[]`, `allStrips[]`) that are just views
into `leds[]` for when you want to think in faces or strips instead of one long
line.

## Steps
1. Create a [new project in Wokwi](https://wokwi.com/projects/new) using Arduino Mega.
2. Copy all the code from [src/main_parallel.cpp](https://github.com/ConorGarry/ArduinoPlayground/blob/main/src/main_parallel.cpp) and paste it into `sketch.ino` (ino is arduino bespoke format, but it's C++ under the hood).
3. Copy all the code from [diagram_parallel.json](https://github.com/ConorGarry/ArduinoPlayground/blob/main/diagram_parallel.json) into the `diagram.json` tab in Wokwi.
4. Press the play icon in the simulator. You should see something like this:

![Kapture 2024-04-26 at 09 15 08](https://github.com/ConorGarry/ArduinoPlayground/assets/6222596/f462a347-72d3-42c8-bbd7-b6267ed3380b)

_Note: Due to the scale of the project, you'll need to zoom in and out a lot to see a close-up for the schema and to flick the switches._

At this point, you'll see it's running the default mode (0) which is the `rainbowChase()` function.


## Where everything lives

```
ArduinoPlayground/
├── platformio.ini          # the Teensy project (FastLED + OctoWS2811)
├── src/
│   ├── main.cpp            # the heart of it: setup(), loop(), selectMode(), runPattern()
│   ├── led.h / led.cpp     # geometry constants + the face/strip/band groupings
│   ├── teensy4controller.h # glue between FastLED and OctoWS2811 (leave this alone)
│   ├── webcontrol.{h,cpp}  # talks to the ESP32 over Serial4 (web app commands + status)
│   ├── beatdetect.{h,cpp}  # mic beat-detector - switched off until the mic is wired
│   ├── app/                # patterns only the web app can trigger (fire ritual, question reactions, calibrate)
│   └── conor|dee|denise|spencer/   # everyone's own pattern folder (switch modes 0-7)
├── esp32/                  # a separate little project for the ESP32 web-app sidecar
│   └── data/               # the actual web app: index.html, app.js, style.css, questions.json
├── tools/                  # bench helpers: flash_both.sh, monitor.py, test_bridge.py, check_bridge.py, dev_server.py
└── docs/
    ├── ESP32-Web-App.md            # the live web app / bridge spec
    └── ESP32-Web-App-Mic-Riddle.md # parked mic + riddle path (design + revival recipe)
```

The Teensy firmware runs perfectly well on its own - you only need the `esp32/`
project if you're playing with the web app.

## Building & flashing

Everything runs through PlatformIO (`pio` on the command line, or the buttons in
the VS Code extension).

**The Teensy** (from the repo root):

```bash
pio run                      # build
pio run -t upload            # build + flash (press the little button on the Teensy if it asks)
pio device monitor -b 9600   # serial debug - only prints when DEBUG_MODE = true in main.cpp
```

**The ESP32 web app** (from `esp32/`):

```bash
pio run                                                  # build the firmware
pio run -t uploadfs --upload-port /dev/cu.usbserial-XXXX # send the web app (data/) to the chip
pio run -t upload   --upload-port /dev/cu.usbserial-XXXX # flash the firmware
pio device monitor -b 115200
```

A couple of things that'll save you a head scratch:

- **Run `uploadfs` before `upload` whenever you change anything in
  `esp32/data/`.** The web app and `questions.json` live on a separate part of
  the flash, so a normal firmware upload leaves them untouched. The classic trap
  is editing a question, flashing, and wondering why it didn't change.
- **The ESP32 board needs a USB-A → USB-C cable to flash if running a M chip**,
  not C-to-C - ours is missing the resistors USB-C needs, so a C-to-C cable won't 
  even show up as a port. Keep a USB-A cable in the kit.

Or just do both boards in one shot:

```bash
bash tools/flash_both.sh     # Teensy firmware, then ESP32 web app, then ESP32 firmware
```

## Driving the lights

### The switches (be trippin')

There are **3 toggle switches** on the controller (Teensy pins 0, 1, 23). Read
together they make a little 3-bit number, which picks the pattern. An **open**
switch counts as `1`, a **grounded** one as `0`, and the first switch is the
smallest digit - so all three open is mode 7.

| Switches | Mode | Pattern | Author |
|---|---|---|---|
| `000` (all grounded) | 0 | `rainbowChase` | conor |
| `001` | 1 | `fireFlies` | conor |
| `010` | 2 | `twinkle` + `comets` | denise |
| `011` | 3 | `waveVerticalsOverwards` | denise |
| `100` | 4 | `freePalestineFullBlink` | dee |
| `101` | 5 | `rainbowFade` | conor |
| `110` | 6 | `showLights` | dee |
| `111` (all open) | 7 | `galaxy` (the default) | spencer |

There are a few more patterns (8-15: the fire-ritual stages, the question
reactions and a bench-only `calibrate` strip-direction tool), but those can
**only** be reached from the web app, never the switches. See the index map
in "Adding or updating patterns" below.

### The web app

Plug the ESP32 in and anyone nearby can join the open WiFi network
(`Enter Infinity`); a captive portal pops a phone friendly page where
they tap "take over the lights", answer a few goofy questions and watch the
sculpture respond. While a phone is driving, its choices win - but **the
operator takes back control instantly** the moment any physical switch is
flicked. The switches are always boss.

That takeover is gated by `WEB_OVERRIDE_ENABLED` in [src/main.cpp](src/main.cpp) (it's **on**). The 
status link to the ESP32 always runs in the background; that flag only decides whether the  
web app's pattern picks are obeyed. The whole experience - questions, takeover,
fire-ritual finale, the captive-portal magic - is written up in [docs/ESP32-Web-App.md](docs/ESP32-Web-App.md). 
The parked mic and riddle path lives in its own doc: [docs/ESP32-Web-App-Mic-Riddle.md](docs/ESP32-Web-App-Mic-Riddle.md).

## Wiring it up

### Which pin drives what

The OctoWS2811 adapter decides which Teensy pins carry LED data - we don't get
to choose. The 6 we use, in order, are in `pinList` in [src/main.cpp](src/main.cpp):

| Segment | Teensy pin | Runs it covers |
|---|---|---|
| 1 | 2  | 1–5   |
| 2 | 14 | 6–10  |
| 3 | 7  | 11–15 |
| 4 | 8  | 16–20 |
| 5 | 6  | 21–25 |
| 6 | 20 | 26–30 |

### Grounding - please read this, it's the big one

If you ever see **random pixels flickering the wrong colour** scattered around
the sculpture, that is almost never a bug in the code - it's the LED data line
losing its clean ground reference. WS2815 strips are fussy about this:

- **Watch out for ground loops - this is the usual cause.** The sneakiest one
  shows up on the bench: powering the Teensy and the ESP32 from **two separate
  USB cables while they also share a ground through the bridge wire**. That
  makes a loop and drags the LED data reference around, and you get the scatter.
  The fix is to power both boards from the **one shared 5 V rail** with a single
  common ground. While debugging, you can unplug the bridge ground and watch 
  the glitches disappear to confirm it.
- **Keep one clean common ground.** Tie the LED 12 V ground, the controller
  ground, and the bridge ground together at a single solid point - don't let
  ground sneak back through a second path.
- **The usual WS281x good manners help too:** a ~330 Ω resistor in series on the
  data line at the controller, a short data lead to the first LED with a ground
  wire running right alongside it, and a fat (~1000 µF) capacitor across the
  supply where you inject power.
- **Keep signal wires apart:** route the ESP32 bridge's RX jumper away from the
  six fast LED data lines, and keep it short.

If it still scatters after all that, the problem is on the data line / first LED
/ level-shifter side - not the firmware.

### Power, the bridge, and the mic

LED power (12 V) and logic power (5 V) are separate supplies that meet at
one common ground point. The full power story all live in [docs/ESP32-Web-App.md §2](docs/ESP32-Web-App.md).
The MAX4466 mic isn't wired yet, so beat detection is off; when it goes in
it follows [docs/ESP32-Web-App-Mic-Riddle.md §3](docs/ESP32-Web-App-Mic-Riddle.md).

## Adding or updating patterns

Patterns come in two flavours: ones the **operator switches** can pick
(indices 0-7), and ones only the **web app** can fire (indices 8 and up:
the fire-ritual stages, the question reactions and the bench-only
`calibrate` tool). The one place that maps a numeric index to a function is
`runPattern()` in [src/main.cpp](src/main.cpp), shared by both paths, so an
index always means the same thing on both sides.

Everyone gets their own folder so we don't step on each other:
`src/<your-name>/<your-name>_patterns.{h,cpp}`. A pattern is just a function
that fills the global `CRGB leds[NUM_LEDS]` array and calls `FastLED.show()`.

### What's already mapped

This is the current index → function table. Don't pick a number that's
taken unless you're explicitly replacing something.

| Index | Where it lives | Function | Notes |
|---|---|---|---|
| 0 | conor | `rainbowChase` | switch `000` |
| 1 | conor | `fireFlies` | switch `001` |
| 2 | denise | `twinkle` + `comets` | switch `010` |
| 3 | denise | `waveVerticalsOverwards` | switch `011` |
| 4 | dee | `freePalestineFullBlink` | switch `100` |
| 5 | conor | `rainbowFade` | switch `101` |
| 6 | dee | `showLights` | switch `110` |
| 7 | spencer | `galaxy` | switch `111` (default) |
| 8 | app | `dimAmbient` | fire-ritual transition |
| 9 | app | `fireFull` | fire-ritual fire |
| 10 | app | `idleAmbient` | fire-ritual post-finale |
| 11 | app | `wormYes` | Q1 yes reaction |
| 12 | app | `wormNo` | Q1 no reaction |
| 13 | app | `scrunch` | Q2 no reaction |
| 14 | app | `fold` | Q2 yes reaction |
| 15 | app | `calibrate` | bench-only strip-direction read-off |

### Adding a switch-selectable pattern (indices 0-7)

Heads up: **all 8 switch slots are taken** today. Adding a new
switch-selectable pattern means replacing one above. Talk to the author
first (the table tells you who).

Say you want a `rgbChase()` of your own and you've agreed to take over
index 5:

1. Write `void rgbChase()` in your folder and include your header in
   [src/main.cpp](src/main.cpp).
2. Replace the existing `case 5` line in `runPattern()`:

   ```cpp
   void runPattern(int idx) {
     switch (idx) {
       // ... existing cases ...
       case 5: rgbChase(); break;   // was rainbowFade()
       default: galaxy(); break;
     }
   }
   ```

3. Flash the Teensy (`pio run -t upload`) and flick switches `101`.

### Adding a web-app-only pattern (indices 8 and up)

Same idea, but the pattern lives in [src/app/](src/app/) and gets a named
ID in [src/app/app_patterns.h](src/app/app_patterns.h) so the Teensy
firmware and the web app's `questions.json` agree on which number means
which pattern.

1. Write your function in
   [src/app/app_patterns.cpp](src/app/app_patterns.cpp) (e.g.
   `void slimeWave()`).
2. Give it a named ID and forward-declare it in
   [src/app/app_patterns.h](src/app/app_patterns.h). Pick the next free
   number (16, since `CALIBRATE` is 15):

   ```cpp
   namespace app_patterns {
     constexpr int SLIME_WAVE = 16;   // next free index after CALIBRATE
   }
   void slimeWave();
   ```

3. Add a `case` to `runPattern()` in [src/main.cpp](src/main.cpp):

   ```cpp
   case SLIME_WAVE: slimeWave(); break;
   ```

4. Bump `MAX_PATTERN_INDEX` in
   [src/webcontrol.cpp](src/webcontrol.cpp) if your new index is higher
   than the current cap (`app_patterns::CALIBRATE`). The bridge rejects
   anything above this constant before dispatch, so skipping this step
   makes your pattern silently unreachable from the web app.
5. Point a question (or fire-ritual stage) at it from
   [esp32/data/questions.json](esp32/data/questions.json), see the next
   section.
6. Flash **both** sides: `bash tools/flash_both.sh`, or by hand:
   `pio run -t upload` from the repo root for the Teensy, then
   `pio run -t uploadfs` from `esp32/` to push the updated
   `questions.json`.

### Editing the questions (the data-only case)

This is the most common change: you're not touching firmware, just rewording a
question or swapping which pattern its yes/no fires. Edit
[esp32/data/questions.json](esp32/data/questions.json), then from `esp32/`:

```bash
pio run -t uploadfs --upload-port /dev/cu.usbserial-XXXX
```

`uploadfs` rewrites just the data partition where the web app lives - the
firmware stays untouched. **Don't skip this step**: a regular `pio run -t
upload` does not push `data/` changes, so editing a question and only doing a
firmware flash leaves the old text on the chip. Classic trap.

Anatomy of a question:

```json
{
  "id": 1,
  "type": "binary",
  "q": "Would you still love me if I was a worm?",
  "yes": { "msg": "Love is all the colours! Slurp the rainbow.", "pattern": 11 },
  "no":  { "msg": "You broke my heart. Bye.",                    "pattern": 12 }
}
```

- `pattern` is the same index used by `runPattern()` - e.g. `11` fires
  `wormYes()`. Add your new web-app pattern's ID here to wire it up.
- Optional `yes.label` / `no.label` override the default "Yes" / "No" button
  text (used for the "fold vs scrunch" question).
- To **temporarily disable** a question without deleting it, move the entry
  from `questions[]` into `_disabled_questions[]`. The firmware only reads
  `questions[]`; the disabled array is just a parking lot.
- Bump the top-level `version` number when you change content - it's a useful
  signal during debugging that the on-device copy is current.

### A couple of general tips

- Use `leds[]` for whole-sculpture patterns, and the named groupings
  (`pentagons[i]`, `allVerticals[]`, `allStrips[]`) when you want to work in
  faces or strips.
- Heads up: `leds1..leds6` are **legacy** per-segment buffers that don't drive
  the output any more - reach for `allStrips[]` if you need per-strip access.
- Keep your `case` blocks in numeric order; future-you will thank present-you.

## Simulating it (Wokwi)

There's a [Wokwi](https://docs.wokwi.com/vscode/getting-started) wiring file
(`diagram.json`) for quick visual testing without the real rig. Build first,
then run **"Wokwi: Start Simulator"** from the VS Code command palette. One
caveat: Wokwi simulates an **Arduino Mega**, not a Teensy, so timing and
behaviour can differ - treat it as a rough preview, not gospel. (It's also
resource-heavy, so close other tabs to give it room.)

## Useful reference images

#### Run numbers
![infinity_run_numbers](https://github.com/ConorGarry/ArduinoPlayground/assets/6222596/84512191-e1bc-4627-b7ac-c1138d38c3a0)

#### Face numbers
<img width="684" alt="infinity_face_numbers" src="https://github.com/ConorGarry/ArduinoPlayground/assets/6222596/916eca12-055a-48eb-bf59-4939c64422fd">

#### Wiring diagram
![Infinity_diagram](https://github.com/ConorGarry/ArduinoPlayground/assets/6222596/5d0f5aef-1128-4305-bdec-b8db443cf0af)

#### Parallel segments
![infinity-parallel-segments](https://github.com/ConorGarry/ArduinoPlayground/assets/6222596/29cab9f7-5643-40b3-b56c-9d57e36bf4d8)

_The colours here are just a visual aid for which parallel output is which - map
them to the real pin order in the table above, not the old DP_1..6 pins the
legacy diagram shows._

|Data Pin|Field Name|Colour|
|-|-|-|
|`7`|`DP_1`|`Red`|
|`8`|`DP_2`|`Green`|
|`9`|`DP_3`|`Blue`|
|`10`|`DP_4`|`Yellow`|
|`11`|`DP_5`|`Purple`|
|`12`|`DP_5`|`White`|


## Resources

- [FastLED documentation](http://fastled.io/docs/)
- [FastLED basics YouTube series](https://www.youtube.com/watch?v=4Ut4UK7612M&list=PLgXkGn3BBAGi5dTOCuEwrLuFtfz0kGFTC) - I **highly** recommend this one; he explains everything really well.
- [OctoWS2811 (the Teensy parallel LED driver)](https://www.pjrc.com/teensy/td_libs_OctoWS2811.html)
- [Wokwi VS Code extension](https://docs.wokwi.com/vscode/getting-started)
- The live web-app / bridge spec: [docs/ESP32-Web-App.md](docs/ESP32-Web-App.md)
- The parked mic + riddle path: [docs/ESP32-Web-App-Mic-Riddle.md](docs/ESP32-Web-App-Mic-Riddle.md)
- Tools and bench scripts: [tools/README.md](tools/README.md)
