#ifndef STRIP_CALIBRATION_H
#define STRIP_CALIBRATION_H

#include "../led.h"   // NUM_STRIPS

// Single source of truth for the physical strip-wiring calibration used by
// every base-climbing / falling pattern in app_patterns.cpp (fireFull,
// paintDali, paintDaliCascade). Re-calibrate the rig in ONE place here — the
// fire ritual and the Dalí drips read the same tables, so they always climb
// and fall in the same direction. See CLAUDE/fire-ritual-base-climb-design.md.
//
// Both tables are 1-based: index [0] is unused (strips are numbered 1..30).

namespace strip_calibration {

// Per-strip data-flow direction.
//   true  = data input is at the physical TOP of the run; "climb from base"
//           fills from high LED-index toward LED 0 (visually bottom → top).
//   false = data input is at the BASE of the run; fills LED 0 → top.
//
// Default for all 30 strips: true (inferred from the bench test of the previous
// "all false except strip 1" default). Flip any individual strip to false here
// if it still climbs/falls the wrong way after testing.
// SPENCER - fix these
static constexpr bool STRIP_WIRED_FROM_TOP[NUM_STRIPS + 1] = {
  /* [0] unused */ false,
  /*  1 */ false, /*  2 */ false, /*  3 */ true,  /*  4 */ true,  /*  5 */ true,
  /*  6 */ false, /*  7 */ true,  /*  8 */ true,  /*  9 */ true,  /* 10 */ true,
  /* 11 */ true,  /* 12 */ true,  /* 13 */ true,  /* 14 */ true,  /* 15 */ true,
  /* 16 */ true,  /* 17 */ true,  /* 18 */ true,  /* 19 */ true,  /* 20 */ true,
  /* 21 */ true,  /* 22 */ true,  /* 23 */ true,  /* 24 */ true,  /* 25 */ true,
  /* 26 */ false, /* 27 */ true,  /* 28 */ true,  /* 29 */ true,  /* 30 */ true,
};

// 1-based strip number -> height-band rank (0 = base ... 4 = top). The 6
// non-vertical strips (3, 8, 11, 16, 19, 22) have no documented band; they
// default to middle (2) so the whole shape ends up ablaze.
static constexpr uint8_t STRIP_BAND[NUM_STRIPS + 1] = {
  /* [0] unused */ 0,
  /*  1.. 6 */ 0, 1, 2, 1, 0, 0,
  /*  7..12 */ 1, 2, 1, 2, 2, 2,
  /* 13..18 */ 3, 3, 2, 2, 2, 1,
  /* 19..24 */ 2, 1, 2, 2, 2, 3,
  /* 25..30 */ 3, 4, 4, 4, 3, 3,
};

}  // namespace strip_calibration

#endif
