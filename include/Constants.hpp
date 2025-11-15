#ifndef OPENWANZER_CONSTANTS_HPP
#define OPENWANZER_CONSTANTS_HPP

#include "raylib.h"

// Default configuration values
extern const int DEFAULT_SCREEN_WIDTH;
extern const int DEFAULT_SCREEN_HEIGHT;
extern const float DEFAULT_HEX_SIZE;
extern const int DEFAULT_MAP_ROWS;
extern const int DEFAULT_MAP_COLS;

// Current settings (can be modified at runtime)
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern float HEX_SIZE;
extern int MAP_ROWS;
extern int MAP_COLS;

// Color definitions
extern const Color COLOR_BACKGROUND;
extern const Color COLOR_GRID;
extern const Color COLOR_FPS;

// Movement cost table [movMethod][terrain]
// 254 = Stop move (can enter but stops there), 255 = Don't enter (impassable)
extern const int MOV_TABLE_DRY[12][18];

#endif // OPENWANZER_CONSTANTS_HPP
