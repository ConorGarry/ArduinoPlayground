#!/usr/bin/env python3
"""Single source of truth enforcer for LED pattern IDs across both firmwares.

The Teensy header ``src/app/app_patterns.h`` owns the pattern indices
(``constexpr int NAME = N;``). Those numbers — and the ``P:<name>`` wire
commands derived from them — are referenced by hand in the ESP32 web data and
in the Teensy bridge parser. Forget to update one file and the ESP can ask for
a pattern the Teensy doesn't know, which silently falls through to galaxy().

This script reads the header (the canonical source) and:

  1. GENERATES ``esp32/data/patterns.json`` — a NAME -> index map so anything
     on the ESP side can refer to patterns by name instead of magic numbers.
  2. VALIDATES ``esp32/data/questions.json`` — every ``"pattern": N`` must be a
     known index (operator bank 0-7 plus the header constants).
  3. VALIDATES the fire-ritual command names agree across
     ``esp32/src/web_routes.h`` (allowlist), ``src/webcontrol.cpp`` (dispatch)
     and ``esp32/data/app.js`` (browser sendCommand calls).

Any drift prints the mismatch and exits non-zero, which aborts the build.

Runs automatically as a PlatformIO pre-build hook (see esp32/platformio.ini).
Run by hand from the project root:  python3 tools/gen_patterns.py
"""

import json
import re
import sys
from pathlib import Path

def find_repo():
    """Locate the project root from any CWD.

    Works standalone (python3 tools/gen_patterns.py) and as a PlatformIO
    extra_script, where the script is exec()'d without a `__file__` global and
    the CWD is the esp32/ project dir.
    """
    try:
        start = Path(__file__).resolve().parent
    except NameError:                       # PlatformIO exec() — no __file__
        start = Path.cwd()
    for base in (start, Path.cwd()):
        for d in (base, *base.parents):
            if (d / "src" / "app" / "app_patterns.h").exists():
                return d
    print("[gen_patterns] ERROR: could not locate repo root "
          "(src/app/app_patterns.h not found)", file=sys.stderr)
    sys.exit(1)


REPO = find_repo()                          # project root
ESP = REPO / "esp32"

HEADER = REPO / "src" / "app" / "app_patterns.h"
WEBCONTROL = REPO / "src" / "webcontrol.cpp"
QUESTIONS = ESP / "data" / "questions.json"
APP_JS = ESP / "data" / "app.js"
WEB_ROUTES = ESP / "src" / "web_routes.h"
PATTERNS_OUT = ESP / "data" / "patterns.json"

# Operator/button bank (raw `case 0:`..`case 7:` in src/main.cpp). These live
# inline in the switch rather than as named constants, so they're mirrored here.
# Stable hardware switches — rarely change. Keep in step with main.cpp if they do.
OPERATOR_BANK = {
    0: "rainbowChase",
    1: "fireFlies",
    2: "twinkle+comets",
    3: "waveVerticalsOverwards",
    4: "freePalestineFullBlink",
    5: "rainbowFade",
    6: "showLights",
    7: "galaxy",
}

# Header constants whose lowercased name is also a `P:<name>` wire command
# (the fire-ritual finale + the bench calibration tool).
FIRE_COMMAND_CONSTANTS = ["DIM_AMBIENT", "FIRE_FULL", "IDLE_AMBIENT", "CALIBRATE"]


def fail(msg):
    print(f"[gen_patterns] ERROR: {msg}", file=sys.stderr)
    sys.exit(1)


def parse_header():
    """{NAME: index} from `constexpr int NAME = N;` lines."""
    consts = {}
    for m in re.finditer(r"constexpr\s+int\s+([A-Z0-9_]+)\s*=\s*(\d+)\s*;", HEADER.read_text()):
        consts[m.group(1)] = int(m.group(2))
    if not consts:
        fail(f"no `constexpr int NAME = N;` constants found in {HEADER}")
    return consts


