#!/usr/bin/env python3
"""
Browser-side test environment for the ESP32 web app.

Serves esp32/data/index.html + app.js + style.css and mocks every /api/*
endpoint the firmware exposes, so the whole interaction flow (questions,
binary answers, riddle, fire ritual, stats) can be demoed and tested on a
Mac without flashing hardware.

State is in-memory and resets when the process exits. Hit GET /__reset to
clear state without restarting (handy mid-demo).

Usage:
    python3 tools/dev_server.py              # http://localhost:8080
    python3 tools/dev_server.py --port 3000
    python3 tools/dev_server.py --open       # also open in default browser
"""

import argparse
import json
import random
import sys
import threading
import time
import webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse

ROOT = Path(__file__).resolve().parent.parent
DATA_DIR = ROOT / "esp32" / "data"

CONTENT_TYPES = {
    ".html": "text/html",
    ".css":  "text/css",
    ".js":   "application/javascript",
    ".json": "application/json",
    ".svg":  "image/svg+xml",
    ".ico":  "image/x-icon",
}

# Mirrors questions.h::RIDDLE_PREFER_PCT
RIDDLE_PREFER_PCT = 70
# Mirrors config.h::PER_IP_COOLDOWN_MS
PER_IP_COOLDOWN_MS = 2000


class State:
    """In-memory mirror of the firmware's shared state."""

    def __init__(self, questions_path: Path):
        self.questions_path = questions_path
        self.lock = threading.Lock()
        self.reload_questions()
        self.total_yes = 0
        self.total_no = 0
        self.beat_activated = False
        self.beat_activations = 0
        self.recent_ids: list[int] = []  # ring of last 8 served qids
        self.last_ip = ""
        self.last_answer_ms = 0
        # /api/state mock fields — match firmware shape
        self.last_teensy_mode = "idle"
        self.last_teensy_pattern = -1
        self.last_teensy_beat = False
        self.boot_ms = int(time.time() * 1000)
        # Fake bridge log so we can see what the firmware *would* send.
        self.bridge_log: list[str] = []

    def reload_questions(self):
        with self.questions_path.open() as f:
            doc = json.load(f)
        self.questions = doc.get("questions", [])
        self.by_id = {q["id"]: q for q in self.questions}
        log(f"[questions] loaded {len(self.questions)} entries")

    def reset(self):
        with self.lock:
            self.total_yes = 0
            self.total_no = 0
            self.beat_activated = False
            self.beat_activations = 0
            self.recent_ids = []
            self.last_ip = ""
            self.last_answer_ms = 0
            self.last_teensy_pattern = -1
            self.last_teensy_beat = False
            self.bridge_log.clear()
            self.reload_questions()
        log("[state] reset")

    def mark_served(self, qid: int):
        self.recent_ids.append(qid)
        if len(self.recent_ids) > 8:
            self.recent_ids.pop(0)

    def was_recent(self, qid: int) -> bool:
        return qid in self.recent_ids

    def send_bridge_line(self, line: str):
        """Pretend to forward a line to the Teensy — log + update mode state."""
        self.bridge_log.append(line)
        if line.startswith("P:"):
            payload = line[2:]
            try:
                self.last_teensy_pattern = int(payload)
                self.last_teensy_mode = "pattern"
            except ValueError:
                self.last_teensy_mode = payload  # e.g. P:fire_full
        elif line == "BEAT:on":
            self.last_teensy_beat = True
        elif line == "BEAT:off":
            self.last_teensy_beat = False
        log(f"[bridge→teensy] {line}")

    def pick_random(self, exclude: set[int]) -> dict | None:
        """Mirror of questions::pickRandom in questions.h."""
        if not self.questions:
            return None

        def is_excluded(qid: int) -> bool:
            return qid in exclude

        def is_riddle(q) -> bool:
            return q.get("type", "binary") == "riddle"

        # Phase 1 — riddle preference window
        if not self.beat_activated and random.randint(0, 99) < RIDDLE_PREFER_PCT:
            for q in self.questions:
                if is_riddle(q):
                    qid = q.get("id", -1)
                    if not is_excluded(qid):
                        self.mark_served(qid)
                        return q
                    break  # riddle already seen — fall through

        # Phase 2 — normal random pick
        for _ in range(12):
            q = random.choice(self.questions)
            if self.beat_activated and is_riddle(q):
                continue
            qid = q.get("id", -1)
            if is_excluded(qid):
                continue
            if qid < 0 or not self.was_recent(qid):
                self.mark_served(qid)
                return q

        # Phase 3 — deterministic fallback
        for q in self.questions:
            if self.beat_activated and is_riddle(q):
                continue
            qid = q.get("id", -1)
            if is_excluded(qid):
                continue
            self.mark_served(qid)
            return q

        return None  # exhausted


