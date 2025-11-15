#ifndef OPENWANZER_CONSTANTS_HPP
#define OPENWANZER_CONSTANTS_HPP

#include "rl/raylib.h"

// Default configuration values
extern const int kDefaultScreenWidth;
extern const int kDefaultScreenHeight;
extern const float kDefaultHexSize;
extern const int kDefaultMapRows;
extern const int kDefaultMapCols;

// Current settings (can be modified at runtime)
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern float HEX_SIZE;
extern int MAP_ROWS;
extern int MAP_COLS;

// Color definitions
extern const Color kColorBackground;
extern const Color kColorGrid;
extern const Color kColorFps;

// Movement cost table [movMethod][terrain]
// 254 = Stop move (can enter but stops there), 255 = Don't enter (impassable)
extern const int kMovTableDry[12][18];

#endif // OPENWANZER_CONSTANTS_HPP
