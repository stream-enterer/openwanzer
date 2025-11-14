#include "constants.h"

// Default configuration values
const int DEFAULT_SCREEN_WIDTH = 1920;
const int DEFAULT_SCREEN_HEIGHT = 1080;
const float DEFAULT_HEX_SIZE = 40.0f;
const int DEFAULT_MAP_ROWS = 12;
const int DEFAULT_MAP_COLS = 16;

// Current settings (can be modified at runtime)
int SCREEN_WIDTH = DEFAULT_SCREEN_WIDTH;
int SCREEN_HEIGHT = DEFAULT_SCREEN_HEIGHT;
float HEX_SIZE = DEFAULT_HEX_SIZE;
int MAP_ROWS = DEFAULT_MAP_ROWS;
int MAP_COLS = DEFAULT_MAP_COLS;

// Color definitions
const Color COLOR_BACKGROUND = BLACK;
const Color COLOR_GRID = Color{245, 245, 220, 255}; // Pale beige
const Color COLOR_FPS = Color{192, 192, 192, 255};   // Light grey

// Movement cost table [movMethod][terrain]
// 254 = Stop move (can enter but stops there), 255 = Don't enter (impassable)
const int MOV_TABLE_DRY[12][18] = {
    // Clear, City, Airfield, Forest, Bocage, Hill, Mountain, Sand, Swamp, Ocean, River, Fort, Port, Stream, Escarp, ImpassRiver, Rough, Road
    {1, 1, 1, 2, 4, 2, 254, 1, 4, 255, 254, 1, 1, 2, 255, 255, 2, 1}, // Tracked
    {1, 1, 1, 2, 254, 2, 254, 1, 4, 255, 254, 1, 1, 2, 255, 255, 2, 1}, // Half Tracked
    {2, 1, 1, 4, 254, 3, 254, 3, 254, 255, 254, 2, 1, 4, 255, 255, 2, 1}, // Wheeled
    {1, 1, 1, 2, 2, 2, 254, 2, 2, 255, 254, 1, 1, 1, 255, 255, 2, 1}, // Leg
    {1, 1, 1, 1, 1, 1, 254, 1, 255, 255, 254, 1, 1, 254, 255, 255, 1, 1}, // Towed
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // Air
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 1, 255, 255, 1, 255, 255, 255, 255, 255}, // Deep Naval
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 2, 1, 255, 1, 255, 255, 255, 255, 255}, // Costal
    {1, 1, 1, 2, 3, 3, 254, 2, 254, 255, 254, 1, 1, 1, 255, 255, 3, 1}, // All Terrain Tracked
    {1, 1, 1, 2, 4, 2, 254, 1, 3, 254, 3, 1, 1, 2, 255, 255, 2, 1}, // Amphibious
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 1, 255, 255, 1, 255, 255, 255, 255, 255}, // Naval
    {1, 1, 1, 1, 2, 1, 1, 2, 2, 255, 254, 1, 1, 1, 255, 255, 1, 1} // All Terrain Leg (Mountain)
};
