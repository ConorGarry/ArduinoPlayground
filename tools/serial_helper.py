"""Shared serial / esptool helpers for the Infinity Flux bench scripts.

All scripts in this directory should `import serial_helper` rather than
re-deriving the gotchas we hit during bring-up.

Gotchas this module exists to side-step:

  - pyserial asserts DTR briefly when `Serial()` opens, which yanks the
    ESP32's EN line and (combined with auto-reset circuitry on cheap CH340
    boards) can drop the chip into download mode. `open_passive()` opens
    with `dsrdtr=False, rtscts=False` to avoid this.
  - If something else already screwed up the chip's state and it boots into
    `waiting for download`, `hard_reset_esp32()` uses esptool's standard
    reset sequence to put it back in run mode.
  - Two processes reading the same serial port produces garbled output.
    `exclusive=True` blocks the second opener cleanly on macOS.
  - Port names on macOS aren't stable across reboots but the patterns are:
    Teensy = /dev/cu.usbmodem*, CH340-based ESP32 = /dev/cu.usbserial-*.
"""

import glob
import os
import subprocess
import time
from typing import Optional

import serial

# PlatformIO's bundled toolchain is what we've validated against. Using
# system Python with a separate pyserial usually works too but bundled is
# guaranteed compatible with the versions PIO actually flashed.
PIO_PYTHON = os.path.expanduser("~/.platformio/penv/bin/python3")
ESPTOOL    = os.path.expanduser("~/.platformio/packages/tool-esptoolpy/esptool.py")

# macOS serial-device patterns by chip family.
TEENSY_GLOB = "/dev/cu.usbmodem*"
ESP32_GLOB  = "/dev/cu.usbserial-*"


# --- Port discovery ----------------------------------------------------------

def find_teensy_port() -> str:
    ports = sorted(glob.glob(TEENSY_GLOB))
    if not ports:
        raise RuntimeError(
            "No Teensy port found (expected /dev/cu.usbmodem*). "
            "Check USB cable + that the Teensy is running firmware (not stuck in HID)."
        )
    return ports[0]


def find_esp32_port() -> str:
    ports = sorted(glob.glob(ESP32_GLOB))
    if not ports:
        raise RuntimeError(
            "No ESP32 port found (expected /dev/cu.usbserial-*). "
            "If using a USB-C-only board, try a USB-A → USB-C cable — "
            "many cheap boards miss the CC pull-down resistors needed for "
            "USB-C → USB-C power negotiation."
        )
    return ports[0]


# --- Serial -----------------------------------------------------------------

def open_passive(port: str, baud: int = 115200, timeout: float = 0.1) -> serial.Serial:
    """Open a serial port without disturbing the device's run state.

    Use this for monitoring an already-running chip. Will NOT pulse DTR/RTS
    on open, so the ESP32 stays in run mode.

    Raises serial.SerialException if the port is locked by another process
    (usually `pio device monitor` in another terminal).
    """
    return serial.Serial(
        port=port, baudrate=baud, timeout=timeout,
        dsrdtr=False, rtscts=False, exclusive=True,
    )


# --- ESP32 reset ------------------------------------------------------------

def hard_reset_esp32(port: Optional[str] = None, quiet: bool = True) -> None:
    """Use esptool to put the ESP32 in run mode.

    Recovery path when the chip is stuck in `waiting for download` because
    our scripts disturbed DTR/RTS. esptool's standard "default_reset" +
    "hard_reset" sequence boots the firmware cleanly.

    Briefly opens the port itself, so any monitor on it must be closed first.
    """
    port = port or find_esp32_port()
    cmd = [PIO_PYTHON, ESPTOOL, "--port", port,
           "--before", "default_reset", "--after", "hard_reset",
           "chip_id"]
    if quiet:
        subprocess.run(cmd, capture_output=True, check=False)
    else:
        subprocess.run(cmd, check=False)
    # Brief settle so the boot banner has time to start streaming before
    # callers open the port again.
    time.sleep(0.5)
