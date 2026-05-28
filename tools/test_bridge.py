#!/usr/bin/env python3
"""Verify the Teensy ↔ ESP32 Serial4 bridge round-trip.

How it works:
  1. Opens ESP32 USB serial passively (no reset).
  2. Captures 20s of bridge traffic.
  3. While capturing, you tap any answer on a phone joined to the AP.
  4. Asserts:
       - At least one `[bridge → teensy] P:N`     (ESP32 sent a command)
       - `[bridge ← teensy] MODE:web`             (Teensy received it)
       - A `[bridge ← teensy] PATTERN:N` matching (Teensy switched pattern)
  5. Reports PASS / FAIL with round-trip time.

Requires DEBUG_BRIDGE = 1 in esp32/src/config.h so the ESP32 actually logs
bridge frames to USB serial. If you see no `[bridge ...]` lines at all,
that's the first thing to flip.

Usage:
    ~/.platformio/penv/bin/python3 tools/test_bridge.py
"""

import re
import sys
import time

import serial
import serial_helper

CAPTURE_S = 20

RE_SENT_P    = re.compile(r"\[bridge → teensy\] P:(\d+)")
RE_MODE_WEB  = re.compile(r"\[bridge ← teensy\] MODE:web")
RE_PATTERN_N = re.compile(r"\[bridge ← teensy\] PATTERN:(\d+)")


def main() -> int:
    port = serial_helper.find_esp32_port()
    try:
        s = serial_helper.open_passive(port)
    except serial.SerialException as e:
        print(f"ERROR: cannot open {port}: {e}", file=sys.stderr)
        print("Hint: close other monitors. Confirm DEBUG_BRIDGE=1 in config.h.", file=sys.stderr)
        return 1

    print(f"⏱  Capturing {CAPTURE_S}s of bridge traffic from {port}.")
    print("   TAP A PHONE ANSWER NOW.  Riddle or Yes/No either works.\n")

    captured = []
    start = time.time()
    deadline = start + CAPTURE_S
    while time.time() < deadline:
        raw = s.readline()
        if not raw:
            continue
        t_ms = int((time.time() - start) * 1000)
        line = raw.decode("utf-8", errors="replace").rstrip()
        captured.append((t_ms, line))
        print(f"{t_ms:>5}ms  {line}")
    s.close()

    print("\n--- analysis ---")

    sent = [(t, int(m.group(1))) for t, l in captured for m in [RE_SENT_P.search(l)] if m]
    saw_web = next(((t, l) for t, l in captured if RE_MODE_WEB.search(l)), None)
    patterns = [(t, int(m.group(1))) for t, l in captured for m in [RE_PATTERN_N.search(l)] if m]

    if not sent:
        print("❌ FAIL: no `[bridge → teensy] P:N` seen.")
        print("   Either you didn't tap an answer during the window, OR")
        print("   DEBUG_BRIDGE is off in esp32/src/config.h (flip it and reflash), OR")
        print("   the phone isn't joined to the AP.")
        return 2

    sent_t, sent_n = sent[0]
    print(f"✓ ESP32 sent {len(sent)} command(s): {sent}")

    if not saw_web:
        print("❌ FAIL: Teensy never reported MODE:web.")
        print("   The ESP32 is talking but the Teensy isn't hearing it. Check:")
        print("   - OctoWS2811 Pin 18 trace cut + Pin 16 jumper (see spec §2.1)")
        print("   - Bridge wire ESP32 GPIO17 (TX2) → Teensy Pin 16 (RX4)")
        print("   - Common GND between both boards")
        return 3

    web_t, _ = saw_web
    print(f"✓ Teensy reported MODE:web within {web_t - sent_t}ms of first command")

    matched_pattern = next(
        ((t, n) for t, n in patterns if t >= sent_t and any(n == sn for _, sn in sent)),
        None,
    )
    if not matched_pattern:
        print("❌ FAIL: Teensy reported MODE:web but never broadcast a PATTERN:N")
        print("   matching the command it was sent. The bridge is delivering")
        print("   commands, but the lights aren't actually following them. Check:")
        print("   - WEB_OVERRIDE_ENABLED is true in src/main.cpp")
        print("   - the commanded index is in range (0..14)")
        print(f"   (sent {sent}, patterns seen {patterns})")
        return 4

    mt, mn = matched_pattern
    print(f"✓ Teensy switched to PATTERN:{mn} within {mt - sent_t}ms")

    print("\n🎉 PASS — bidirectional bridge confirmed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
