#!/usr/bin/env python3
"""Timestamped serial monitor for the InfinityFlux ESP32 (or Teensy).

Won't reset the chip on open — safe to attach mid-run. Ctrl+C to exit.

Usage:
    ~/.platformio/penv/bin/python3 tools/monitor.py             # auto-detect ESP32
    ~/.platformio/penv/bin/python3 tools/monitor.py --teensy    # auto-detect Teensy
    ~/.platformio/penv/bin/python3 tools/monitor.py /dev/cu.X   # explicit port
"""

import argparse
import sys
import time

import serial
import serial_helper


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("port", nargs="?", help="Serial device (default: auto-detect)")
    ap.add_argument("--teensy", action="store_true", help="Auto-detect Teensy instead of ESP32")
    ap.add_argument("--baud", type=int, default=115200)
    args = ap.parse_args()

    if args.port:
        port = args.port
    elif args.teensy:
        port = serial_helper.find_teensy_port()
    else:
        port = serial_helper.find_esp32_port()

    print(f"Monitoring {port} @ {args.baud}.  Ctrl+C to exit.")
    try:
        s = serial_helper.open_passive(port, baud=args.baud)
    except serial.SerialException as e:
        print(f"\nERROR: cannot open {port}: {e}", file=sys.stderr)
        print("Hint: close any other terminal running `pio device monitor`.", file=sys.stderr)
        return 1

    start = time.time()
    try:
        while True:
            line = s.readline()
            if line:
                ts = int((time.time() - start) * 1000)
                print(f"{ts:>7}ms  {line.decode('utf-8', errors='replace').rstrip()}")
    except KeyboardInterrupt:
        print("\n--- done ---")
    finally:
        s.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
