# tools/

Bench scripts for the Infinity ESP32 + Teensy controller. Not part of
the firmware itself, these are for diagnosis, manual testing and reducing
the friction of common flash / monitor / verify cycles.

All scripts use PlatformIO's bundled Python (`~/.platformio/penv/bin/python3`)
which already has `pyserial` and `esptool` installed.

## Files

| File | Purpose |
|---|---|
| `serial_helper.py` | Shared module: port discovery, passive serial open (won't reset the chip), `hard_reset_esp32()` for recovery |
| `monitor.py` | Timestamped serial monitor, alternative to `pio device monitor` |
| `test_bridge.py` | 20s capture: PASS/FAIL on the Teensy ↔ ESP32 Serial4 bridge round-trip (requires a phone tap during the window) |
| `check_bridge.py` | 15s passive sniff: counts MODE / PATTERN / PING frames and flags corruption. No phone tap needed. Use as a pre-flight before powering LEDs |
| `dev_server.py` | Local browser mock of every `/api/*` endpoint. Demo the question flow, takeover and fire-ritual finale on a Mac without flashing hardware |
| `flash_both.sh` | One command, both boards flashed (Teensy → ESP32 LittleFS → ESP32 firmware) |
| `gen_patterns.py` | Generates `esp32/data/patterns.json` from the Teensy's `app_patterns.h` and fails if any pattern ID / `P:` command name has drifted out of sync across the two firmwares. Runs automatically as an ESP32 pre-build hook; run by hand with `python3 tools/gen_patterns.py` |

## Common usage

Run from the project root (`ArduinoPlayground/`):

```bash
# Flash both boards (press Teensy program button when prompted)
bash tools/flash_both.sh

# Watch ESP32 USB serial (with timestamps, won't reset the chip)
~/.platformio/penv/bin/python3 tools/monitor.py

# Same but for the Teensy
~/.platformio/penv/bin/python3 tools/monitor.py --teensy

# Verify the bridge round-trip (tap a phone answer during the 20s window)
~/.platformio/penv/bin/python3 tools/test_bridge.py
```

## Gotchas the scripts side-step for you

- **pyserial DTR pulse on open** → would crash the ESP32 into download mode.
  `serial_helper.open_passive()` disables it. If a chip is already stuck,
  `serial_helper.hard_reset_esp32()` recovers it.
- **Port-locked errors** → another process (`pio device monitor`, a previous
  script) is holding the port. Scripts use `exclusive=True` and report
  cleanly. Close the other reader.
- **Teensy Loader cold-launch bug** → `flash_both.sh` pre-launches the app
  so the "could not find file firmware" error doesn't bite.

The full gotcha catalogue is in `../docs/ESP32-Web-App.md` §8.4.
