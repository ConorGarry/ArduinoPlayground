#!/usr/bin/env python3
"""Pre-compress the captive-portal static assets so the ESP serves them gzipped.

serveStatic() in web_routes.h prefers a `<file>.gz` sibling and sends it with
Content-Encoding: gzip, cutting the initial connect-storm transfer ~3-4x — the
heaviest moment of a festival night, through a synchronous web server.

Runs as an ESP32 pre-build hook (see esp32/platformio.ini), before the LittleFS
image is packed. Run by hand from the project root:  python3 tools/gzip_assets.py
"""

import gzip
import sys
from pathlib import Path


def find_repo():
    """Repo root, from any CWD. Handles PlatformIO exec() (no __file__)."""
    try:
        start = Path(__file__).resolve().parent
    except NameError:
        start = Path.cwd()
    for base in (start, Path.cwd()):
        for d in (base, *base.parents):
            if (d / "esp32" / "data").is_dir():
                return d
    print("[gzip_assets] ERROR: could not locate repo root (esp32/data not found)",
          file=sys.stderr)
    sys.exit(1)


REPO = find_repo()
DATA = REPO / "esp32" / "data"

# Client-served text assets. (questions.json / patterns.json are read server-side,
# not fetched by the browser, so they don't need compressing.)
ASSETS = ["index.html", "app.js", "style.css", "geometry.js", "particles.js"]


def run():
    count = 0
    for name in ASSETS:
        src = DATA / name
        if not src.exists():
            print(f"[gzip_assets] skip {name} (missing)")
            continue
        raw = src.read_bytes()
        # mtime=0 → deterministic output, so unchanged sources don't churn the
        # .gz (no needless reflash / git noise).
        packed = gzip.compress(raw, compresslevel=9, mtime=0)
        (DATA / (name + ".gz")).write_bytes(packed)
        count += 1
        pct = 100 - round(len(packed) / len(raw) * 100) if raw else 0
        print(f"[gzip_assets] {name}: {len(raw)} -> {len(packed)} bytes (-{pct}%)")
    print(f"[gzip_assets] wrote {count} .gz asset(s)")


run()
