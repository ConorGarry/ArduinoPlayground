#!/usr/bin/env python3
"""Passive pre-flight check of the Teensy ↔ ESP32 serial bridge.

Unlike test_bridge.py, this needs no phone tap — it sniffs the traffic the
bridge runs on its own (Teensy MODE/PATTERN broadcasts every 500 ms + ESP PING
every 5 s) and validates that the frames are arriving cleanly. Useful as a
grounding sanity check BEFORE powering the LED strips: gross ground faults
between the two boards usually mangle the UART stream first, and serial
corruption is a strong predictor of LED-data-line corruption.

Requirements:
  - DEBUG_BRIDGE = 1 in esp32/src/config.h
  - ESP32 reflashed with that flag on
  - Both boards powered, bridge wires (TX/RX/GND) connected

PASS criteria (within a 15 s capture):
  - ≥ 30 MODE: frames from Teensy        (nominal cadence ~120)
  - ≥ 30 PATTERN: frames from Teensy
  - ≥ 2  PING frames from ESP
  - 0   garbled / unrecognised bridge lines

FAIL = any of those missing, or any corrupted line — which means electrical
noise on the bridge. Lift the bridge GND wire and re-run to confirm grounding
is the culprit.

LIMITATIONS:
  - This is UART-level. WS2815 data lines run at 800 kHz with tight timing, and
    can fail with ground noise too subtle to corrupt a 115200 baud UART. PASS
    here means "the bridge ground is at least usable," NOT "the LED ground is
    clean." Confirm with a brief LEDs-on test.
  - Does not check ESP brownouts, Teensy resets, or anything that doesn't show
    up in the bridge stream.

Usage:
    ~/.platformio/penv/bin/python3 tools/check_bridge.py
"""

import re
import sys
import time

import serial
import serial_helper

CAPTURE_S = 15

# DEBUG_BRIDGE in esp32/src/bridge.h prints these two prefixes.
RE_RX_LINE = re.compile(r"\[bridge ← teensy\] (.*)")  # ←
RE_TX_LINE = re.compile(r"\[bridge → teensy\] (.*)")  # →

RE_MODE    = re.compile(r"^MODE:(operator|web)\s*$")
RE_PATTERN = re.compile(r"^PATTERN:(\d+)\s*$")
RE_PING    = re.compile(r"^PING\s*$")


def main() -> int:
    port = serial_helper.find_esp32_port()
    try:
        s = serial_helper.open_passive(port)
    except serial.SerialException as e:
        print(f"ERROR: cannot open {port}: {e}", file=sys.stderr)
        print("Hint: close any other monitors (pio device monitor, monitor.py).",
              file=sys.stderr)
        return 1

    print(f"Sniffing bridge from {port} for {CAPTURE_S}s.")
    print("LEDs can stay off — this just checks the serial bridge between the boards.")
    print("Expect ~4 lines/sec from the Teensy (MODE + PATTERN every 500 ms)")
    print("and 1 PING every 5 s from the ESP.\n")

    rx_modes     = []   # MODE: lines from Teensy
    rx_patterns  = []   # PATTERN: lines from Teensy
    tx_pings     = []   # PING lines from ESP
    rx_garbled   = []   # bridge ← teensy lines that didn't parse
    bridge_lines = 0    # any line containing "[bridge "

    start = time.time()
    deadline = start + CAPTURE_S
    while time.time() < deadline:
        raw = s.readline()
        if not raw:
            continue
        t_ms = int((time.time() - start) * 1000)
        line = raw.decode("utf-8", errors="replace").rstrip()

        if "[bridge " in line:
            bridge_lines += 1

        m = RE_RX_LINE.search(line)
        if m:
            body = m.group(1).strip()
            if RE_MODE.match(body):
                rx_modes.append((t_ms, body))
            elif RE_PATTERN.match(body):
                rx_patterns.append((t_ms, body))
            else:
                rx_garbled.append((t_ms, body))
            continue

        m = RE_TX_LINE.search(line)
        if m and RE_PING.match(m.group(1).strip()):
            tx_pings.append((t_ms, m.group(1).strip()))

    s.close()

    # ---- report --------------------------------------------------------
    print(f"--- captured in {CAPTURE_S}s ---")
    print(f"Teensy → ESP    MODE:     {len(rx_modes):>3} frames")
    print(f"Teensy → ESP    PATTERN:  {len(rx_patterns):>3} frames")
    print(f"ESP    → Teensy PING:     {len(tx_pings):>3} frames")
    print(f"Garbled bridge lines:     {len(rx_garbled):>3}")

    if rx_garbled:
        print("\n!! garbled lines (first 8):")
        for t, l in rx_garbled[:8]:
            print(f"   {t:>5} ms  {l!r}")

    print("\n--- analysis ---")

    # Heuristic: did the user forget to flip DEBUG_BRIDGE?
    if bridge_lines == 0:
        print("❌ No `[bridge ...]` lines at all.")
        print("   Almost certainly DEBUG_BRIDGE is still 0 in esp32/src/config.h.")
        print("   Flip it to 1, reflash the ESP, and re-run:")
        print("     pio run -d esp32 -t upload")
        return 2

    fail = False

    EXPECTED_MIN_MODES    = 30   # half of nominal ~120
    EXPECTED_MIN_PATTERNS = 30
    EXPECTED_MIN_PINGS    = 2    # 3 nominal + boot ping

    if len(rx_modes) < EXPECTED_MIN_MODES:
        print(f"❌ Teensy → ESP MODE broadcasts too few "
              f"({len(rx_modes)} < {EXPECTED_MIN_MODES}).")
        print("   The Teensy → ESP path is impaired. Check:")
        print("   - Teensy is powered")
        print("   - Teensy Pin 17 (TX4) → ESP GPIO16 (RX2)")
        print("   - common GND between the boards")
        fail = True
    else:
        print(f"✓ Teensy → ESP MODE broadcasts cadence OK ({len(rx_modes)})")

    if len(rx_patterns) < EXPECTED_MIN_PATTERNS:
        print(f"❌ Teensy → ESP PATTERN broadcasts too few "
              f"({len(rx_patterns)} < {EXPECTED_MIN_PATTERNS}).")
        fail = True
    else:
        print(f"✓ Teensy → ESP PATTERN broadcasts cadence OK ({len(rx_patterns)})")

    if len(tx_pings) < EXPECTED_MIN_PINGS:
        print(f"❌ ESP → Teensy PINGs missing ({len(tx_pings)} < {EXPECTED_MIN_PINGS}).")
        print("   The ESP isn't sending. Power? Did you reflash with DEBUG_BRIDGE=1?")
        fail = True
    else:
        print(f"✓ ESP → Teensy PINGs present ({len(tx_pings)})")

    if rx_garbled:
        print(f"❌ {len(rx_garbled)} garbled / unrecognised bridge line(s).")
        print("   Corruption in the bridge UART almost always means electrical noise.")
        print("   Most likely a ground problem between the boards. Quick test:")
        print("   lift the bridge GND wire and re-run; if the garble disappears,")
        print("   you've localised it to grounding.")
        fail = True
    else:
        print("✓ No garbled bridge lines")

    if fail:
        print("\n❌ Bridge integrity FAILED — fix the bridge before powering the LEDs.")
        return 3

    print("\n🎉 Bridge integrity PASSED (UART-level).")
    print("   This rules out gross ground faults between the two boards.")
    print("   It does NOT guarantee the LED data-line / strip ground is clean —")
    print("   confirm with a brief LEDs-on look for scattered wrong-colour pixels.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
