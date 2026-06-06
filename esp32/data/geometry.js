/* ============================================================
   meta sonic flux // geometric inventory
   the four marks, rendered as glowing 3d wireframes.
   these are the brand's icon set. never flat icons.

   usage:
     <canvas data-msf-shape="tesseract" data-color="#00E5FF"></canvas>
     MSFGeometry.auto();            // mount every [data-msf-shape]
   or:
     MSFGeometry.mount(canvasEl, { shape:'dodeca', color:'#FF0099',
                                   speed:1, glow:true });
   shapes: dodeca | tesseract | triskelion | singularity
   ============================================================ */
(function (global) {
  'use strict';

  var REDUCED = global.matchMedia &&
    global.matchMedia('(prefers-reduced-motion: reduce)').matches;

  /* ---- linear algebra helpers ---- */
  function rotXYZ(p, ax, ay, az) {
    var x = p[0], y = p[1], z = p[2], c, s, t;
    c = Math.cos(ax); s = Math.sin(ax); t = y * c - z * s; z = y * s + z * c; y = t;
    c = Math.cos(ay); s = Math.sin(ay); t = x * c + z * s; z = -x * s + z * c; x = t;
    c = Math.cos(az); s = Math.sin(az); t = x * c - y * s; y = x * s + y * c; x = t;
    return [x, y, z];
  }

  /* ---- geometry builders ---- */
  function buildDodeca() {
    var phi = (1 + Math.sqrt(5)) / 2, ip = 1 / phi;
    var v = [], s = [-1, 1];
    for (var i = 0; i < 2; i++) for (var j = 0; j < 2; j++) for (var k = 0; k < 2; k++)
      v.push([s[i], s[j], s[k]]);
    for (var a = 0; a < 2; a++) for (var b = 0; b < 2; b++) {
      v.push([0, s[a] * ip, s[b] * phi]);
      v.push([s[a] * ip, s[b] * phi, 0]);
      v.push([s[a] * phi, 0, s[b] * ip]);
    }
    // edges = pairs at minimum distance
    var edges = [], min = Infinity, d, dx, dy, dz;
    for (var m = 0; m < v.length; m++) for (var n = m + 1; n < v.length; n++) {
      dx = v[m][0] - v[n][0]; dy = v[m][1] - v[n][1]; dz = v[m][2] - v[n][2];
      d = Math.sqrt(dx * dx + dy * dy + dz * dz);
      if (d < min) min = d;
    }
    for (var p = 0; p < v.length; p++) for (var q = p + 1; q < v.length; q++) {
      dx = v[p][0] - v[q][0]; dy = v[p][1] - v[q][1]; dz = v[p][2] - v[q][2];
      d = Math.sqrt(dx * dx + dy * dy + dz * dz);
      if (Math.abs(d - min) < 0.01) edges.push([p, q]);
    }
    return { verts: v, edges: edges, scale: 0.42 };
  }

  function buildTesseract() {
    var v = [], s = [-1, 1];
    for (var i = 0; i < 2; i++) for (var j = 0; j < 2; j++)
      for (var k = 0; k < 2; k++) for (var l = 0; l < 2; l++)
        v.push([s[i], s[j], s[k], s[l]]);
    var edges = [];
    for (var m = 0; m < v.length; m++) for (var n = m + 1; n < v.length; n++) {
      var diff = 0;
      for (var c = 0; c < 4; c++) if (v[m][c] !== v[n][c]) diff++;
      if (diff === 1) edges.push([m, n]);
    }
    return { verts4: v, edges: edges, scale: 0.5 };
  }

  /* ---- the renderer ---- */
  function mount(canvas, opts) {
    opts = opts || {};
    var shape = opts.shape || canvas.getAttribute('data-msf-shape') || 'dodeca';
    var color = opts.color || canvas.getAttribute('data-color') || '#FF0099';
    var speed = opts.speed != null ? opts.speed
      : parseFloat(canvas.getAttribute('data-speed') || '1');
    var glow = opts.glow !== false;
    var lineW = opts.lineWidth || 1.4;
    var ctx = canvas.getContext('2d');
    var dpr = Math.min(global.devicePixelRatio || 1, 2);

    var geo;
    if (shape === 'dodeca') geo = buildDodeca();
    else if (shape === 'tesseract') geo = buildTesseract();

    function size() {
      var r = canvas.getBoundingClientRect();
      var w = r.width || 120, h = r.height || 120;
      canvas.width = w * dpr; canvas.height = h * dpr;
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
      return { w: w, h: h };
    }
    var dim = size();
    var ro = global.ResizeObserver ? new ResizeObserver(function () { dim = size(); }) : null;
    if (ro) ro.observe(canvas);

    var t0 = performance.now();

    function project3(p, w, h, persp) {
      var d = persp / (persp + p[2]);
      return [w / 2 + p[0] * d, h / 2 + p[1] * d, p[2]];
    }

    function strokeEdge(a, b, alpha) {
      ctx.globalAlpha = alpha;
      ctx.beginPath();
      ctx.moveTo(a[0], a[1]); ctx.lineTo(b[0], b[1]);
      ctx.stroke();
    }

    function frame(now) {
      var t = (now - t0) / 1000;
      var w = dim.w, h = dim.h, R = Math.min(w, h);
      ctx.clearRect(0, 0, w, h);
      ctx.strokeStyle = color;
      ctx.fillStyle = color;
      ctx.lineWidth = lineW;
      ctx.lineCap = 'round';
      if (glow) { ctx.shadowColor = color; ctx.shadowBlur = R * 0.06; }

      var spin = REDUCED ? 0 : t * speed;

      if (shape === 'dodeca') {
        var sc = R * geo.scale;
        var pts = geo.verts.map(function (vv) {
          var p = rotXYZ([vv[0] * sc, vv[1] * sc, vv[2] * sc], spin * 0.25, spin * 0.35, 0);
          return project3(p, w, h, R * 1.6);
        });
        geo.edges.forEach(function (e) { strokeEdge(pts[e[0]], pts[e[1]], 0.95); });
      }
      else if (shape === 'tesseract') {
        // fold: 4d rotation in xw + yw planes
        var fold = REDUCED ? 0.6 : (Math.sin(t * 0.4 * speed) * 0.5 + 0.5);
        var aw = fold * Math.PI;            // ana-kata fold
        var sc = R * geo.scale;
        var pts = geo.verts4.map(function (vv) {
          var x = vv[0], y = vv[1], z = vv[2], wc = vv[3];
          // rotate in x-w plane (the fold)
          var c = Math.cos(aw), s = Math.sin(aw);
          var nx = x * c - wc * s, nw = x * s + wc * c;
          // rotate in y-w
          var c2 = Math.cos(aw * 0.6), s2 = Math.sin(aw * 0.6);
          var ny = y * c2 - nw * s2; nw = y * s2 + nw * c2;
          // 4d->3d perspective
          var k = 2 / (2.4 - nw);
          var p3 = [nx * k * sc, ny * k * sc, z * k * sc];
          p3 = rotXYZ(p3, spin * 0.2, spin * 0.3, 0);
          return project3(p3, w, h, R * 2.2);
        });
        geo.edges.forEach(function (e) { strokeEdge(pts[e[0]], pts[e[1]], 0.9); });
      }
      else if (shape === 'triskelion') {
        // triple spiral, 3 arms at 120deg, rotating
        ctx.save();
        ctx.translate(w / 2, h / 2);
        ctx.rotate(spin * 0.5);
        var arms = 3, turns = 1.15, steps = 60, maxR = R * 0.42;
        for (var a = 0; a < arms; a++) {
          ctx.save();
          ctx.rotate((a / arms) * Math.PI * 2);
          ctx.beginPath();
          for (var i = 0; i <= steps; i++) {
            var f = i / steps;
            var ang = f * turns * Math.PI * 2;
            var rr = f * maxR;
            var x = Math.cos(ang) * rr, y = Math.sin(ang) * rr;
            if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
          }
          ctx.globalAlpha = 0.95; ctx.stroke();
          ctx.restore();
        }
        ctx.restore();
      }
      else if (shape === 'singularity') {
        ctx.save();
        ctx.translate(w / 2, h / 2);
        // event horizon rings (tilted ellipses)
        var rings = 3;
        for (var r = 0; r < rings; r++) {
          var rad = R * (0.18 + r * 0.11);
          var breathe = REDUCED ? 1 : (1 + Math.sin(t * 1.2 + r) * 0.03);
          ctx.globalAlpha = 0.85 - r * 0.22;
          ctx.beginPath();
          ctx.ellipse(0, 0, rad * breathe, rad * 0.34 * breathe,
            spin * 0.3, 0, Math.PI * 2);
          ctx.stroke();
        }
        // singular point
        ctx.globalAlpha = 1;
        ctx.shadowBlur = R * 0.18;
        ctx.beginPath();
        ctx.arc(0, 0, R * 0.035, 0, Math.PI * 2);
        ctx.fill();
        ctx.restore();
      }

      ctx.globalAlpha = 1; ctx.shadowBlur = 0;
      raf = requestAnimationFrame(frame);
    }
    var raf;
    frame(performance.now());            // immediate first paint + starts loop

    return {
      stop: function () { cancelAnimationFrame(raf); if (ro) ro.disconnect(); },
      canvas: canvas
    };
  }

  function auto(root) {
    (root || document).querySelectorAll('[data-msf-shape]').forEach(function (c) {
      if (!c.__msf) c.__msf = mount(c);
    });
  }

  global.MSFGeometry = { mount: mount, auto: auto };
  if (document.readyState !== 'loading') auto();
  else document.addEventListener('DOMContentLoaded', function () { auto(); });
})(window);