def expected_commands(header):
    """{'P:fire_full': 9, ...} for the wire-command subset of the header."""
    cmds = {}
    for name in FIRE_COMMAND_CONSTANTS:
        if name not in header:
            fail(f"fire-command constant {name} missing from {HEADER.name}")
        cmds["P:" + name.lower()] = header[name]
    return cmds


def collect_question_patterns():
    """Every (json-path, value) for a "pattern" key, recursively."""
    found = []

    def walk(node, path):
        if isinstance(node, dict):
            for k, v in node.items():
                if k == "pattern" and isinstance(v, int):
                    found.append((path + ".pattern", v))
                else:
                    walk(v, f"{path}.{k}")
        elif isinstance(node, list):
            for i, v in enumerate(node):
                walk(v, f"{path}[{i}]")

    walk(json.loads(QUESTIONS.read_text()), "questions.json")
    return found


def validate_questions(header):
    valid = set(header.values()) | set(OPERATOR_BANK)
    ok = True
    for where, idx in collect_question_patterns():
        if idx not in valid:
            print(f"[gen_patterns] questions.json: pattern {idx} at {where} "
                  f"is not a known index", file=sys.stderr)
            ok = False
    return ok


def validate_commands(header):
    expected = set(expected_commands(header))
    ok = True

    # web_routes.h: FIRE_RITUAL_COMMANDS[] string allowlist — must match exactly.
    block = re.search(r"FIRE_RITUAL_COMMANDS\[\]\s*=\s*\{(.*?)\}",
                      WEB_ROUTES.read_text(), re.S)
    if not block:
        fail("FIRE_RITUAL_COMMANDS[] allowlist not found in web_routes.h")
    allowlist = set(re.findall(r'"(P:[^"]+)"', block.group(1)))
    if allowlist != expected:
        print(f"[gen_patterns] web_routes.h allowlist {sorted(allowlist)} "
              f"!= expected {sorted(expected)}", file=sys.stderr)
        ok = False

    # webcontrol.cpp: strcmp(line, "P:...") dispatch — must match exactly.
    dispatch = set(re.findall(r'strcmp\(line,\s*"(P:[a-z_]+)"\)', WEBCONTROL.read_text()))
    if dispatch != expected:
        print(f"[gen_patterns] webcontrol.cpp dispatch {sorted(dispatch)} "
              f"!= expected {sorted(expected)}", file=sys.stderr)
        ok = False

    # app.js: sendCommand('P:...') — a subset (calibrate is bench-only), but it
    # must never send a command outside the canonical set.
    sent = set(re.findall(r"""sendCommand\(\s*['"](P:[a-z_]+)['"]""", APP_JS.read_text()))
    unknown = sent - expected
    if unknown:
        print(f"[gen_patterns] app.js sends unknown command(s) {sorted(unknown)}",
              file=sys.stderr)
        ok = False
    return ok


def generate(header):
    payload = {
        "_generated": "by tools/gen_patterns.py from src/app/app_patterns.h — DO NOT EDIT",
        "operator_bank": {str(k): v for k, v in OPERATOR_BANK.items()},
        "patterns": header,
        "commands": expected_commands(header),
    }
    PATTERNS_OUT.write_text(json.dumps(payload, indent=2) + "\n")
    print(f"[gen_patterns] wrote {PATTERNS_OUT.relative_to(REPO)} "
          f"({len(header)} patterns, {len(OPERATOR_BANK)} operator)")


def run():
    header = parse_header()
    ok = validate_questions(header)
    ok = validate_commands(header) and ok
    if not ok:
        fail("pattern IDs / command names are out of sync — fix the files above")
    generate(header)
    print("[gen_patterns] OK — pattern IDs in sync")


# Executes both standalone (python3 tools/gen_patterns.py) and when PlatformIO
# imports this as a pre-build extra_script. On success it returns normally so
# the build proceeds; on drift fail() exits non-zero and aborts the build.
run()
