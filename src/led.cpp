#include "./led.h"

using namespace std;

CRGB leds1[NUM_LEDS_PER_SEGMENT]; // 1–5
CRGB leds2[NUM_LEDS_PER_SEGMENT]; // 6–10
CRGB leds3[NUM_LEDS_PER_SEGMENT]; // 11–15
CRGB leds4[NUM_LEDS_PER_SEGMENT]; // 16–20
CRGB leds5[NUM_LEDS_PER_SEGMENT]; // 21–25
CRGB leds6[NUM_LEDS_PER_SEGMENT]; // 26–30

#pragma region Vertical Segments
/*


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
    CRGBSet(leds, 0, NUM_LEDS_PER_STRIP - 1),                      // 1
    CRGBSet(leds, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 5
    CRGBSet(leds, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP * 3) // 6
};

struct CRGB *midBottom[] = {
    CRGBSet(leds, NUM_LEDS_PER_STRIP, (NUM_LEDS_PER_STRIP * 2)), // 2
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 3), (NUM_LEDS_PER_STRIP * 4)), // 4
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 2), (NUM_LEDS_PER_STRIP * 3)), // 18
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 4), (NUM_LEDS_PER_STRIP * 5)), // 20
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 1), (NUM_LEDS_PER_STRIP * 2)), // 7
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 3), (NUM_LEDS_PER_STRIP * 3)), // 9
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
#pragma endregion

#pragma region Pentagon Segments
////////////////////////
// Pentagon divisions //
////////////////////////

struct CRGB *one[] = {
    CRGBSet(leds, 0, NUM_LEDS), // 1
    CRGBSet(leds, NUM_LEDS * 1, NUM_LEDS * 2), // 2
    CRGBSet(leds, NUM_LEDS * 2, NUM_LEDS * 3), // 3
    CRGBSet(leds, NUM_LEDS * 3, NUM_LEDS * 4), // 4
    CRGBSet(leds, NUM_LEDS * 4, NUM_LEDS * 5), // 5
};

struct CRGB *two[] = {
    CRGBSet(leds1, 0, NUM_LEDS_PER_STRIP), // 1
    CRGBSet(leds2, 0, NUM_LEDS_PER_STRIP), // 6
    CRGBSet(leds2, NUM_LEDS_PER_STRIP * 1, NUM_LEDS_PER_STRIP * 2), // 7
    CRGBSet(leds2, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 8
    CRGBSet(leds2, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP * 4), // 9
};

// 2 ,9, 10, 11, 12
struct CRGB *three[] = {
    CRGBSet(leds1, NUM_LEDS_PER_STRIP * 1, NUM_LEDS_PER_STRIP * 2), // 2
    CRGBSet(leds2, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP * 4), // 9
    CRGBSet(leds2, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 10
    CRGBSet(leds3, NUM_LEDS_PER_STRIP * 0, NUM_LEDS_PER_STRIP * 1), // 11
    CRGBSet(leds3, NUM_LEDS_PER_STRIP * 1, NUM_LEDS_PER_STRIP * 2), // 12
};

// 12, 13, 14, 15, 3,
struct CRGB *four[] = {
    CRGBSet(leds3, NUM_LEDS_PER_STRIP * 1, NUM_LEDS_PER_STRIP * 2), // 12
    CRGBSet(leds3, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 13
    CRGBSet(leds3, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP * 4), // 14
    CRGBSet(leds3, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 15
    CRGBSet(leds1, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 3
};

// 15, 16, 17, 18, 4
struct CRGB *five[] = {
    CRGBSet(leds3, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 15
    CRGBSet(leds4, NUM_LEDS_PER_STRIP * 0, NUM_LEDS_PER_STRIP * 1), // 16
    CRGBSet(leds4, NUM_LEDS_PER_STRIP * 1, NUM_LEDS_PER_STRIP * 2), // 17
    CRGBSet(leds4, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 18
    CRGBSet(leds1, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP * 4), // 4
};

// 18, 19, 20, 6, 5
struct CRGB *six[] = {
    CRGBSet(leds4, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 18
    CRGBSet(leds4, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP * 4), // 19
    CRGBSet(leds4, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 20
    CRGBSet(leds2, NUM_LEDS_PER_STRIP * 0, NUM_LEDS_PER_STRIP * 1), // 6
    CRGBSet(leds1, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 5
};

// 21, 22, 23, 8, 10
struct CRGB *seven[] = {
    CRGBSet(leds5, NUM_LEDS_PER_STRIP * 0, NUM_LEDS_PER_STRIP * 1), // 21
    CRGBSet(leds5, NUM_LEDS_PER_STRIP * 1, NUM_LEDS_PER_STRIP * 2), // 22
    CRGBSet(leds5, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 23
    CRGBSet(leds2, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 8
    CRGBSet(leds2, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 10
};

// 23, 24, 25, 11, 9
struct CRGB *eight[] = {
    CRGBSet(leds5, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 23
    CRGBSet(leds5, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP * 4), // 24
    CRGBSet(leds5, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 25
    CRGBSet(leds2, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 10
    CRGBSet(leds2, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 8
};

// 25, 26, 27, 11, 13
struct CRGB *nine[] = {
    CRGBSet(leds5, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 25
    CRGBSet(leds6, NUM_LEDS_PER_STRIP * 0, NUM_LEDS_PER_STRIP * 1), // 26
    CRGBSet(leds6, NUM_LEDS_PER_STRIP * 1, NUM_LEDS_PER_STRIP * 2), // 27
    CRGBSet(leds3, NUM_LEDS_PER_STRIP * 0, NUM_LEDS_PER_STRIP * 1), // 11
    CRGBSet(leds3, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 13
};

// 27, 28, 29, 14, 16
struct CRGB *ten[] = {
    CRGBSet(leds6, NUM_LEDS_PER_STRIP * 1, NUM_LEDS_PER_STRIP * 2), // 27
    CRGBSet(leds6, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 28
    CRGBSet(leds6, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP * 4), // 29
    CRGBSet(leds3, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP * 4), // 14
    CRGBSet(leds4, NUM_LEDS_PER_STRIP * 0, NUM_LEDS_PER_STRIP * 1), // 16
};
// 29, 30, 17, 19, 21
struct CRGB *eleven[] = {
    CRGBSet(leds6, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP * 4), // 29
    CRGBSet(leds6, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 30
    CRGBSet(leds4, NUM_LEDS_PER_STRIP * 1, NUM_LEDS_PER_STRIP * 2), // 17
    CRGBSet(leds4, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP * 4), // 19
    CRGBSet(leds5, NUM_LEDS_PER_STRIP * 0, NUM_LEDS_PER_STRIP * 1), // 21
};

// 22, 24, 26, 28, 30
struct CRGB *twelve[] = {
    CRGBSet(leds5, NUM_LEDS_PER_STRIP * 1, NUM_LEDS_PER_STRIP * 2), // 22
    CRGBSet(leds5, NUM_LEDS_PER_STRIP * 3, NUM_LEDS_PER_STRIP * 4), // 24
    CRGBSet(leds6, NUM_LEDS_PER_STRIP * 0, NUM_LEDS_PER_STRIP * 1), // 26
    CRGBSet(leds6, NUM_LEDS_PER_STRIP * 2, NUM_LEDS_PER_STRIP * 3), // 28
    CRGBSet(leds6, NUM_LEDS_PER_STRIP * 4, NUM_LEDS_PER_STRIP * 5), // 30
};

// array of pentagon divisions
struct CRGB **pentagons[] = { one, two, three, four, five, six, seven, eight, nine, ten, eleven, twelve };

#pragma endregion

#pragma region Util Functions

////////////////////////
//   Uitil Functions  //
////////////////////////

/**
 * Fill a single run of LEDs with a color.
 */
void fillRun(CRGB *runs, CRGB color) {
  for (int l = 0; l < NUM_LEDS_PER_STRIP; l++) {
    runs[l] = color;
  }
}

/**
 * Fill a pentagon with a color.
 */
void fillPentagon(CRGB **pentagon, CRGB color) {
  for (int run = 0; run < NUM_RUNS_PER_PENTAGON; ++run) {
    fillRun(pentagon[run], color);
  }
}

#pragma endregion