def log(msg: str):
    sys.stdout.write(f"{msg}\n")
    sys.stdout.flush()


def parse_seen(raw: str) -> set[int]:
    seen: set[int] = set()
    for tok in raw.split(","):
        tok = tok.strip()
        if not tok:
            continue
        try:
            seen.add(int(tok))
        except ValueError:
            pass
    return seen


def make_handler(state: State):

    FIRE_RITUAL_COMMANDS = {"P:dim_ambient", "P:fire_full", "P:idle_ambient"}

    class Handler(BaseHTTPRequestHandler):
        # Quieter default request logging — we do our own.
        def log_message(self, fmt, *args):
            log(f"[http] {self.address_string()} {fmt % args}")

        # ---- helpers ----
        def send_json(self, status: int, payload: dict | list):
            body = json.dumps(payload).encode("utf-8")
            self.send_response(status)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0")
            self.end_headers()
            self.wfile.write(body)

        def send_static(self, path: Path) -> bool:
            if not path.is_file():
                return False
            ext = path.suffix.lower()
            ctype = CONTENT_TYPES.get(ext, "text/plain")
            data = path.read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", ctype)
            self.send_header("Content-Length", str(len(data)))
            self.send_header("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0")
            self.end_headers()
            self.wfile.write(data)
            return True

        def read_json_body(self) -> dict | None:
            length = int(self.headers.get("Content-Length", "0") or "0")
            if length <= 0:
                return None
            raw = self.rfile.read(length)
            try:
                return json.loads(raw.decode("utf-8"))
            except (UnicodeDecodeError, json.JSONDecodeError):
                return None

        # ---- routes ----
        def do_GET(self):
            url = urlparse(self.path)
            path = url.path
            query = parse_qs(url.query)

            if path in ("/", "/index.html"):
                if not self.send_static(DATA_DIR / "index.html"):
                    self.send_error(500, "index.html missing")
                return

            if path == "/api/question":
                seen = parse_seen(query.get("seen", [""])[0])
                with state.lock:
                    q = state.pick_random(seen)
                if q is None:
                    if not state.questions:
                        self.send_json(503, {"error": "no questions loaded"})
                    else:
                        self.send_json(200, {"exhausted": True})
                    return
                self.send_json(200, q)
                return

            if path == "/api/state":
                ms_since = (
                    -1
                    if state.boot_ms == 0
                    else int(time.time() * 1000) - state.boot_ms
                )
                self.send_json(200, {
                    "mode":           state.last_teensy_mode,
                    "pattern":        state.last_teensy_pattern,
                    "bridge_ms_since": ms_since,
                    "beatActivated":  state.beat_activated,
                    "teensyBeat":     state.last_teensy_beat,
                })
                return

            if path == "/api/stats":
                self.send_json(200, {
                    "yes":             state.total_yes,
                    "no":              state.total_no,
                    "beatActivated":   state.beat_activated,
                    "beatActivations": state.beat_activations,
                })
                return

            if path == "/command":
                cmd = query.get("cmd", [""])[0]
                if not cmd:
                    self.send_json(400, {"error": "need cmd"})
                    return
                if cmd not in FIRE_RITUAL_COMMANDS:
                    self.send_json(400, {"error": "bad cmd"})
                    return
                with state.lock:
                    state.send_bridge_line(cmd)
                self.send_json(200, {"ok": True})
                return

            # Dev-only conveniences
            if path == "/__reset":
                state.reset()
                self.send_json(200, {"ok": True, "msg": "state reset"})
                return

            if path == "/__bridge":
                # Tail of "what the firmware would have sent to the Teensy"
                self.send_json(200, {"bridge": state.bridge_log[-50:]})
                return

            # Static fallback (style.css, app.js, etc.)
            relpath = path.lstrip("/")
            candidate = (DATA_DIR / relpath).resolve()
            if DATA_DIR in candidate.parents or candidate == DATA_DIR:
                if self.send_static(candidate):
                    return

            self.send_error(404, "not found")

        def do_POST(self):
            url = urlparse(self.path)
            path = url.path

            if path != "/api/answer":
                self.send_error(404, "not found")
                return

            ip = self.client_address[0]
            now = int(time.time() * 1000)
            with state.lock:
                if ip == state.last_ip and (now - state.last_answer_ms) < PER_IP_COOLDOWN_MS:
                    self.send_json(429, {"error": "slow down"})
                    return

            body = self.read_json_body()
            if body is None:
                self.send_json(400, {"error": "bad json"})
                return

            qid = body.get("qid", -1)
            answer = (body.get("answer") or "").strip()
            if qid < 0 or not answer:
                self.send_json(400, {"error": "need qid + answer"})
                return

            with state.lock:
                q = state.by_id.get(qid)
                if not q:
                    self.send_json(404, {"error": "unknown qid"})
                    return

                state.last_ip = ip
                state.last_answer_ms = now

                if q.get("type") == "riddle":
                    self._answer_riddle(q, answer)
                    return

                if q.get("type") == "choice":
                    self._answer_choice(q, answer)
                    return

                if answer not in ("yes", "no"):
                    self.send_json(400, {"error": "need yes/no"})
                    return

                self._answer_binary(q, answer)

        def _answer_binary(self, q: dict, answer: str):
            branch = q.get(answer, {}) or {}
            pattern = branch.get("pattern", -1)
            msg = branch.get("msg", "")
            if pattern >= 0:
                state.send_bridge_line(f"P:{pattern}")
            else:
                state.send_bridge_line(f"A:{answer}")
            if answer == "yes":
                state.total_yes += 1
            else:
                state.total_no += 1
            self.send_json(200, {"ok": True, "msg": msg, "pattern": pattern})

        def _answer_choice(self, q: dict, choice_id: str):
            wanted = (choice_id or "").lower().strip()
            for opt in q.get("options", []) or []:
                oid = str(opt.get("id", "")).lower().strip()
                if oid != wanted:
                    continue
                pattern = opt.get("pattern", -1)
                msg = opt.get("msg", "")
                if pattern >= 0:
                    state.send_bridge_line(f"P:{pattern}")
                self.send_json(200, {"ok": True, "msg": msg, "pattern": pattern})
                return
            self.send_json(400, {"error": "unknown choice"})

        def _answer_riddle(self, q: dict, user_ans: str):
            normalized = user_ans.lower().strip()
            candidates = [str(a).lower().strip() for a in q.get("answers", [])]
            correct = normalized in candidates
            if correct:
                action = q.get("success", {}).get("action", "")
                if action:
                    state.send_bridge_line(action)
                pattern = q.get("success", {}).get("pattern", -1)
                if pattern >= 0:
                    state.send_bridge_line(f"P:{pattern}")
                if not state.beat_activated:
                    state.beat_activated = True
                    state.beat_activations += 1
                msg = q.get("success", {}).get("msg", "you've activated the beat")
                self.send_json(200, {
                    "ok":            True,
                    "msg":           msg,
                    "beatActivated": True,
                })
            else:
                fail = q.get("fail_msg", "not quite…")
                self.send_json(200, {"ok": False, "msg": fail})

    return Handler


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--port", type=int, default=8080)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--open", action="store_true", help="open URL in default browser")
    args = ap.parse_args()

    questions_path = DATA_DIR / "questions.json"
    if not questions_path.is_file():
        log(f"[fatal] {questions_path} not found")
        sys.exit(1)

    state = State(questions_path)
    handler = make_handler(state)
    httpd = ThreadingHTTPServer((args.host, args.port), handler)

    url = f"http://{args.host}:{args.port}/"
    banner = (
        "\n"
        "─────────────────────────────────────────────────────\n"
        f"  ESP32 web-app dev server\n"
        f"  → {url}\n"
        f"  data:   {DATA_DIR}\n"
        f"  questions: {len(state.questions)} loaded\n"
        "  conveniences:\n"
        f"    GET {url}__reset    — clear state + reload questions.json\n"
        f"    GET {url}__bridge   — tail of mocked Teensy serial traffic\n"
        "  Ctrl-C to stop.\n"
        "─────────────────────────────────────────────────────\n"
    )
    log(banner)

    if args.open:
        threading.Timer(0.4, lambda: webbrowser.open(url)).start()

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        log("\n[shutdown] bye")
        httpd.server_close()


if __name__ == "__main__":
    main()
