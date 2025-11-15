#include "Constants.hpp"

// Default configuration values
const int kDefaultScreenWidth = 1920;
const int kDefaultScreenHeight = 1080;
const float kDefaultHexSize = 40.0f;
const int kDefaultMapRows = 12;
const int kDefaultMapCols = 16;

// Current settings (can be modified at runtime)
int SCREEN_WIDTH = kDefaultScreenWidth;
int SCREEN_HEIGHT = kDefaultScreenHeight;
float HEX_SIZE = kDefaultHexSize;
int MAP_ROWS = kDefaultMapRows;
int MAP_COLS = kDefaultMapCols;

// Color definitions
const Color kColorBackground = BLACK;
const Color kColorGrid = Color {245, 245, 220, 255}; // Pale beige
const Color kColorFps = Color {192, 192, 192, 255};  // Light grey

// Movement cost table [movMethod][terrain]
// 254 = Stop move (can enter but stops there), 255 = Don't enter (impassable)
const int kMovTableDry[12][18] = {
    // Clear, City, Airfield, Forest, Bocage, Hill, Mountain, Sand, Swamp, Ocean, River, Fort, Port, Stream, Escarp, ImpassRiver, Rough, Road
    {1, 1, 1, 2, 4, 2, 254, 1, 4, 255, 254, 1, 1, 2, 255, 255, 2, 1},                       // Tracked
    {1, 1, 1, 2, 254, 2, 254, 1, 4, 255, 254, 1, 1, 2, 255, 255, 2, 1},                     // Half Tracked
    {2, 1, 1, 4, 254, 3, 254, 3, 254, 255, 254, 2, 1, 4, 255, 255, 2, 1},                   // Wheeled
    {1, 1, 1, 2, 2, 2, 254, 2, 2, 255, 254, 1, 1, 1, 255, 255, 2, 1},                       // Leg
    {1, 1, 1, 1, 1, 1, 254, 1, 255, 255, 254, 1, 1, 254, 255, 255, 1, 1},                   // Towed
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},                                 // Air
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 1, 255, 255, 1, 255, 255, 255, 255, 255}, // Deep Naval
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 2, 1, 255, 1, 255, 255, 255, 255, 255},   // Costal
    {1, 1, 1, 2, 3, 3, 254, 2, 254, 255, 254, 1, 1, 1, 255, 255, 3, 1},                     // All Terrain Tracked
    {1, 1, 1, 2, 4, 2, 254, 1, 3, 254, 3, 1, 1, 2, 255, 255, 2, 1},                         // Amphibious
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 1, 255, 255, 1, 255, 255, 255, 255, 255}, // Naval
    {1, 1, 1, 1, 2, 1, 1, 2, 2, 255, 254, 1, 1, 1, 255, 255, 1, 1}                          // All Terrain Leg (Mountain)
};
