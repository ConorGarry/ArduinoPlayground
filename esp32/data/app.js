// Vanilla JS — no framework. Drives the meta sonic flux captive portal.
//
// Question routing:
//   type=binary (or absent) → yes/no buttons → POST /api/answer { answer:"yes"|"no" }
//   type=riddle             → free-text input → POST /api/answer { answer:"<text>" }
//                             client tracks attempts; reveals + advances on max_attempts

(() => {
  const $q          = document.getElementById('question');
  const $binary     = document.getElementById('binary-input');
  const $choice     = document.getElementById('choice-input');
  const $riddle     = document.getElementById('riddle-input');
  const $riddleText = document.getElementById('riddle-text');
  const $riddleHint = document.getElementById('riddle-hint');
  const $yes        = document.getElementById('btn-yes');
  const $no         = document.getElementById('btn-no');
  const $card       = document.getElementById('card');
  const $resp       = document.getElementById('response');
  const $respMsg    = document.getElementById('response-msg');
  const $respSub    = document.getElementById('response-sub');
  const $stats      = document.getElementById('stats');
  const $again      = document.getElementById('btn-again');
  const $fire       = document.getElementById('fire-stage');
  const $pulse      = document.getElementById('pulse');
  const $takeover      = document.getElementById('takeover');
  const $takeoverBlurb = document.getElementById('takeover-blurb');
  const $btnTakeover   = document.getElementById('btn-takeover');
  const $takeoverCount = document.getElementById('takeover-count');
  const $holding       = document.getElementById('holding');

  let current = null;
  let inFlight = false;
  let attemptsLeft = 0;
  let seenQids   = new Set();
  let fireTimers = [];

  // Header pulse indicator — adopts the surface accent to signal state.
  // states: idle | loading | active | live
  function setPulse(state, label) {
    $pulse.dataset.state = state;
    $pulse.textContent = label;
  }

  // Remount any [data-msf-shape] canvases inside a region after innerHTML swap.
  // Particles + the takeover mark auto-mount on DOMContentLoaded; this is for
  // anything injected dynamically (fire stages).
  function mountMarks(root) {
    if (window.MSFGeometry) window.MSFGeometry.auto(root);
  }

  // --- Fire-ritual finale -------------------------------------------------
  //
  // Triggered the moment the user runs out of unseen questions — i.e. the
  // server returns a qid that's already in `seenQids`. Auto-scales to pool
  // size: 3 questions → fire after 3, 50 questions → fire after 50.
  //
  // The ritual: "// one more thing" → choose what to throw to the fire →
  // "it's burning" → "it's gone, find your friends".
  //
  // What they chose is PRIVATE — never logged, never sent to the Teensy.
  // The Teensy only ever sees "P:fire_full" regardless of which option was tapped.
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
    if (!r.ok) {
      const e = new Error('HTTP ' + r.status);
      e.status = r.status;          // 409 = our controller slot was lost
      throw e;
    }
    return r.json();
  }

  // --- Takeover gate ------------------------------------------------------
  //
  // The app boots into this gate instead of straight into the questions. Tap
  // "{connect} // take the controls" → 3s countdown (client-side only, lights
  // keep their current pattern) → POST /api/takeover claims control at the
  // current pattern (so nothing visibly jumps) → the question flow starts.
  // The operator can reclaim instantly at any time by moving a physical switch.
  const TAKEOVER_SECONDS = 3;
  const HOLDING_POLL_MS  = 3000;   // re-try acquiring the slot while waiting
  const HEARTBEAT_MS     = 8000;   // keep our slot alive while active
  let takeoverTimer = null;
  let holdingTimer  = null;
  let heartbeatTimer = null;

  function startTakeover() {
    if (takeoverTimer) return;            // already counting down
    setPulse('loading', 'establishing link');
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

  async function finishTakeover() {
    // Try to claim the single controller slot. If someone else holds it, drop
    // into the holding screen and keep retrying; otherwise enter the flow.
    try {
      const r = await fetchJSON('/api/takeover', { method: 'POST' });
      if (r && r.busy) { showHolding(); return; }
    } catch (_) {
      showHolding();   // network blip — wait it out on the holding screen
      return;
    }
    enterExperience();
  }

  // We hold the slot — start the question flow and begin heartbeating so the
  // server knows we're still here (even while reading a long riddle).
  function enterExperience() {
    hideHolding();
    $takeover.classList.add('hidden');
    setPulse('active', 'live now');
    $card.style.display = '';
    startHeartbeat();
    loadNext();
    refreshStats();
  }

  // --- Holding screen (someone else is at the controls) -------------------
  function showHolding() {
    $takeover.classList.add('hidden');
    hideAllStages();
    hideFireStage();
    stopHeartbeat();
    $holding.classList.remove('hidden');
    $holding.innerHTML =
      '<canvas class="mark anim-breath" data-msf-shape="singularity" ' +
        'data-color="#FF8A00" data-speed="0.6" aria-hidden="true"></canvas>' +
      '<div>' +
        '<p class="display title">hold tight</p>' +
        '<p class="takeover-blurb">someone is inside infinity right now.<br>' +
        'you\'ll enter the moment it\'s free.</p>' +
      '</div>' +
      '<p class="takeover-count">// waiting for the controls</p>';
    mountMarks($holding);
    setPulse('loading', 'in queue');
    if (!holdingTimer) holdingTimer = setInterval(tryAcquire, HOLDING_POLL_MS);
  }

  function hideHolding() {
    if (holdingTimer) { clearInterval(holdingTimer); holdingTimer = null; }
    $holding.classList.add('hidden');
    $holding.innerHTML = '';
  }

  async function tryAcquire() {
    try {
      const r = await fetchJSON('/api/takeover', { method: 'POST' });
      if (r && r.ok) enterExperience();   // hideHolding() runs inside
    } catch (_) { /* keep waiting */ }
  }

  // Bounce back to the holding screen if we lose the slot mid-session (idle
  // timeout fired, or the operator reclaimed). Heartbeat / 409s call this.
  function bounceToHolding() {
    cancelFireTimers();
    current = null;
    inFlight = false;
    seenQids.clear();
    showHolding();
  }

  function startHeartbeat() {
    stopHeartbeat();
    heartbeatTimer = setInterval(async () => {
      try {
        const r = await fetchJSON('/api/heartbeat');
        if (r && r.mine === false) bounceToHolding();
      } catch (_) { /* transient — next beat retries */ }
    }, HEARTBEAT_MS);
  }

  function stopHeartbeat() {
    if (heartbeatTimer) { clearInterval(heartbeatTimer); heartbeatTimer = null; }
  }

  function isRiddle(q) { return q && q.type === 'riddle'; }
  function isChoice(q) { return q && q.type === 'choice'; }

  function showQuestion(q) {
    current = q;
    $q.textContent = q.q;
    $resp.classList.add('hidden');
    $card.style.display = '';

    if (isRiddle(q)) {
      attemptsLeft = q.max_attempts || 3;
      $binary.classList.add('hidden');
      $choice.classList.add('hidden');
      $riddle.classList.remove('hidden');
      $riddleText.value = '';
      $riddleHint.textContent = '';
      $riddleHint.classList.remove('success');
      // Don't autofocus — opens keyboard immediately, can feel aggressive.
    } else if (isChoice(q)) {
      $binary.classList.add('hidden');
      $riddle.classList.add('hidden');
      renderChoices(q.options || []);
      $choice.classList.remove('hidden');
    } else {
      $riddle.classList.add('hidden');
      $choice.classList.add('hidden');
      $binary.classList.remove('hidden');
      // Optional per-question button labels (e.g. "Fold" / "Scrunch").
      // CSS lowercases everything visually — content stays as authored.
      $yes.textContent = (q.yes && q.yes.label) ? q.yes.label : 'yes';
      $no.textContent  = (q.no  && q.no.label)  ? q.no.label  : 'no';
      $yes.disabled = false;
      $no.disabled  = false;
    }
  }

  // Build N choice buttons for a type:choice question. Each button posts the
  // option `id` as the answer; the server matches and dispatches the pattern.
  function renderChoices(options) {
    $choice.innerHTML = '';
    options.forEach((opt) => {
      const btn = document.createElement('button');
      btn.type = 'button';
      btn.className = 'choice';
      btn.dataset.id = opt.id;
      btn.textContent = '↳ ' + (opt.label || opt.id);
      btn.addEventListener('click', () => answerChoice(opt.id), { once: false });
      $choice.appendChild(btn);
    });
  }

  function lockChoice() {
    $choice.querySelectorAll('button').forEach((b) => (b.disabled = true));
  }

  function lockBinary() { $yes.disabled = true; $no.disabled = true; }
  function lockRiddle() { $riddleText.disabled = true; }
  function unlockRiddle() { $riddleText.disabled = false; }

  function showResponse({ msg, sub, celebrate }) {
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
  // "↳ and i go at it again" button on the response screen (Paddy Losty).
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
    $fire.classList.remove('hidden');
    $fire.innerHTML = html;
    mountMarks($fire);            // canvases inside the stage become live wireframes
  }

  function hideFireStage() {
    $fire.classList.add('hidden');
    $fire.innerHTML = '';
  }

  function startFireSequence() {
    cancelFireTimers();
    hideAllStages();
    setPulse('loading', 'handover');
    sendCommand('P:dim_ambient');
    showFireTransition();
  }

  // Screen 1 — soft fade, "// one more thing", 2s hold
  function showFireTransition() {
    showFireStage('<p class="fire-transition">// one more thing</p>');
    fireTimers.push(setTimeout(showFireChoice, TRANSITION_MS));
  }

  // Screen 2 — choose what to throw to the fire. Six options. No auto-advance.
  // The singularity ring stands in for the fire — wireframe, amber, glowing.
  function showFireChoice() {
    const optsHtml = FIRE_OPTIONS.map((opt, i) =>
      `<button class="fire-option" type="button" data-i="${i}">${escapeHtml(opt)}</button>`
    ).join('');
    showFireStage(
      '<canvas class="fire-mark" data-msf-shape="singularity" data-color="#FF8A00" data-speed="1.4" aria-hidden="true"></canvas>' +
      '<p class="fire-choice-prompt">choose something to let go<br>and throw to the fire</p>' +
      '<p class="fire-prompt-line">↳ private. nothing leaves this phone.</p>' +
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

  // Screen 3 — burning, 10s. Singularity rings spin faster, amber glow.
  function showFireBurning() {
    showFireStage(
      '<canvas class="fire-mark" data-msf-shape="singularity" data-color="#FF8A00" data-speed="2.4" aria-hidden="true"></canvas>' +
      '<p class="fire-burning-msg">it\'s burning</p>' +
      '<p class="fire-burning-sub">look up <span class="arrow" aria-hidden="true">↑</span></p>' +
      '<p class="fire-burning-tail">// infinity is taking it</p>'
    );
    fireTimers.push(setTimeout(showGoodbye, FIRE_DURATION_MS));
  }

  // Screen 4 — "it's gone", 6s, then reset
  function showGoodbye() {
    sendCommand('P:idle_ambient');
    setPulse('idle', 'idle');
    showFireStage(
      '<p class="goodbye-headline">it\'s gone</p>' +
      '<p class="goodbye-body">thanks for playing.<br>find your friends.<br>drink some water.</p>'
    );
    fireTimers.push(setTimeout(resetSession, GOODBYE_MS));
  }

  function resetSession() {
    cancelFireTimers();
    hideFireStage();
    stopHeartbeat();
    // Hand the controller slot back so the next person isn't waiting out the
    // idle timeout. Fire-and-forget.
    fetch('/api/release', { method: 'POST' }).catch(() => {});
    seenQids.clear();
    inFlight = false;
    current = null;
    // Back to the takeover gate so the next visitor starts fresh.
    $takeover.classList.remove('hidden');
    $btnTakeover.classList.remove('hidden');
    $takeoverBlurb.classList.remove('hidden');
    $takeoverCount.classList.add('hidden');
    $card.style.display = 'none';
    setPulse('idle', 'idle');
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
      if (err.status === 409) { bounceToHolding(); return; }   // lost our slot
      $q.textContent = '// no questions, check the controller';
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
      if (err.status === 409) { bounceToHolding(); return; }
      showResponse({ msg: '// one sec' });
    }
    // Response stays until the user taps "↳ and i go at it again".
  }

  // ----- Choice answer (type:choice) -----
  async function answerChoice(optionId) {
    if (inFlight || !current) return;
    inFlight = true;
    lockChoice();
    try {
      const resp = await fetchJSON('/api/answer', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ qid: current.id, answer: optionId }),
      });
      showResponse({ msg: resp.msg });
    } catch (err) {
      if (err.status === 409) { bounceToHolding(); return; }
      showResponse({ msg: '// one sec' });
    }
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
        // Correct. Lime = live / urgent — the beat is on.
        setPulse('live', 'beat is on');
        showResponse({
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
            msg: current.reveal_msg || 'the answer was the mic',
          });
        } else {
          $riddleHint.textContent =
            (resp.msg || 'not quite') +
            '  ·  ' + attemptsLeft + ' attempt' + (attemptsLeft === 1 ? '' : 's') + ' left';
          $riddleText.value = '';
          unlockRiddle();
          $riddleText.focus();
          inFlight = false;
        }
      }
    } catch (err) {
      if (err.status === 409) { bounceToHolding(); return; }
      $riddleHint.textContent = '// one sec';
      unlockRiddle();
      inFlight = false;
    }
  }

  async function refreshStats() {
    try {
      const s = await fetchJSON('/api/stats');
      const total = (s.yes || 0) + (s.no || 0);
      const beat  = s.beatActivated ? ' · <span class="beat">beat is on</span>' : '';
      if (total > 0) {
        const yesPct = Math.round((s.yes / total) * 100);
        $stats.innerHTML =
          '<span class="pct">' + yesPct + '%</span> have said yes tonight · ' +
          total + ' answers' + beat;
      } else {
        $stats.innerHTML = 'be the first to answer' + beat;
      }
    } catch (_) {
      $stats.textContent = '';
    }
  }

  $yes.addEventListener('click', () => answerBinary('yes'));
  $no .addEventListener('click', () => answerBinary('no'));
  $again.addEventListener('click', () => advance());
  $btnTakeover.addEventListener('click', () => startTakeover());

  // Free the controller slot promptly when the active phone leaves (tab close,
  // Wi-Fi drop). No-op server-side if we weren't the active controller.
  window.addEventListener('pagehide', () => {
    if (navigator.sendBeacon) navigator.sendBeacon('/api/release');
  });
  $riddle.addEventListener('submit', (e) => {
    e.preventDefault();
    answerRiddle($riddleText.value);
  });

  // Boot — gate the experience behind the takeover countdown. The question
  // flow does not start until the user taps through and the 3s countdown ends.
  $card.style.display = 'none';
  $takeover.classList.remove('hidden');
  setPulse('idle', 'idle');
  refreshStats();
  setInterval(refreshStats, 10000);
})();
