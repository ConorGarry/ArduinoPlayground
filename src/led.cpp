#include "./led.h"

using namespace std;

CRGB leds1[NUM_LEDS_PER_SEGMENT]; // 1–5
CRGB leds2[NUM_LEDS_PER_SEGMENT]; // 6–10
CRGB leds3[NUM_LEDS_PER_SEGMENT]; // 11–15
CRGB leds4[NUM_LEDS_PER_SEGMENT]; // 16–20
CRGB leds5[NUM_LEDS_PER_SEGMENT]; // 21–25
CRGB leds6[NUM_LEDS_PER_SEGMENT]; // 26–30

struct CRGB *ledSegments[] = {
    leds1,                      // 1-5
    leds2,                      // 6-10
    leds3,                      // 11–15
    leds4,                      // 16–20
    leds5,                      // 21–25
    leds6                       // 26–30
};

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

struct CRGB *allVerticals[] = {
    CRGBSet(leds, 0, NUM_LEDS_PER_STRIP),                      // 1
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 4), (NUM_LEDS_PER_STRIP * 5)), // 5
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 5), (NUM_LEDS_PER_STRIP * 6)), // 6
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 1), (NUM_LEDS_PER_STRIP * 2)), // 2
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 3), (NUM_LEDS_PER_STRIP * 4)), // 4
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 17), (NUM_LEDS_PER_STRIP * 18)), // 18
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 19), (NUM_LEDS_PER_STRIP * 20)), // 20
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 6), (NUM_LEDS_PER_STRIP * 7)), // 7
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 8), (NUM_LEDS_PER_STRIP * 9)), // 9
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 11), (NUM_LEDS_PER_STRIP * 12)), // 12
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 14), (NUM_LEDS_PER_STRIP * 15)), // 15
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 16), (NUM_LEDS_PER_STRIP * 17)), // 17 
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 22), (NUM_LEDS_PER_STRIP * 23)), // 23
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 20), NUM_LEDS_PER_STRIP * 21), // 21
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 9), (NUM_LEDS_PER_STRIP * 10)), // 10
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 12), (NUM_LEDS_PER_STRIP * 13)), // 13
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 13), (NUM_LEDS_PER_STRIP * 14)), // 14
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 23), (NUM_LEDS_PER_STRIP * 24)), // 24
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 24), (NUM_LEDS_PER_STRIP * 25)), // 25
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 28), (NUM_LEDS_PER_STRIP * 29)), // 29
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 29), (NUM_LEDS_PER_STRIP * 30)), // 30
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 25), (NUM_LEDS_PER_STRIP * 26)), // 26
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 26), (NUM_LEDS_PER_STRIP * 27)), // 27
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 27), (NUM_LEDS_PER_STRIP * 28)), // 28
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

#pragma strip Segments

struct CRGB *allStrips[] = {
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 0), (NUM_LEDS_PER_STRIP * 1)), // Strip 1  - 0 - 71
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 1), (NUM_LEDS_PER_STRIP * 2)), // Strip 2  - 71 - 142
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 2), (NUM_LEDS_PER_STRIP * 3)), // Strip 3  - 142 - 213
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 3), (NUM_LEDS_PER_STRIP * 4)), // Strip 4  - 213 - 284
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 4), (NUM_LEDS_PER_STRIP * 5)), // Strip 5  - 284 - 355
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 5), (NUM_LEDS_PER_STRIP * 6)), // Strip 6  - 355 - 426
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 6), (NUM_LEDS_PER_STRIP * 7)), // Strip 7  - 426 - 497
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 7), (NUM_LEDS_PER_STRIP * 8)), // Strip 8  - 497 - 568
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 8), (NUM_LEDS_PER_STRIP * 9)), // Strip 9  - 568 - 639
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 9), (NUM_LEDS_PER_STRIP * 10)), // Strip 10  - 639 - 710
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 10), (NUM_LEDS_PER_STRIP * 11)), // Strip 11  - 710 - 781
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 11), (NUM_LEDS_PER_STRIP * 12)), // Strip 12  - 781 - 852
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 12), (NUM_LEDS_PER_STRIP * 13)), // Strip 13  - 852 - 923
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 13), (NUM_LEDS_PER_STRIP * 14)), // Strip 14  - 923 - 994
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 14), (NUM_LEDS_PER_STRIP * 15)), // Strip 15  - 994 - 1065
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 15), (NUM_LEDS_PER_STRIP * 16)), // Strip 16  - 1065 - 1136
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 16), (NUM_LEDS_PER_STRIP * 17)), // Strip 17  - 1136 - 1207
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 17), (NUM_LEDS_PER_STRIP * 18)), // Strip 18  - 1207 - 1278
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 18), (NUM_LEDS_PER_STRIP * 19)), // Strip 19  - 1278 - 1349
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 19), (NUM_LEDS_PER_STRIP * 20)), // Strip 20  - 1349 - 1420
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 20), (NUM_LEDS_PER_STRIP * 21)), // Strip 21  - 1420 - 1491
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 21), (NUM_LEDS_PER_STRIP * 22)), // Strip 22  - 1491 - 1562
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 22), (NUM_LEDS_PER_STRIP * 23)), // Strip 23  - 1562 - 1633
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 23), (NUM_LEDS_PER_STRIP * 24)), // Strip 24  - 1633 - 1704
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 24), (NUM_LEDS_PER_STRIP * 25)), // Strip 25  - 1704 - 1775
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 25), (NUM_LEDS_PER_STRIP * 26)), // Strip 26  - 1775 - 1846
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 26), (NUM_LEDS_PER_STRIP * 27)), // Strip 27  - 1846 - 1917
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 27), (NUM_LEDS_PER_STRIP * 28)), // Strip 28  - 1917 - 1988
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 28), (NUM_LEDS_PER_STRIP * 29)), // Strip 29  - 1988 - 2059
    CRGBSet(leds, (NUM_LEDS_PER_STRIP * 29), (NUM_LEDS_PER_STRIP * 30)), // Strip 30  - 2059 - 2130
};

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