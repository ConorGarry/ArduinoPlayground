#!/usr/bin/env bash
# Flash both boards: Teensy firmware, then ESP32 LittleFS image, then ESP32 firmware.
#
# Both USB cables can be plugged in at the same time on the bench — the boards
# don't yet share an external 5V rail. (If they do, unplug one USB.)
#
# When the Teensy Loader window appears, press the small white button on the
# Teensy 4.1 to trigger the flash.
#
# Overrides via env:
#   ESP32_PORT=/dev/cu.usbserial-XYZ   bash tools/flash_both.sh

set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
PIO="$HOME/.platformio/penv/bin/pio"
TEENSY_LOADER_APP="$HOME/.platformio/packages/tool-teensy/teensy.app"

# Auto-detect ESP32 port unless overridden.
if [[ -z "${ESP32_PORT:-}" ]]; then
  ESP32_PORT="$(ls /dev/cu.usbserial-* 2>/dev/null | head -1 || true)"
fi
if [[ -z "${ESP32_PORT:-}" ]]; then
  echo "ERROR: no /dev/cu.usbserial-* port — is the ESP32 plugged in via USB-A→C?" >&2
  exit 1
fi

echo "==> Teensy build + flash"
# Teensy Loader on macOS sometimes can't find the firmware file on cold
# launch; pre-opening it avoids the "could not find file firmware" error.
[[ -d "$TEENSY_LOADER_APP" ]] && open -g "$TEENSY_LOADER_APP" && sleep 1
(cd "$ROOT" && "$PIO" run -t upload)
echo "(if Teensy Loader is waiting, press the white program button on the Teensy)"
echo

echo "==> ESP32 LittleFS image (data/ assets)"
(cd "$ROOT/esp32" && "$PIO" run -t uploadfs --upload-port "$ESP32_PORT")
echo

echo "==> ESP32 firmware"
(cd "$ROOT/esp32" && "$PIO" run -t upload --upload-port "$ESP32_PORT")
echo

echo "==> Done. Verify with:"
echo "    ~/.platformio/penv/bin/python3 tools/monitor.py"
