// Vanilla JS — no framework. ~3KB.
//
// Question routing:
//   type=binary (or absent) → Yes/No buttons → POST /api/answer { answer:"yes"|"no" }
//   type=riddle             → free-text input → POST /api/answer { answer:"<text>" }
//                             client tracks attempts; reveals + advances on max_attempts

(() => {
  const $q          = document.getElementById('question');
  const $binary     = document.getElementById('binary-input');
  const $riddle     = document.getElementById('riddle-input');
  const $riddleText = document.getElementById('riddle-text');
  const $riddleHint = document.getElementById('riddle-hint');
  const $yes        = document.getElementById('btn-yes');
  const $no         = document.getElementById('btn-no');
  const $card       = document.getElementById('card');
  const $resp       = document.getElementById('response');
  const $respEmoji  = document.getElementById('response-emoji');
  const $respMsg    = document.getElementById('response-msg');
  const $respSub    = document.getElementById('response-sub');
  const $stats      = document.getElementById('stats');
  const $again      = document.getElementById('btn-again');
  const $fire       = document.getElementById('fire-stage');
  const $takeover      = document.getElementById('takeover');
  const $takeoverBlurb = document.getElementById('takeover-blurb');
  const $btnTakeover   = document.getElementById('btn-takeover');
  const $takeoverCount = document.getElementById('takeover-count');

  let current = null;
  let inFlight = false;
  let attemptsLeft = 0;
  let seenQids   = new Set();
  let fireTimers = [];

  // --- Fire-ritual finale -------------------------------------------------
  //
  // Triggered the moment the user runs out of unseen questions — i.e. the
  // server returns a qid that's already in `seenQids`. This auto-scales to
  // the pool size: 3 questions → fire after 3, 50 questions → fire after 50.
  // No constant to keep in sync with the pool.
  //
  // The ritual: "one more thing" → choose what to throw to the fire → "it's
  // burning" → "it's gone, find your friends".
  //
  // What they chose is PRIVATE — never logged, never sent to the Teensy. The
  // Teensy only ever sees "P:fire_full" regardless of which option was tapped.
  const TRANSITION_MS    = 2000;
  const FIRE_DURATION_MS = 10000;
  const GOODBYE_MS       = 6000;
  const FIRE_OPTIONS = [
    "a regret",
    "someone's name",
    "a bad year",
    "a version of you",
    "what they said",
    "the thing you carry",
  ];

  async function fetchJSON(url, opts) {
    const r = await fetch(url, opts);
    if (!r.ok) throw new Error('HTTP ' + r.status);
    return r.json();
  }

  // --- Takeover gate ------------------------------------------------------
  //
  // The app boots into this gate instead of straight into the questions. Tap
  // "Take over the lights" → 3s countdown (client-side only, lights keep their
  // current pattern) → POST /api/takeover claims control at the current pattern
  // (so nothing visibly jumps) → the question flow starts. The operator can
  // reclaim instantly at any time by moving a physical switch.
  const TAKEOVER_SECONDS = 3;
  let takeoverTimer = null;

  function startTakeover() {
    if (takeoverTimer) return;            // already counting down
    $btnTakeover.classList.add('hidden');
    $takeoverBlurb.classList.add('hidden');
    let n = TAKEOVER_SECONDS;
    $takeoverCount.textContent = n;
    $takeoverCount.classList.remove('hidden');
    takeoverTimer = setInterval(() => {
      n -= 1;
      if (n <= 0) {
        clearInterval(takeoverTimer);
        takeoverTimer = null;
        finishTakeover();
      } else {
        $takeoverCount.textContent = n;
      }
    }, 1000);
  }

  function finishTakeover() {
    // Claim control at whatever pattern is currently showing, then begin the
    // normal question flow. Fire-and-forget — we don't block the UI on it.
    fetch('/api/takeover', { method: 'POST' }).catch(() => {});
    $takeover.classList.add('hidden');
    $card.style.display = '';
    loadNext();
    refreshStats();
  }

  function isRiddle(q) { return q && q.type === 'riddle'; }

  function showQuestion(q) {
    current = q;
    $q.textContent = q.q;
    $resp.classList.add('hidden');
    $card.style.display = '';

    if (isRiddle(q)) {
      attemptsLeft = q.max_attempts || 3;
      $binary.classList.add('hidden');
      $riddle.classList.remove('hidden');
      $riddleText.value = '';
      $riddleHint.textContent = '';
      $riddleHint.classList.remove('success');
      // Don't autofocus — opens keyboard immediately, can feel aggressive.
      // Users tap the input naturally.
    } else {
      $riddle.classList.add('hidden');
      $binary.classList.remove('hidden');
      // Optional per-question button labels (e.g. "Fold" / "Scrunch").
      // Fall back to plain Yes / No when not specified.
      $yes.textContent = (q.yes && q.yes.label) ? q.yes.label : 'Yes';
      $no.textContent  = (q.no  && q.no.label)  ? q.no.label  : 'No';
      $yes.disabled = false;
      $no.disabled  = false;
    }
  }

  function lockBinary() { $yes.disabled = true; $no.disabled = true; }
  function lockRiddle() { $riddleText.disabled = true; }
  function unlockRiddle() { $riddleText.disabled = false; }

  function showResponse({ emoji, msg, sub, celebrate }) {
    $respEmoji.textContent = emoji || '';
    $respMsg.textContent   = msg || '…';
    $respMsg.classList.toggle('celebrate', !!celebrate);
    if (sub) {
      $respSub.textContent = sub;
      $respSub.classList.remove('hidden');
    } else {
      $respSub.classList.add('hidden');
    }
    $card.style.display = 'none';
    $resp.classList.remove('hidden');
  }

  // Advance to the next question. No auto-advance — the user must tap the
  // "and i go at it again" button on the response screen (Paddy Losty).
  // This gives the user as long as they want to read the reply and look up
  // at the lights.
  //
  // Marks the just-completed question as seen; loadNext() will trigger the
  // fire ritual if the server returns a qid we've already answered (i.e.,
  // we've exhausted the unseen pool).
  function advance() {
    $card.style.display = 'none';
    $resp.classList.add('hidden');
    inFlight = false;
    if (current && current.id !== undefined) {
      seenQids.add(current.id);
    }
    loadNext();
    refreshStats();
  }

  // --- Fire ritual --------------------------------------------------------

  function sendCommand(cmd) {
    // Fire-and-forget. We don't care about the response — the lights take
    // their own time, and we don't want to block the UI on a network call.
    fetch('/command?cmd=' + encodeURIComponent(cmd)).catch(() => {});
  }

  function cancelFireTimers() {
    fireTimers.forEach(clearTimeout);
    fireTimers = [];
  }

  function hideAllStages() {
    $card.style.display = 'none';
    $resp.classList.add('hidden');
  }

  function showFireStage(html) {
    // Force-restart the fadeIn animation by re-inserting the node.
    $fire.classList.remove('hidden');
    $fire.innerHTML = html;
  }

  function hideFireStage() {
    $fire.classList.add('hidden');
    $fire.innerHTML = '';
  }

  function startFireSequence() {
    cancelFireTimers();
    hideAllStages();
    sendCommand('P:dim_ambient');
    showFireTransition();
  }

  // Screen 1 — soft fade, "one more thing.", 2s hold
  function showFireTransition() {
    showFireStage('<p class="fire-transition">one more thing.</p>');
    fireTimers.push(setTimeout(showFireChoice, TRANSITION_MS));
  }

  // Screen 2 — choose what to throw to the fire. Six options. No auto-advance.
  function showFireChoice() {
    const optsHtml = FIRE_OPTIONS.map((opt, i) =>
      `<button class="fire-option" type="button" data-i="${i}">${escapeHtml(opt)}</button>`
    ).join('');
    showFireStage(
      '<p class="fire-emoji" aria-hidden="true">🔥</p>' +
      '<p class="fire-choice-prompt">choose something<br>to let go<br>and throw to the fire</p>' +
      '<div class="fire-options" role="group">' + optsHtml + '</div>'
    );
    // Wire up the buttons. We don't read the data-i — the choice is private,
    // we just need to know that *something* was tapped.
    $fire.querySelectorAll('.fire-option').forEach((btn) => {
      btn.addEventListener('click', () => onFireChoiceTapped(), { once: true });
    });
  }

  function onFireChoiceTapped() {
    // Disable all the buttons immediately so a double-tap doesn't re-fire.
    $fire.querySelectorAll('.fire-option').forEach((b) => (b.disabled = true));
    sendCommand('P:fire_full');
    showFireBurning();
  }

  // Screen 3 — burning, 10s, three flickering emojis
  function showFireBurning() {
    showFireStage(
      '<p class="fire-trio" aria-hidden="true"><span>🔥</span><span>🔥</span><span>🔥</span></p>' +
      '<p class="fire-burning-msg">it\'s burning</p>' +
      '<p class="fire-burning-sub">look up <span aria-hidden="true">↑</span></p>' +
      '<p class="fire-burning-tail">the lights are taking it</p>'
    );
    fireTimers.push(setTimeout(showGoodbye, FIRE_DURATION_MS));
  }

  // Screen 4 — "it's gone", 6s, then reset
  function showGoodbye() {
    sendCommand('P:idle_ambient');
    showFireStage(
      '<p class="goodbye-headline">it\'s gone</p>' +
      '<p class="goodbye-body">thanks for playing.<br>find your friends.<br>drink some water.</p>'
    );
    fireTimers.push(setTimeout(resetSession, GOODBYE_MS));
  }

  function resetSession() {
    cancelFireTimers();
    hideFireStage();
    seenQids.clear();
    inFlight = false;
    current = null;
    loadNext();
    refreshStats();
  }

  function escapeHtml(s) {
    return String(s).replace(/[&<>"']/g, (c) => ({
      '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;',
    }[c]));
  }

  async function loadNext() {
    try {
      // Pass our seen qids so the server can exclude them. For a pool of N
      // the user runs out after N answers regardless of pool size — server
      // returns {"exhausted":true} and we hand off to the fire ritual.
      const seen = Array.from(seenQids).join(',');
      const url = '/api/question' + (seen ? '?seen=' + seen : '');
      const q = await fetchJSON(url);
      if (q && q.exhausted) {
        startFireSequence();
        return;
      }
      // Safety net: if the server somehow returned a seen qid (older
      // firmware without the ?seen= filter, or a race), still trigger fire.
      if (q && q.id !== undefined && seenQids.has(q.id)) {
        startFireSequence();
        return;
      }
      showQuestion(q);
    } catch (err) {
      $q.textContent = 'No questions available — check the controller.';
      lockBinary();
    }
  }

  // ----- Binary answer -----
  async function answerBinary(yesOrNo) {
    if (inFlight || !current) return;
    inFlight = true;
    lockBinary();
    try {
      const resp = await fetchJSON('/api/answer', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ qid: current.id, answer: yesOrNo }),
      });
      showResponse({ msg: resp.msg });
    } catch (err) {
      showResponse({ msg: 'one sec…' });
    }
    // Response stays until the user taps "and i go at it again".
  }

  // ----- Riddle answer -----
  async function answerRiddle(text) {
    if (inFlight || !current) return;
    const normalized = (text || '').toLowerCase().trim();
    if (!normalized) return;
    inFlight = true;
    lockRiddle();

    try {
      const resp = await fetchJSON('/api/answer', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ qid: current.id, answer: normalized }),
      });

      if (resp.ok) {
        // Correct! Magic moment. Response stays until the user taps again.
        showResponse({
          emoji: '🎤',
          msg: resp.msg || "you've activated the beat",
          sub: 'the lights now hear what you hear',
          celebrate: true,
        });
      } else {
        // Wrong answer. Server returns 200 with ok:false + fail msg.
        attemptsLeft--;
        if (attemptsLeft <= 0) {
          // Out of attempts — reveal and wait for them to tap onward.
          showResponse({
            emoji: '🎤',
            msg: current.reveal_msg || 'the answer was the mic',
          });
        } else {
          $riddleHint.textContent =
            (resp.msg || 'not quite…') +
            '  ·  ' + attemptsLeft + ' attempt' + (attemptsLeft === 1 ? '' : 's') + ' left';
          $riddleText.value = '';
          unlockRiddle();
          $riddleText.focus();
          inFlight = false;
        }
      }
    } catch (err) {
      $riddleHint.textContent = 'one sec…';
      unlockRiddle();
      inFlight = false;
    }
  }

  async function refreshStats() {
    try {
      const s = await fetchJSON('/api/stats');
      const total = (s.yes || 0) + (s.no || 0);
      const beat  = s.beatActivated ? ' · beat is on 🎤' : '';
      if (total > 0) {
        const yesPct = Math.round((s.yes / total) * 100);
        $stats.textContent =
          yesPct + '% have said yes tonight · ' + total + ' answers' + beat;
      } else {
        $stats.textContent = 'be the first to answer' + beat;
      }
    } catch (_) {
      $stats.textContent = '';
    }
  }

  $yes.addEventListener('click', () => answerBinary('yes'));
  $no .addEventListener('click', () => answerBinary('no'));
  $again.addEventListener('click', () => advance());
  $btnTakeover.addEventListener('click', () => startTakeover());
  $riddle.addEventListener('submit', (e) => {
    e.preventDefault();
    answerRiddle($riddleText.value);
  });

  // Boot — gate the experience behind the takeover countdown. The question
  // flow does not start until the user taps through and the 5s countdown ends.
  $card.style.display = 'none';
  $takeover.classList.remove('hidden');
  refreshStats();
  setInterval(refreshStats, 10000);
})();
