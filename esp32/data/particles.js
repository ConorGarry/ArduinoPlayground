/* ============================================================
   meta sonic flux // particle field
   single-pixel dust catching light in festival haze.
   opacity 0.2-0.4, drifts ~1px / 2-3s. hero + section-break only.

   usage: <canvas data-msf-particles></canvas>
          MSFParticles.auto();
   ============================================================ */
(function (global) {
  'use strict';
  var REDUCED = global.matchMedia &&
    global.matchMedia('(prefers-reduced-motion: reduce)').matches;

  function mount(canvas, opts) {
    opts = opts || {};
    var density = opts.density || 0.00018;   // particles per px^2
    var color = opts.color || '#F2EFE9';
    var ctx = canvas.getContext('2d');
    var dpr = Math.min(global.devicePixelRatio || 1, 2);
    var parts = [], w, h;

    function seed() {
      var r = canvas.getBoundingClientRect();
      w = r.width || 300; h = r.height || 300;
      canvas.width = w * dpr; canvas.height = h * dpr;
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
      var n = Math.floor(w * h * density);
      parts = [];
      for (var i = 0; i < n; i++) {
        parts.push({
          x: Math.random() * w, y: Math.random() * h,
          a: 0.2 + Math.random() * 0.2,
          vx: (Math.random() - 0.5) * 0.06,
          vy: (Math.random() - 0.5) * 0.06,
          tw: Math.random() * Math.PI * 2
        });
      }
    }
    seed();
    var ro = global.ResizeObserver ? new ResizeObserver(seed) : null;
    if (ro) ro.observe(canvas);

    function frame(now) {
      ctx.clearRect(0, 0, w, h);
      for (var i = 0; i < parts.length; i++) {
        var p = parts[i];
        if (!REDUCED) {
          p.x += p.vx; p.y += p.vy; p.tw += 0.01;
          if (p.x < 0) p.x += w; if (p.x > w) p.x -= w;
          if (p.y < 0) p.y += h; if (p.y > h) p.y -= h;
        }
        var a = p.a * (REDUCED ? 1 : (0.7 + Math.sin(p.tw) * 0.3));
        ctx.globalAlpha = a;
        ctx.fillStyle = color;
        ctx.fillRect(p.x, p.y, 1, 1);
      }
      ctx.globalAlpha = 1;
      raf = requestAnimationFrame(frame);
    }
    var raf = requestAnimationFrame(frame);
    return { stop: function () { cancelAnimationFrame(raf); if (ro) ro.disconnect(); } };
  }

  function auto(root) {
    (root || document).querySelectorAll('[data-msf-particles]').forEach(function (c) {
      if (!c.__msfp) c.__msfp = mount(c, {
        color: c.getAttribute('data-color') || '#F2EFE9'
      });
    });
  }
  global.MSFParticles = { mount: mount, auto: auto };
  if (document.readyState !== 'loading') auto();
  else document.addEventListener('DOMContentLoaded', function () { auto(); });
})(window);
