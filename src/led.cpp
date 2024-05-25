#include "./led.h"

CRGB leds1[NUM_LEDS_PER_SEGMENT]; // 1–5
CRGB leds2[NUM_LEDS_PER_SEGMENT]; // 6–10
CRGB leds3[NUM_LEDS_PER_SEGMENT]; // 11–15
CRGB leds4[NUM_LEDS_PER_SEGMENT]; // 16–20
CRGB leds5[NUM_LEDS_PER_SEGMENT]; // 21–25
CRGB leds6[NUM_LEDS_PER_SEGMENT]; // 26–30

/*
Divisions of the dodecahedron

     _,--"^"--._     top
   ,'\         /`.   --------
 ,'   \_______/   `.            midTop
|     /       \     | ---------
|    /         \    |           middle
|  _/           \_  |
\'' `-.       ,-' ``/ ----------
 \     `-._,-'     /            midBottom
  \       |       /  
   `-.._  |  _,,-'   -----------
        ``"''        bottom
*/


struct CRGB *bottom[] = {
    CRGBSet(leds1, 0, NUM_LEDS_PER_STRIP - 1),                      // 1
    CRGBSet(leds1, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 5
    CRGBSet(leds2, 0, NUM_LEDS_PER_STRIP)                           // 6
};

struct CRGB *midBottom[] = {
    CRGBSet(leds1, NUM_LEDS_PER_STRIP, (NUM_LEDS_PER_STRIP * 2)), // 2
    CRGBSet(leds1, (NUM_LEDS_PER_STRIP * 3), (NUM_LEDS_PER_STRIP * 4)), // 4
    CRGBSet(leds4, (NUM_LEDS_PER_STRIP * 2), (NUM_LEDS_PER_STRIP * 3)), // 18
    CRGBSet(leds4, (NUM_LEDS_PER_STRIP * 4), (NUM_LEDS_PER_STRIP * 5)), // 20
    CRGBSet(leds2, (NUM_LEDS_PER_STRIP * 1), (NUM_LEDS_PER_STRIP * 2)), // 7
    CRGBSet(leds2, (NUM_LEDS_PER_STRIP * 3), (NUM_LEDS_PER_STRIP * 3)), // 9
};

struct CRGB *middle[] = {
    CRGBSet(leds3, (NUM_LEDS_PER_STRIP * 1), (NUM_LEDS_PER_STRIP * 2)), // 12
    CRGBSet(leds3, (NUM_LEDS_PER_STRIP * 4), (NUM_LEDS_PER_STRIP * 5)), // 15
    CRGBSet(leds4, (NUM_LEDS_PER_STRIP * 1), (NUM_LEDS_PER_STRIP * 0)), // 17 
    CRGBSet(leds5, (NUM_LEDS_PER_STRIP * 2), (NUM_LEDS_PER_STRIP * 3)), // 23
    CRGBSet(leds5, 0, NUM_LEDS_PER_STRIP), // 21
    CRGBSet(leds2, (NUM_LEDS_PER_STRIP * 4), (NUM_LEDS_PER_STRIP * 5)), // 10
};

struct CRGB *midTop[] = {
    CRGBSet(leds3, (NUM_LEDS_PER_STRIP * 2), (NUM_LEDS_PER_STRIP * 3)), // 13
    CRGBSet(leds3, (NUM_LEDS_PER_STRIP * 3), (NUM_LEDS_PER_STRIP * 4)), // 14
    CRGBSet(leds5, (NUM_LEDS_PER_STRIP * 3), (NUM_LEDS_PER_STRIP * 4)), // 24
    CRGBSet(leds5, (NUM_LEDS_PER_STRIP * 4), (NUM_LEDS_PER_STRIP * 5)), // 25
    CRGBSet(leds6, (NUM_LEDS_PER_STRIP * 3), (NUM_LEDS_PER_STRIP * 4)), // 29
    CRGBSet(leds6, (NUM_LEDS_PER_STRIP * 4), (NUM_LEDS_PER_STRIP * 5)), // 30
};

struct CRGB *top[] = {
    CRGBSet(leds6, 0, NUM_LEDS_PER_STRIP), // 26
    CRGBSet(leds6, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP * 2), // 27
    CRGBSet(leds6, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 28
};



