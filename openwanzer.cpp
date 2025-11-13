#include "raylib.h"
#include "raymath.h"

// Suppress warnings from raygui.h (external library)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wstringop-overflow"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#pragma GCC diagnostic pop

#include "hex.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>

// Constants
const int DEFAULT_SCREEN_WIDTH = 1920;
const int DEFAULT_SCREEN_HEIGHT = 1080;
const float DEFAULT_HEX_SIZE = 40.0f;
const int DEFAULT_MAP_ROWS = 12;
const int DEFAULT_MAP_COLS = 16;

// Current settings (can be modified)
int SCREEN_WIDTH = DEFAULT_SCREEN_WIDTH;
int SCREEN_HEIGHT = DEFAULT_SCREEN_HEIGHT;
float HEX_SIZE = DEFAULT_HEX_SIZE;
int MAP_ROWS = DEFAULT_MAP_ROWS;
int MAP_COLS = DEFAULT_MAP_COLS;

// Color definitions
const Color COLOR_BACKGROUND = BLACK;
const Color COLOR_GRID = Color{245, 245, 220, 255}; // Pale beige
const Color COLOR_FPS = Color{192, 192, 192, 255};   // Light grey

// Enums
enum class TerrainType {
  PLAINS,      // Open grassland
  FOREST,      // Woods/Trees
  MOUNTAIN,    // High elevation
  HILL,        // Low elevation
  DESERT,      // Sandy/arid
  SWAMP,       // Marsh/wetland
  CITY,        // Urban
  WATER,       // River/lake
  ROAD,        // Paved road
  ROUGH        // Rocky/broken terrain
};

enum class UnitClass {
  INFANTRY,
  TANK,
  ARTILLERY,
  RECON,
  ANTI_TANK,
  AIR_DEFENSE
};

enum class Side { AXIS = 0, ALLIED = 1 };

// Movement methods (12 types)
enum class MovMethod {
  TRACKED = 0,
  HALF_TRACKED = 1,
  WHEELED = 2,
  LEG = 3,
  TOWED = 4,
  AIR = 5,
  DEEP_NAVAL = 6,
  COSTAL = 7,
  ALL_TERRAIN_TRACKED = 8,
  AMPHIBIOUS = 9,
  NAVAL = 10,
  ALL_TERRAIN_LEG = 11
};

// Terrain type indices for movement tables
enum TerrainIndex {
  TI_CLEAR = 0,
  TI_CITY = 1,
  TI_AIRFIELD = 2,
  TI_FOREST = 3,
  TI_BOCAGE = 4,
  TI_HILL = 5,
  TI_MOUNTAIN = 6,
  TI_SAND = 7,
  TI_SWAMP = 8,
  TI_OCEAN = 9,
  TI_RIVER = 10,
  TI_FORTIFICATION = 11,
  TI_PORT = 12,
  TI_STREAM = 13,
  TI_ESCARPMENT = 14,
  TI_IMPASSABLE_RIVER = 15,
  TI_ROUGH = 16,
  TI_ROAD = 17
};

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

// Helper function to map TerrainType to movement table index
int getTerrainIndex(TerrainType terrain) {
  switch (terrain) {
  case TerrainType::PLAINS:
    return TI_CLEAR;
  case TerrainType::CITY:
    return TI_CITY;
  case TerrainType::FOREST:
    return TI_FOREST;
  case TerrainType::HILL:
    return TI_HILL;
  case TerrainType::MOUNTAIN:
    return TI_MOUNTAIN;
  case TerrainType::DESERT:
    return TI_SAND;
  case TerrainType::SWAMP:
    return TI_SWAMP;
  case TerrainType::WATER:
    return TI_OCEAN;
  case TerrainType::ROAD:
    return TI_ROAD;
  case TerrainType::ROUGH:
    return TI_ROUGH;
  default:
    return TI_CLEAR;
  }
}

// Forward declaration for calculateCenteredCameraOffset
struct CameraState;
void calculateCenteredCameraOffset(CameraState& camera, int screenWidth, int screenHeight);

// Camera state structure
struct CameraState {
  float offsetX;
  float offsetY;
  float zoom;           // 0.5 to 2.0 (50% to 200%)
  int zoomDirection;    // -1 for zooming out, 1 for zooming in, 0 for neutral
  bool isPanning;
  Vector2 panStartMouse;
  Vector2 panStartOffset;

  CameraState()
      : offsetX(0.0f), offsetY(0.0f), zoom(1.0f), zoomDirection(0),
        isPanning(false), panStartMouse{0, 0}, panStartOffset{0, 0} {
    // Initialize to centered position (will be properly calculated after layout is set)
    offsetX = 100.0f;
    offsetY = 100.0f;
  }
};

// Settings Structure
struct VideoSettings {
  int resolutionIndex;
  bool fullscreen;
  bool vsync;
  int fpsIndex;
  float hexSize;
  float panSpeed;
  bool msaa;
  int guiScaleIndex;
  std::string styleTheme;
  bool resolutionDropdownEdit;
  bool fpsDropdownEdit;
  bool guiScaleDropdownEdit;
  bool styleThemeDropdownEdit;

  VideoSettings()
      : resolutionIndex(6), fullscreen(true), vsync(false), fpsIndex(6),
        hexSize(40.0f), panSpeed(5.0f), msaa(false), guiScaleIndex(0),
        styleTheme("dark"), resolutionDropdownEdit(false), fpsDropdownEdit(false),
        guiScaleDropdownEdit(false), styleThemeDropdownEdit(false) {}
};

// Game Layout Structure
struct GameLayout {
  Rectangle statusBar;   // Top status bar
  Rectangle unitPanel;   // Right unit info panel
  Rectangle helpBar;     // Bottom help text area
  Rectangle playArea;    // Map rendering area

  void recalculate(int w, int h) {
    statusBar = {0, 0, (float)w, 40};
    unitPanel = {(float)w - 250, 50, 250, 300};
    helpBar = {0, (float)h - 30, (float)w, 30};
    playArea = {0, 40, (float)w - 250, (float)h - 70};
  }

  GameLayout() { recalculate(SCREEN_WIDTH, SCREEN_HEIGHT); }
};

// Resolution options
struct Resolution {
  int width;
  int height;
  const char *label;
};

const Resolution RESOLUTIONS[] = {
    {800, 600, "800x600"},     {1024, 768, "1024x768"},
    {1280, 720, "1280x720"},   {1280, 800, "1280x800"},
    {1366, 768, "1366x768"},   {1600, 900, "1600x900"},
    {1920, 1080, "1920x1080"}, {2560, 1440, "2560x1440"},
    {3840, 2160, "3840x2160"}};
const int RESOLUTION_COUNT = 9;

const int FPS_VALUES[] = {30, 60, 75, 120, 144, 240, 0};
const char *FPS_LABELS = "30;60;75;120;144;240;Unlimited";

const float GUI_SCALE_VALUES[] = {1.0f, 1.5f, 2.0f};
const char *GUI_SCALE_LABELS = "1.00;1.50;2.00";
const int GUI_SCALE_COUNT = 3;

// Global variables for style themes
std::vector<std::string> AVAILABLE_STYLES;
std::string STYLE_LABELS_STRING;

// Function to discover available styles
void discoverStyles() {
  AVAILABLE_STYLES.clear();
  const char* stylesPath = "resources/styles";

  DIR* dir = opendir(stylesPath);
  if (dir == nullptr) {
    TraceLog(LOG_WARNING, "Failed to open styles directory");
    // Add default style as fallback
    AVAILABLE_STYLES.push_back("default");
    STYLE_LABELS_STRING = "default";
    return;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    // Skip . and ..
    if (entry->d_name[0] == '.') continue;

    // Check if it's a directory
    std::string fullPath = std::string(stylesPath) + "/" + entry->d_name;
    struct stat statbuf;
    if (stat(fullPath.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
      // Check if .rgs file exists
      std::string rgsPath = fullPath + "/style_" + entry->d_name + ".rgs";
      std::ifstream rgsFile(rgsPath);
      if (rgsFile.good()) {
        AVAILABLE_STYLES.push_back(entry->d_name);
      }
    }
  }
  closedir(dir);

  // Sort styles alphabetically
  std::sort(AVAILABLE_STYLES.begin(), AVAILABLE_STYLES.end());

  // Create labels string
  STYLE_LABELS_STRING.clear();
  for (size_t i = 0; i < AVAILABLE_STYLES.size(); i++) {
    if (i > 0) STYLE_LABELS_STRING += ";";
    STYLE_LABELS_STRING += AVAILABLE_STYLES[i];
  }

  TraceLog(LOG_INFO, TextFormat("Found %d styles", (int)AVAILABLE_STYLES.size()));
}

// Get index of a style by name
int getStyleIndex(const std::string& styleName) {
  for (size_t i = 0; i < AVAILABLE_STYLES.size(); i++) {
    if (AVAILABLE_STYLES[i] == styleName) {
      return (int)i;
    }
  }
  return 0; // Default to first style if not found
}

// Structures
struct HexCoord {
  int row;
  int col;

  bool operator==(const HexCoord &other) const {
    return row == other.row && col == other.col;
  }
};

struct GameHex {
  HexCoord coord;
  TerrainType terrain;
  int owner; // -1 = neutral, 0 = axis, 1 = allied
  bool isVictoryHex;
  bool isDeployment;
  bool isSpotted[2]; // spotted by each side
  int zoc[2];        // zone of control counter per side
  bool isMoveSel;    // highlighted for movement
  bool isAttackSel;  // highlighted for attack

  GameHex()
      : terrain(TerrainType::PLAINS), owner(-1), isVictoryHex(false),
        isDeployment(false), isMoveSel(false), isAttackSel(false) {
    isSpotted[0] = false;
    isSpotted[1] = false;
    zoc[0] = 0;
    zoc[1] = 0;
  }

  void setZOC(int side, bool on) {
    if (on) {
      zoc[side]++;
    } else if (zoc[side] > 0) {
      zoc[side]--;
    }
  }

  bool isZOC(int side) const { return zoc[side] > 0; }
};

struct Unit {
  std::string name;
  UnitClass unitClass;
  int side;     // 0 = axis, 1 = allied
  int strength; // 1-10
  int maxStrength;
  int experience;   // 0-5 bars
  int entrenchment; // 0-5
  HexCoord position;

  // Combat stats
  int hardAttack;
  int softAttack;
  int groundDefense;
  int closeDefense;
  int initiative;

  // Movement & logistics
  MovMethod movMethod;
  int movementPoints;
  int movesLeft;
  int fuel;
  int ammo;
  int spotRange;

  bool hasMoved;
  bool hasFired;
  bool isCore; // campaign unit

  Unit()
      : strength(10), maxStrength(10), experience(0), entrenchment(0),
        hardAttack(8), softAttack(10), groundDefense(6), closeDefense(5),
        initiative(5), movMethod(MovMethod::TRACKED), movementPoints(6),
        movesLeft(6), fuel(50), ammo(20), spotRange(2), hasMoved(false),
        hasFired(false), isCore(false) {}
};

// Game State
struct GameState {
  std::vector<std::vector<GameHex>> map;
  std::vector<std::unique_ptr<Unit>> units;
  Unit *selectedUnit;
  int currentTurn;
  int currentPlayer; // 0 or 1
  int maxTurns;
  bool showOptionsMenu;
  VideoSettings settings;
  GameLayout layout;
  CameraState camera;

  GameState()
      : selectedUnit(nullptr), currentTurn(1), currentPlayer(0), maxTurns(20),
        showOptionsMenu(false) {
    initializeMap();
  }

  void initializeMap() {
    map.resize(MAP_ROWS);
    for (int row = 0; row < MAP_ROWS; row++) {
      map[row].resize(MAP_COLS);
      for (int col = 0; col < MAP_COLS; col++) {
        map[row][col].coord = {row, col};

        // Wargame terrain generation with realistic distribution
        int randVal = GetRandomValue(0, 100);
        if (randVal < 35)
          map[row][col].terrain = TerrainType::PLAINS;      // 35% plains (most common)
        else if (randVal < 55)
          map[row][col].terrain = TerrainType::FOREST;      // 20% forest
        else if (randVal < 68)
          map[row][col].terrain = TerrainType::HILL;        // 13% hills
        else if (randVal < 75)
          map[row][col].terrain = TerrainType::ROUGH;       // 7% rough
        else if (randVal < 82)
          map[row][col].terrain = TerrainType::DESERT;      // 7% desert
        else if (randVal < 87)
          map[row][col].terrain = TerrainType::MOUNTAIN;    // 5% mountain
        else if (randVal < 91)
          map[row][col].terrain = TerrainType::SWAMP;       // 4% swamp
        else if (randVal < 95)
          map[row][col].terrain = TerrainType::CITY;        // 4% city
        else
          map[row][col].terrain = TerrainType::WATER;       // 5% water

        // Set some victory hexes
        if (row == 5 && (col == 4 || col == 12)) {
          map[row][col].isVictoryHex = true;
          map[row][col].owner = 1; // Start owned by Allies
        }
      }
    }
  }

  Unit *getUnitAt(const HexCoord &coord) {
    for (auto &unit : units) {
      if (unit->position == coord) {
        return unit.get();
      }
    }
    return nullptr;
  }

  void addUnit(UnitClass uClass, int side, int row, int col) {
    auto unit = std::make_unique<Unit>();
    unit->unitClass = uClass;
    unit->side = side;
    unit->position = {row, col};

    // Set unit name and movement method based on class
    switch (uClass) {
    case UnitClass::INFANTRY:
      unit->name = "Infantry";
      unit->movMethod = MovMethod::LEG;
      break;
    case UnitClass::TANK:
      unit->name = "Tank";
      unit->movMethod = MovMethod::TRACKED;
      break;
    case UnitClass::ARTILLERY:
      unit->name = "Artillery";
      unit->movMethod = MovMethod::HALF_TRACKED;
      break;
    case UnitClass::RECON:
      unit->name = "Recon";
      unit->movMethod = MovMethod::WHEELED;
      break;
    case UnitClass::ANTI_TANK:
      unit->name = "Anti-Tank";
      unit->movMethod = MovMethod::HALF_TRACKED;
      break;
    case UnitClass::AIR_DEFENSE:
      unit->name = "Air Defense";
      unit->movMethod = MovMethod::HALF_TRACKED;
      break;
    }

    units.push_back(std::move(unit));
  }
};

// Create hex layout for rendering
Layout createHexLayout(float hexSize, float offsetX, float offsetY, float zoom) {
  Point size(hexSize * zoom, hexSize * zoom);
  Point origin(offsetX, offsetY);
  return Layout(layout_pointy, size, origin);
}

// Convert our game's row/col to hex library's offset coordinates
OffsetCoord gameCoordToOffset(const HexCoord &coord) {
  return OffsetCoord(coord.col, coord.row);
}

// Convert hex library's offset coordinates to our game's row/col
HexCoord offsetToGameCoord(const OffsetCoord &offset) {
  return HexCoord{offset.row, offset.col};
}

// Draw a hexagon using raylib
void drawHexagon(const std::vector<Point> &corners, Color color, bool filled) {
  if (filled) {
    // Draw filled hexagon using triangles from center
    Vector2 center = {0, 0};
    for (const auto &corner : corners) {
      center.x += corner.x;
      center.y += corner.y;
    }
    center.x /= corners.size();
    center.y /= corners.size();

    for (size_t i = 0; i < corners.size(); i++) {
      size_t next = (i + 1) % corners.size();
      DrawTriangle(
          Vector2{(float)corners[i].x, (float)corners[i].y},
          Vector2{(float)corners[next].x, (float)corners[next].y},
          center,
          color);
    }
  } else {
    // Draw hexagon outline
    for (size_t i = 0; i < corners.size(); i++) {
      size_t next = (i + 1) % corners.size();
      DrawLineEx(
          Vector2{(float)corners[i].x, (float)corners[i].y},
          Vector2{(float)corners[next].x, (float)corners[next].y},
          2.0f, color);
    }
  }
}

Color getTerrainColor(TerrainType terrain) {
  switch (terrain) {
  case TerrainType::PLAINS:
    return Color{144, 186, 96, 255};   // Light green grass
  case TerrainType::FOREST:
    return Color{34, 102, 34, 255};    // Dark green woods
  case TerrainType::MOUNTAIN:
    return Color{120, 100, 80, 255};   // Gray-brown peaks
  case TerrainType::HILL:
    return Color{160, 140, 100, 255};  // Tan hills
  case TerrainType::DESERT:
    return Color{220, 200, 140, 255};  // Sandy yellow
  case TerrainType::SWAMP:
    return Color{100, 120, 80, 255};   // Murky green-brown
  case TerrainType::CITY:
    return Color{140, 140, 140, 255};  // Gray urban
  case TerrainType::WATER:
    return Color{80, 140, 200, 255};   // Blue water
  case TerrainType::ROAD:
    return Color{100, 100, 100, 255};  // Dark gray pavement
  case TerrainType::ROUGH:
    return Color{130, 110, 90, 255};   // Brown rocky
  default:
    return GRAY;
  }
}

Color getUnitColor(int side) {
  return side == 0 ? Color{200, 0, 0, 255} : Color{0, 0, 200, 255};
}

std::string getUnitSymbol(UnitClass uClass) {
  switch (uClass) {
  case UnitClass::INFANTRY:
    return "INF";
  case UnitClass::TANK:
    return "TNK";
  case UnitClass::ARTILLERY:
    return "ART";
  case UnitClass::RECON:
    return "RCN";
  case UnitClass::ANTI_TANK:
    return "AT";
  case UnitClass::AIR_DEFENSE:
    return "AA";
  default:
    return "???";
  }
}

void drawMap(GameState &game) {
  Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
                                  game.camera.offsetY, game.camera.zoom);

  // Draw hexes
  for (int row = 0; row < MAP_ROWS; row++) {
    for (int col = 0; col < MAP_COLS; col++) {
      GameHex &hex = game.map[row][col];

      // Skip if not spotted by current player (FOG OF WAR)
      if (!hex.isSpotted[game.currentPlayer]) continue;

      OffsetCoord offset = gameCoordToOffset(hex.coord);
      ::Hex cubeHex = offset_to_cube(offset);

      std::vector<Point> corners = polygon_corners(layout, cubeHex);

      // Draw terrain
      Color terrainColor = getTerrainColor(hex.terrain);
      drawHexagon(corners, terrainColor, true);

      // Draw hex outline
      drawHexagon(corners, COLOR_GRID, false);

      // Draw victory hex marker
      if (hex.isVictoryHex) {
        Point center = hex_to_pixel(layout, cubeHex);
        DrawCircle((int)center.x, (int)center.y, 8 * game.camera.zoom, GOLD);
      }

      // Draw movement/attack selection highlights
      if (hex.isMoveSel) {
        std::vector<Point> innerCorners;
        Point center = hex_to_pixel(layout, cubeHex);
        for (int i = 0; i < 6; i++) {
          Point offset = hex_corner_offset(layout, i);
          float scale = 0.85f;
          innerCorners.push_back(Point(center.x + offset.x * scale,
                                      center.y + offset.y * scale));
        }
        drawHexagon(innerCorners, Color{0, 255, 0, 100}, true);
      }
      if (hex.isAttackSel) {
        std::vector<Point> innerCorners;
        Point center = hex_to_pixel(layout, cubeHex);
        for (int i = 0; i < 6; i++) {
          Point offset = hex_corner_offset(layout, i);
          float scale = 0.85f;
          innerCorners.push_back(Point(center.x + offset.x * scale,
                                      center.y + offset.y * scale));
        }
        drawHexagon(innerCorners, Color{255, 0, 0, 100}, true);
      }
    }
  }

  // Draw units (only those on spotted hexes)
  for (auto &unit : game.units) {
    GameHex &unitHex = game.map[unit->position.row][unit->position.col];

    // Only show units on spotted hexes (FOG OF WAR)
    if (!unitHex.isSpotted[game.currentPlayer]) continue;

    OffsetCoord offset = gameCoordToOffset(unit->position);
    ::Hex cubeHex = offset_to_cube(offset);
    Point center = hex_to_pixel(layout, cubeHex);

    float unitWidth = 40 * game.camera.zoom;
    float unitHeight = 30 * game.camera.zoom;

    // Draw unit square
    Color unitColor = getUnitColor(unit->side);
    DrawRectangle((int)(center.x - unitWidth / 2),
                  (int)(center.y - unitHeight / 2),
                  (int)unitWidth, (int)unitHeight, unitColor);
    DrawRectangleLines((int)(center.x - unitWidth / 2),
                       (int)(center.y - unitHeight / 2),
                       (int)unitWidth, (int)unitHeight, BLACK);

    // Draw unit symbol
    std::string symbol = getUnitSymbol(unit->unitClass);
    int fontSize = (int)(10 * game.camera.zoom);
    if (fontSize >= 8) {  // Only draw text if it's readable
      int textWidth = MeasureText(symbol.c_str(), fontSize);
      DrawText(symbol.c_str(),
               (int)(center.x - textWidth / 2),
               (int)(center.y - unitHeight / 2 + 2),
               fontSize, WHITE);

      // Draw strength
      std::string strength = std::to_string(unit->strength);
      fontSize = (int)(12 * game.camera.zoom);
      textWidth = MeasureText(strength.c_str(), fontSize);
      DrawText(strength.c_str(),
               (int)(center.x - textWidth / 2),
               (int)(center.y + 5 * game.camera.zoom),
               fontSize, YELLOW);
    }

    // Draw selection highlight
    if (unit.get() == game.selectedUnit) {
      DrawRectangleLines((int)(center.x - unitWidth / 2 - 2),
                         (int)(center.y - unitHeight / 2 - 2),
                         (int)(unitWidth + 4), (int)(unitHeight + 4), YELLOW);
      DrawRectangleLines((int)(center.x - unitWidth / 2 - 3),
                         (int)(center.y - unitHeight / 2 - 3),
                         (int)(unitWidth + 6), (int)(unitHeight + 6), YELLOW);
    }
  }
}

void clearSelectionHighlights(GameState &game) {
  for (int row = 0; row < MAP_ROWS; row++) {
    for (int col = 0; col < MAP_COLS; col++) {
      game.map[row][col].isMoveSel = false;
      game.map[row][col].isAttackSel = false;
    }
  }
}

// Calculate distance between two hexes using the hex library
int hexDistance(const HexCoord &a, const HexCoord &b) {
  OffsetCoord offsetA = gameCoordToOffset(a);
  OffsetCoord offsetB = gameCoordToOffset(b);
  ::Hex cubeA = offset_to_cube(offsetA);
  ::Hex cubeB = offset_to_cube(offsetB);
  return hex_distance(cubeA, cubeB);
}

// Get adjacent hex coordinates
std::vector<HexCoord> getAdjacent(int row, int col) {
  std::vector<HexCoord> result;
  OffsetCoord center(col, row);
  ::Hex cubeHex = offset_to_cube(center);

  for (int i = 0; i < 6; i++) {
    ::Hex neighbor = hex_neighbor(cubeHex, i);
    OffsetCoord neighborOffset = cube_to_offset(neighbor);

    if (neighborOffset.row >= 0 && neighborOffset.row < MAP_ROWS &&
        neighborOffset.col >= 0 && neighborOffset.col < MAP_COLS) {
      result.push_back({neighborOffset.row, neighborOffset.col});
    }
  }

  return result;
}

// Check if unit is air unit (ignores ZOC)
bool isAir(Unit *unit) {
  return unit && unit->movMethod == MovMethod::AIR;
}

// Set or clear ZOC for a unit
void setUnitZOC(GameState &game, Unit *unit, bool on) {
  if (!unit || isAir(unit)) return;

  std::vector<HexCoord> adjacent = getAdjacent(unit->position.row, unit->position.col);

  for (const auto& adj : adjacent) {
    game.map[adj.row][adj.col].setZOC(unit->side, on);
  }
}

// Initialize ZOC for all units on the map
void initializeAllZOC(GameState &game) {
  // Clear all ZOC first
  for (int row = 0; row < MAP_ROWS; row++) {
    for (int col = 0; col < MAP_COLS; col++) {
      game.map[row][col].zoc[0] = 0;
      game.map[row][col].zoc[1] = 0;
    }
  }

  // Set ZOC for all units
  for (auto &unit : game.units) {
    setUnitZOC(game, unit.get(), true);
  }
}

// Get all cells within a certain range (using hex distance)
std::vector<HexCoord> getCellsInRange(int row, int col, int range) {
  std::vector<HexCoord> result;

  for (int r = 0; r < MAP_ROWS; r++) {
    for (int c = 0; c < MAP_COLS; c++) {
      HexCoord target = {r, c};
      HexCoord center = {row, col};
      int dist = hexDistance(center, target);

      if (dist <= range) {
        result.push_back(target);
      }
    }
  }

  return result;
}

// Set or clear spotting range for a unit
void setUnitSpotRange(GameState &game, Unit *unit, bool on) {
  if (!unit) return;

  HexCoord pos = unit->position;
  int range = unit->spotRange;
  std::vector<HexCoord> cells = getCellsInRange(pos.row, pos.col, range);

  for (const auto& cell : cells) {
    GameHex& hex = game.map[cell.row][cell.col];
    if (on) {
      hex.isSpotted[unit->side] = true;
    } else {
      hex.isSpotted[unit->side] = false;
    }
  }
}

// Initialize spotting for all units on the map
void initializeAllSpotting(GameState &game) {
  // Clear all spotting first
  for (int row = 0; row < MAP_ROWS; row++) {
    for (int col = 0; col < MAP_COLS; col++) {
      game.map[row][col].isSpotted[0] = false;
      game.map[row][col].isSpotted[1] = false;
    }
  }

  // Set spotting for all units
  for (auto &unit : game.units) {
    setUnitSpotRange(game, unit.get(), true);
  }
}

void highlightMovementRange(GameState &game, Unit *unit) {
  clearSelectionHighlights(game);
  if (!unit)
    return;

  int maxRange = unit->movesLeft;
  int movMethodIdx = static_cast<int>(unit->movMethod);
  int enemySide = 1 - unit->side;
  bool ignoreZOC = isAir(unit);

  // Track cells we can reach with their remaining movement
  std::vector<std::pair<HexCoord, int>> frontier;
  std::vector<std::pair<HexCoord, int>> visited;

  // Start with unit's position
  frontier.push_back({unit->position, maxRange});
  visited.push_back({unit->position, maxRange});

  while (!frontier.empty()) {
    auto current = frontier.back();
    frontier.pop_back();

    HexCoord pos = current.first;
    int remainingMoves = current.second;

    // Check all adjacent hexes
    std::vector<HexCoord> adjacent = getAdjacent(pos.row, pos.col);

    for (const auto& adj : adjacent) {
      // Get terrain cost
      GameHex& hex = game.map[adj.row][adj.col];
      int terrainIdx = getTerrainIndex(hex.terrain);
      int cost = MOV_TABLE_DRY[movMethodIdx][terrainIdx];

      // Skip impassable terrain
      if (cost >= 255) continue;

      // Check if we can enter this hex
      int newRemaining = remainingMoves - cost;

      // For cost 254, we can enter but it stops us (remaining becomes 0)
      if (cost == 254) newRemaining = 0;

      // ZOC: Enemy zone of control stops movement
      // Units can enter enemy ZOC but must stop there (unless air)
      if (!ignoreZOC && hex.isZOC(enemySide) && cost < 254) {
        newRemaining = 0;  // Can enter but must stop
      }

      // Skip if we can't afford to enter
      if (newRemaining < 0) continue;

      // Check if another unit occupies this hex
      Unit *occupant = game.getUnitAt(adj);
      if (occupant && occupant->side != unit->side) continue;

      // Check if we've already visited with more movement
      bool shouldUpdate = true;
      bool alreadyVisited = false;
      for (auto& v : visited) {
        if (v.first == adj) {
          alreadyVisited = true;
          if (v.second >= newRemaining) {
            shouldUpdate = false;
          } else {
            v.second = newRemaining;
          }
          break;
        }
      }

      if (!alreadyVisited) {
        visited.push_back({adj, newRemaining});
      }

      if (shouldUpdate && newRemaining > 0) {
        frontier.push_back({adj, newRemaining});
      }
    }
  }

  // Highlight all reachable cells (except starting position)
  for (const auto& v : visited) {
    if (!(v.first == unit->position)) {
      game.map[v.first.row][v.first.col].isMoveSel = true;
    }
  }
}

void highlightAttackRange(GameState &game, Unit *unit) {
  if (!unit || unit->hasFired)
    return;

  int range = 1; // Default attack range
  if (unit->unitClass == UnitClass::ARTILLERY)
    range = 3;

  for (int row = 0; row < MAP_ROWS; row++) {
    for (int col = 0; col < MAP_COLS; col++) {
      HexCoord target = {row, col};
      int dist = hexDistance(unit->position, target);

      if (dist > 0 && dist <= range) {
        Unit *occupant = game.getUnitAt(target);
        if (occupant && occupant->side != unit->side) {
          game.map[row][col].isAttackSel = true;
        }
      }
    }
  }
}

void moveUnit(GameState &game, Unit *unit, const HexCoord &target) {
  if (!unit)
    return;

  // Calculate actual movement cost based on terrain
  int movMethodIdx = static_cast<int>(unit->movMethod);
  GameHex& targetHex = game.map[target.row][target.col];
  int terrainIdx = getTerrainIndex(targetHex.terrain);
  int cost = MOV_TABLE_DRY[movMethodIdx][terrainIdx];

  // Don't move if impassable
  if (cost >= 255) return;

  // For difficult terrain (cost 254), we can enter but it uses all remaining moves
  if (cost == 254) cost = unit->movesLeft;

  // Only move if we have enough movement points
  if (cost <= unit->movesLeft) {
    // Clear ZOC and spotting from old position
    setUnitZOC(game, unit, false);
    setUnitSpotRange(game, unit, false);

    // Store old position for fuel calculation
    HexCoord oldPos = unit->position;

    // Move unit
    unit->position = target;
    unit->movesLeft -= cost;
    unit->hasMoved = true;

    // Set ZOC and spotting at new position
    setUnitZOC(game, unit, true);
    setUnitSpotRange(game, unit, true);

    // Reduce fuel by hex distance (not terrain cost)
    int distance = hexDistance(oldPos, target);
    unit->fuel = std::max(0, unit->fuel - distance);
  }
}

void performAttack(GameState &game, Unit *attacker, Unit *defender) {
  if (!attacker || !defender || attacker->hasFired)
    return;

  // Simple combat calculation
  int attackValue = attacker->hardAttack + attacker->experience * 2;
  int defenseValue = defender->groundDefense + defender->entrenchment * 2 +
                     defender->experience;

  // Calculate damage
  int attackRoll = GetRandomValue(1, 10);
  int defenseRoll = GetRandomValue(1, 10);

  int attackerDamage =
      std::max(0, (attackValue + attackRoll) - (defenseValue + defenseRoll));
  int defenderDamage = std::max(0, (defenseValue + defenseRoll / 2) -
                                       (attackValue + attackRoll / 2));

  // Apply damage
  defender->strength = std::max(0, defender->strength - attackerDamage / 3);
  attacker->strength = std::max(0, attacker->strength - defenderDamage / 4);

  // Increase experience
  if (attackerDamage > 0)
    attacker->experience = std::min(5, attacker->experience + 1);

  // Mark as fired
  attacker->hasFired = true;
  attacker->ammo = std::max(0, attacker->ammo - 1);

  // Clear ZOC and spotting for units about to be destroyed
  for (auto &unit : game.units) {
    if (unit->strength <= 0) {
      setUnitZOC(game, unit.get(), false);
      setUnitSpotRange(game, unit.get(), false);
    }
  }

  // Remove destroyed units
  game.units.erase(std::remove_if(game.units.begin(), game.units.end(),
                                  [](const std::unique_ptr<Unit> &u) {
                                    return u->strength <= 0;
                                  }),
                   game.units.end());
}

void endTurn(GameState &game) {
  // Reset unit actions
  for (auto &unit : game.units) {
    if (unit->side == game.currentPlayer) {
      unit->hasMoved = false;
      unit->hasFired = false;
      unit->movesLeft = unit->movementPoints;

      // Increase entrenchment if unit didn't move
      if (!unit->hasMoved) {
        unit->entrenchment = std::min(5, unit->entrenchment + 1);
      } else {
        unit->entrenchment = 0;
      }
    }
  }

  // Switch player
  game.currentPlayer = 1 - game.currentPlayer;

  // If both players have moved, advance turn
  if (game.currentPlayer == 0) {
    game.currentTurn++;
  }

  // Clear selection
  game.selectedUnit = nullptr;
  clearSelectionHighlights(game);
}

// Calculate centered camera offset to center the hex map in the play area
void calculateCenteredCameraOffset(CameraState& camera, int screenWidth, int screenHeight) {
  // Play area: excludes top bar (40px) and right panel (250px) and bottom bar (30px)
  float playAreaWidth = screenWidth - 250;
  float playAreaHeight = screenHeight - 70;  // 40px top + 30px bottom
  float playAreaCenterX = playAreaWidth / 2.0f;
  float playAreaCenterY = 40.0f + playAreaHeight / 2.0f;

  // Calculate the center of the hex map in world coordinates
  // The map center is approximately at hex (MAP_ROWS/2, MAP_COLS/2)
  HexCoord mapCenter = {MAP_ROWS / 2, MAP_COLS / 2};
  OffsetCoord offset = gameCoordToOffset(mapCenter);
  ::Hex cubeHex = offset_to_cube(offset);

  // Create a temporary layout to calculate pixel position
  Layout tempLayout = createHexLayout(HEX_SIZE, 0, 0, camera.zoom);
  Point mapCenterPixel = hex_to_pixel(tempLayout, cubeHex);

  // Calculate offset so that map center appears at play area center
  camera.offsetX = playAreaCenterX - mapCenterPixel.x;
  camera.offsetY = playAreaCenterY - mapCenterPixel.y;
}

// Forward declarations
void saveConfig(const VideoSettings& settings);
void loadConfig(VideoSettings& settings);
void applyGuiScale(float scale);
void loadStyleTheme(const std::string& themeName);

void drawUI(GameState &game) {
  // Turn info panel (status bar)
  DrawRectangleRec(game.layout.statusBar, Color{40, 40, 40, 240});

  std::string turnText = "Turn: " + std::to_string(game.currentTurn) + "/" +
                         std::to_string(game.maxTurns);
  DrawText(turnText.c_str(), 10, 10, 20, WHITE);

  std::string playerText = game.currentPlayer == 0 ? "Axis" : "Allied";
  playerText = "Current: " + playerText;
  DrawText(playerText.c_str(), 200, 10, 20,
           game.currentPlayer == 0 ? RED : BLUE);

  // Zoom indicator
  char zoomText[32];
  snprintf(zoomText, sizeof(zoomText), "Zoom: %.0f%%", game.camera.zoom * 100);
  DrawText(zoomText, 400, 10, 20, WHITE);

  // Reset Map button
  if (GuiButton(Rectangle{game.layout.statusBar.width - 240, 5, 120, 30},
                "RESET MAP")) {
    // Reset camera to center and 100% zoom
    game.camera.zoom = 1.0f;
    game.camera.zoomDirection = 0;
    calculateCenteredCameraOffset(game.camera, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  // Options button
  if (GuiButton(Rectangle{game.layout.statusBar.width - 110, 5, 100, 30},
                "OPTIONS")) {
    game.showOptionsMenu = !game.showOptionsMenu;
  }

  // Unit info panel
  if (game.selectedUnit && !game.showOptionsMenu) {
    DrawRectangleRec(game.layout.unitPanel, Color{40, 40, 40, 240});

    Unit *unit = game.selectedUnit;
    int y = (int)game.layout.unitPanel.y + 10;
    int x = (int)game.layout.unitPanel.x + 10;

    DrawText(unit->name.c_str(), x, y, 20, WHITE);
    y += 30;

    std::string info = "Strength: " + std::to_string(unit->strength) + "/" +
                       std::to_string(unit->maxStrength);
    DrawText(info.c_str(), x, y, 16, WHITE);
    y += 25;

    info = "Experience: " + std::string(unit->experience, '*');
    DrawText(info.c_str(), x, y, 16, YELLOW);
    y += 25;

    info = "Moves: " + std::to_string(unit->movesLeft) + "/" +
           std::to_string(unit->movementPoints);
    DrawText(info.c_str(), x, y, 16, WHITE);
    y += 25;

    info = "Fuel: " + std::to_string(unit->fuel);
    DrawText(info.c_str(), x, y, 16, WHITE);
    y += 25;

    info = "Ammo: " + std::to_string(unit->ammo);
    DrawText(info.c_str(), x, y, 16, WHITE);
    y += 25;

    info = "Entrenchment: " + std::to_string(unit->entrenchment);
    DrawText(info.c_str(), x, y, 16, WHITE);
    y += 35;

    DrawText("Stats:", x, y, 16, GRAY);
    y += 20;
    info = "Hard Atk: " + std::to_string(unit->hardAttack);
    DrawText(info.c_str(), x, y, 14, WHITE);
    y += 20;
    info = "Soft Atk: " + std::to_string(unit->softAttack);
    DrawText(info.c_str(), x, y, 14, WHITE);
    y += 20;
    info = "Defense: " + std::to_string(unit->groundDefense);
    DrawText(info.c_str(), x, y, 14, WHITE);
  }
}

void drawOptionsMenu(GameState &game, bool &needsRestart) {
  int menuWidth = 600;
  int menuHeight = 650;
  int menuX = (SCREEN_WIDTH - menuWidth) / 2;
  int menuY = (SCREEN_HEIGHT - menuHeight) / 2;

  // Draw background overlay
  DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 180});

  // Get colors from current style
  Color backgroundColor = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
  Color borderColor = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED));
  Color titleColor = GetColor(GuiGetStyle(DEFAULT, LINE_COLOR));

  // Draw menu panel
  DrawRectangle(menuX, menuY, menuWidth, menuHeight, backgroundColor);
  DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, borderColor);

  // Title
  DrawText("VIDEO OPTIONS", menuX + 20, menuY + 15, 30, titleColor);

  int y = menuY + 70;
  int labelX = menuX + 30;
  int controlX = menuX + 250;
  int controlWidth = 300;

  // Get label and text colors from style
  Color labelColor = GetColor(GuiGetStyle(LABEL, TEXT_COLOR_NORMAL));
  Color valueColor = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED));

  // Store positions for dropdowns to draw them last
  int resolutionY = y;
  y += 50;
  int fullscreenY = y;
  y += 50;
  int vsyncY = y;
  y += 50;
  int fpsY = y;
  y += 50;
  int guiScaleY = y;
  y += 50;
  int styleThemeY = y;
  y += 50;

  // Draw labels and non-dropdown controls first
  // Resolution label
  DrawText("Resolution:", labelX, resolutionY, 20, labelColor);

  // Fullscreen
  DrawText("Fullscreen:", labelX, fullscreenY, 20, labelColor);
  GuiCheckBox(Rectangle{(float)controlX, (float)fullscreenY - 5, 30, 30}, "",
              &game.settings.fullscreen);

  // VSync
  DrawText("VSync:", labelX, vsyncY, 20, labelColor);
  GuiCheckBox(Rectangle{(float)controlX, (float)vsyncY - 5, 30, 30}, "",
              &game.settings.vsync);

  // FPS Target label
  DrawText("FPS Target:", labelX, fpsY, 20, labelColor);
  std::string currentFps =
      game.settings.fpsIndex == 6
          ? "Unlimited"
          : std::to_string(FPS_VALUES[game.settings.fpsIndex]);
  DrawText(currentFps.c_str(), controlX + controlWidth + 15, fpsY, 20, valueColor);

  // GUI Scale label
  DrawText("GUI Scale:", labelX, guiScaleY, 20, labelColor);

  // Style Theme label
  DrawText("Style Theme:", labelX, styleThemeY, 20, labelColor);

  // MSAA
  DrawText("Anti-Aliasing (4x):", labelX, y, 20, labelColor);
  bool oldMsaa = game.settings.msaa;
  GuiCheckBox(Rectangle{(float)controlX, (float)y - 5, 30, 30}, "",
              &game.settings.msaa);
  if (game.settings.msaa != oldMsaa)
    needsRestart = true;
  y += 50;

  // Hex Size Slider
  DrawText("Hex Size:", labelX, y, 20, labelColor);
  GuiSlider(Rectangle{(float)controlX, (float)y, (float)controlWidth, 20}, "20",
            "80", &game.settings.hexSize, 20, 80);
  std::string hexSizeStr = std::to_string((int)game.settings.hexSize);
  DrawText(hexSizeStr.c_str(), controlX + controlWidth + 15, y, 20, valueColor);
  y += 50;

  // Pan Speed Slider
  DrawText("Camera Pan Speed:", labelX, y, 20, labelColor);
  GuiSlider(Rectangle{(float)controlX, (float)y, (float)controlWidth, 20}, "1",
            "20", &game.settings.panSpeed, 1, 20);
  std::string panSpeedStr = std::to_string((int)game.settings.panSpeed);
  DrawText(panSpeedStr.c_str(), controlX + controlWidth + 15, y, 20, valueColor);
  y += 60;

  // Buttons
  int buttonY = menuY + menuHeight - 70;
  if (GuiButton(Rectangle{(float)menuX + 30, (float)buttonY, 150, 40},
                "Apply")) {
    // Close any open dropdowns
    game.settings.resolutionDropdownEdit = false;
    game.settings.fpsDropdownEdit = false;
    game.settings.guiScaleDropdownEdit = false;
    game.settings.styleThemeDropdownEdit = false;

    // Apply settings
    Resolution res = RESOLUTIONS[game.settings.resolutionIndex];

    if (res.width != SCREEN_WIDTH || res.height != SCREEN_HEIGHT) {
      SetWindowSize(res.width, res.height);
      SCREEN_WIDTH = res.width;
      SCREEN_HEIGHT = res.height;
      game.layout.recalculate(res.width, res.height);
    }

    if (game.settings.fullscreen != IsWindowFullscreen()) {
      ToggleFullscreen();
      game.layout.recalculate(GetScreenWidth(), GetScreenHeight());
    }

    SetTargetFPS(FPS_VALUES[game.settings.fpsIndex]);

    // Apply hex size
    HEX_SIZE = game.settings.hexSize;

    // Apply style theme
    loadStyleTheme(game.settings.styleTheme);

    // Apply GUI scale (after style is loaded)
    applyGuiScale(GUI_SCALE_VALUES[game.settings.guiScaleIndex]);

    // Save config to file
    saveConfig(game.settings);

    // Menu stays open after applying settings
  }

  if (GuiButton(Rectangle{(float)menuX + 220, (float)buttonY, 150, 40},
                "Cancel")) {
    // Close any open dropdowns
    game.settings.resolutionDropdownEdit = false;
    game.settings.fpsDropdownEdit = false;
    game.settings.guiScaleDropdownEdit = false;
    game.settings.styleThemeDropdownEdit = false;
    game.showOptionsMenu = false;
  }

  if (GuiButton(Rectangle{(float)menuX + 410, (float)buttonY, 150, 40},
                "Defaults")) {
    game.settings.resolutionIndex = 6; // 1920x1080
    game.settings.fullscreen = true;
    game.settings.vsync = false;
    game.settings.fpsIndex = 6; // Unlimited FPS
    game.settings.hexSize = 40.0f;
    game.settings.panSpeed = 5.0f;
    game.settings.msaa = false;
    game.settings.guiScaleIndex = 0; // 1.0
    game.settings.styleTheme = "dark";
  }

  // Draw dropdowns last so they appear on top
  // Draw from bottom to top (Style Theme, GUI Scale, FPS, Resolution) so top dropdowns overlap bottom ones
  std::string resLabels;
  for (int i = 0; i < RESOLUTION_COUNT; i++) {
    if (i > 0)
      resLabels += ";";
    resLabels += RESOLUTIONS[i].label;
  }

  // Style Theme dropdown (draw first - bottommost)
  // Get current style index
  int currentStyleIndex = getStyleIndex(game.settings.styleTheme);
  if (GuiDropdownBox(
          Rectangle{(float)controlX, (float)styleThemeY - 5, (float)controlWidth, 30},
          STYLE_LABELS_STRING.c_str(), &currentStyleIndex, game.settings.styleThemeDropdownEdit)) {
    game.settings.styleThemeDropdownEdit = !game.settings.styleThemeDropdownEdit;
  }
  // Update style theme name if index changed
  if (currentStyleIndex >= 0 && currentStyleIndex < (int)AVAILABLE_STYLES.size()) {
    game.settings.styleTheme = AVAILABLE_STYLES[currentStyleIndex];
  }

  // GUI Scale dropdown
  if (GuiDropdownBox(
          Rectangle{(float)controlX, (float)guiScaleY - 5, (float)controlWidth, 30},
          GUI_SCALE_LABELS, &game.settings.guiScaleIndex, game.settings.guiScaleDropdownEdit)) {
    game.settings.guiScaleDropdownEdit = !game.settings.guiScaleDropdownEdit;
  }

  // FPS Target dropdown
  if (GuiDropdownBox(
          Rectangle{(float)controlX, (float)fpsY - 5, (float)controlWidth, 30},
          FPS_LABELS, &game.settings.fpsIndex, game.settings.fpsDropdownEdit)) {
    game.settings.fpsDropdownEdit = !game.settings.fpsDropdownEdit;
  }

  // Resolution dropdown (draw last - topmost, overlaps all others)
  if (GuiDropdownBox(
          Rectangle{(float)controlX, (float)resolutionY - 5, (float)controlWidth, 30},
          resLabels.c_str(), &game.settings.resolutionIndex,
          game.settings.resolutionDropdownEdit)) {
    game.settings.resolutionDropdownEdit =
        !game.settings.resolutionDropdownEdit;
  }

  // Restart warning
  if (needsRestart) {
    Color warningColor = GetColor(GuiGetStyle(DEFAULT, LINE_COLOR));
    DrawText("Note: MSAA requires restart to take effect", menuX + 30,
             menuY + menuHeight - 25, 14, warningColor);
  }
}

// Handle mouse zoom with special behavior
void handleZoom(GameState &game) {
  float wheelMove = GetMouseWheelMove();

  if (wheelMove != 0) {
    float oldZoom = game.camera.zoom;
    int direction = wheelMove > 0 ? 1 : -1;  // 1 for zoom in, -1 for zoom out

    // Check if we're continuing in the same direction or changing direction
    bool changingDirection = (game.camera.zoomDirection != 0 &&
                             game.camera.zoomDirection != direction);

    // If we're at 100% and starting to zoom
    bool startingFromNeutral = (oldZoom == 1.0f && game.camera.zoomDirection == 0);

    // If changing direction from 100%, reset the direction counter
    if (oldZoom == 1.0f && changingDirection) {
      game.camera.zoomDirection = 0;
    }

    // Determine if we should actually zoom
    bool shouldZoom = false;

    if (startingFromNeutral) {
      // Starting from 100% - zoom immediately
      shouldZoom = true;
      game.camera.zoomDirection = direction;
    } else if (oldZoom == 1.0f && game.camera.zoomDirection == direction) {
      // Second input in same direction from 100% - zoom again
      shouldZoom = true;
    } else if (oldZoom != 1.0f && changingDirection) {
      // Changing direction while not at 100% - first input goes back to 100%
      shouldZoom = true;
      game.camera.zoomDirection = 0;  // Reset direction
    } else if (oldZoom != 1.0f && game.camera.zoomDirection == direction) {
      // Continuing in same direction - zoom normally
      shouldZoom = true;
    } else if (oldZoom != 1.0f && game.camera.zoomDirection == 0) {
      // After returning to 100%, first input in any direction
      shouldZoom = true;
      game.camera.zoomDirection = direction;
    }

    if (shouldZoom) {
      float newZoom = oldZoom + (direction * 0.25f);

      // Clamp zoom between 0.5 (50%) and 2.0 (200%)
      newZoom = Clamp(newZoom, 0.5f, 2.0f);

      if (newZoom != oldZoom) {
        // Get mouse position for zoom center
        Vector2 mousePos = GetMousePosition();

        // Calculate world position at mouse before zoom
        Point worldPosOld((mousePos.x - game.camera.offsetX) / oldZoom,
                         (mousePos.y - game.camera.offsetY) / oldZoom);

        // Apply zoom
        game.camera.zoom = newZoom;

        // Calculate world position at mouse after zoom
        Point worldPosNew((mousePos.x - game.camera.offsetX) / newZoom,
                         (mousePos.y - game.camera.offsetY) / newZoom);

        // Adjust offset to keep world position under mouse constant
        game.camera.offsetX += (worldPosNew.x - worldPosOld.x) * newZoom;
        game.camera.offsetY += (worldPosNew.y - worldPosOld.y) * newZoom;

        // Update zoom direction if we're moving away from 100%
        if (newZoom != 1.0f && game.camera.zoomDirection != direction) {
          game.camera.zoomDirection = direction;
        } else if (newZoom == 1.0f) {
          // Reset direction when we reach 100%
          game.camera.zoomDirection = 0;
        }
      }
    }
  }
}

// Handle middle mouse button panning
void handlePan(GameState &game) {
  if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
    game.camera.isPanning = true;
    game.camera.panStartMouse = GetMousePosition();
    game.camera.panStartOffset = Vector2{game.camera.offsetX, game.camera.offsetY};
  }

  if (IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE)) {
    game.camera.isPanning = false;
  }

  if (game.camera.isPanning) {
    Vector2 currentMouse = GetMousePosition();
    Vector2 delta = {
      currentMouse.x - game.camera.panStartMouse.x,
      currentMouse.y - game.camera.panStartMouse.y
    };

    game.camera.offsetX = game.camera.panStartOffset.x + delta.x;
    game.camera.offsetY = game.camera.panStartOffset.y + delta.y;
  }
}

// Save config to file
void saveConfig(const VideoSettings& settings) {
  std::ofstream configFile("config.txt");
  if (!configFile.is_open()) {
    TraceLog(LOG_WARNING, "Failed to save config.txt");
    return;
  }

  configFile << "resolutionIndex=" << settings.resolutionIndex << "\n";
  configFile << "fullscreen=" << (settings.fullscreen ? 1 : 0) << "\n";
  configFile << "vsync=" << (settings.vsync ? 1 : 0) << "\n";
  configFile << "fpsIndex=" << settings.fpsIndex << "\n";
  configFile << "hexSize=" << settings.hexSize << "\n";
  configFile << "panSpeed=" << settings.panSpeed << "\n";
  configFile << "msaa=" << (settings.msaa ? 1 : 0) << "\n";
  configFile << "guiScaleIndex=" << settings.guiScaleIndex << "\n";
  configFile << "styleTheme=" << settings.styleTheme << "\n";

  configFile.close();
  TraceLog(LOG_INFO, "Config saved to config.txt");
}

// Load config from file
void loadConfig(VideoSettings& settings) {
  std::ifstream configFile("config.txt");
  if (!configFile.is_open()) {
    TraceLog(LOG_INFO, "No config.txt found, creating default config");
    saveConfig(settings);
    return;
  }

  std::string line;
  while (std::getline(configFile, line)) {
    // Skip empty lines and comments
    if (line.empty() || line[0] == '#') continue;

    size_t equalPos = line.find('=');
    if (equalPos == std::string::npos) continue;

    std::string key = line.substr(0, equalPos);
    std::string value = line.substr(equalPos + 1);

    try {
      if (key == "resolutionIndex") {
        int val = std::stoi(value);
        if (val >= 0 && val < RESOLUTION_COUNT) {
          settings.resolutionIndex = val;
        }
      } else if (key == "fullscreen") {
        settings.fullscreen = (std::stoi(value) != 0);
      } else if (key == "vsync") {
        settings.vsync = (std::stoi(value) != 0);
      } else if (key == "fpsIndex") {
        int val = std::stoi(value);
        if (val >= 0 && val <= 6) {
          settings.fpsIndex = val;
        }
      } else if (key == "hexSize") {
        float val = std::stof(value);
        if (val >= 20.0f && val <= 80.0f) {
          settings.hexSize = val;
        }
      } else if (key == "panSpeed") {
        float val = std::stof(value);
        if (val >= 1.0f && val <= 20.0f) {
          settings.panSpeed = val;
        }
      } else if (key == "msaa") {
        settings.msaa = (std::stoi(value) != 0);
      } else if (key == "guiScaleIndex") {
        int val = std::stoi(value);
        if (val >= 0 && val < GUI_SCALE_COUNT) {
          settings.guiScaleIndex = val;
        }
      } else if (key == "styleTheme") {
        settings.styleTheme = value;
      }
    } catch (const std::exception& e) {
      // Ignore malformed values
      TraceLog(LOG_WARNING, TextFormat("Failed to parse config value: %s", key.c_str()));
    }
  }

  configFile.close();
  TraceLog(LOG_INFO, "Config loaded from config.txt");
}

// Apply GUI scale to raygui (currently disabled - functionality removed)
void applyGuiScale(float scale) {
  // GUI scaling functionality has been removed as requested
  // The menu option remains for potential future use
  TraceLog(LOG_INFO, TextFormat("GUI scale setting: %.2f (scaling disabled)", scale));
}

// Load style theme
void loadStyleTheme(const std::string& themeName) {
  std::string stylePath = "resources/styles/" + themeName + "/style_" + themeName + ".rgs";

  // Check if file exists
  std::ifstream styleFile(stylePath);
  if (!styleFile.good()) {
    TraceLog(LOG_WARNING, TextFormat("Style file not found: %s", stylePath.c_str()));
    return;
  }
  styleFile.close();

  // Load the style
  GuiLoadStyle(stylePath.c_str());
  TraceLog(LOG_INFO, TextFormat("Style loaded: %s", themeName.c_str()));
}

int main() {
  // Discover available styles first (before window init)
  discoverStyles();

  // Create temporary settings to load config before window init
  VideoSettings tempSettings;
  loadConfig(tempSettings);

  // Set config flags before window creation
  unsigned int flags = FLAG_WINDOW_RESIZABLE;
  if (tempSettings.vsync) {
    flags |= FLAG_VSYNC_HINT;
  }
  if (tempSettings.msaa) {
    flags |= FLAG_MSAA_4X_HINT;
  }
  SetConfigFlags(flags);

  // Apply resolution from config
  SCREEN_WIDTH = RESOLUTIONS[tempSettings.resolutionIndex].width;
  SCREEN_HEIGHT = RESOLUTIONS[tempSettings.resolutionIndex].height;
  HEX_SIZE = tempSettings.hexSize;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
             "Panzer General 2 Prototype - Raylib + RayGUI");

  // Disable ESC key to exit - we use it for menu control
  SetExitKey(KEY_NULL);

  // Apply fullscreen from config
  if (tempSettings.fullscreen && !IsWindowFullscreen()) {
    ToggleFullscreen();
  }

  // Apply FPS from config
  SetTargetFPS(FPS_VALUES[tempSettings.fpsIndex]);

  // Load style theme from config
  loadStyleTheme(tempSettings.styleTheme);

  // Apply GUI scale from config
  applyGuiScale(GUI_SCALE_VALUES[tempSettings.guiScaleIndex]);

  GameState game;
  // Apply loaded settings to game state
  game.settings = tempSettings;

  // Center the camera on the hex map
  calculateCenteredCameraOffset(game.camera, SCREEN_WIDTH, SCREEN_HEIGHT);

  // Add some initial units
  game.addUnit(UnitClass::INFANTRY, 0, 2, 2);
  game.addUnit(UnitClass::TANK, 0, 2, 3);
  game.addUnit(UnitClass::ARTILLERY, 0, 1, 2);

  game.addUnit(UnitClass::INFANTRY, 1, 8, 10);
  game.addUnit(UnitClass::TANK, 1, 9, 10);
  game.addUnit(UnitClass::RECON, 1, 8, 11);

  // Initialize Zone of Control and Spotting for all units
  initializeAllZOC(game);
  initializeAllSpotting(game);

  bool needsRestart = false;

  while (!WindowShouldClose()) {
    // Input handling (only when menu is closed)
    if (!game.showOptionsMenu) {
      // Handle zoom
      handleZoom(game);

      // Handle middle mouse panning
      handlePan(game);

      if (IsKeyPressed(KEY_SPACE)) {
        endTurn(game);
      }

      if (IsKeyPressed(KEY_ESCAPE)) {
        game.showOptionsMenu = true;
      }

      // Mouse input for unit selection
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();

        // Convert mouse position to hex coordinate
        Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
                                       game.camera.offsetY, game.camera.zoom);
        Point mousePoint(mousePos.x, mousePos.y);
        FractionalHex fracHex = pixel_to_hex(layout, mousePoint);
        ::Hex cubeHex = hex_round(fracHex);
        OffsetCoord offset = cube_to_offset(cubeHex);
        HexCoord clickedHex = offsetToGameCoord(offset);

        // Check if clicked on a hex that's within map bounds
        if (clickedHex.row >= 0 && clickedHex.row < MAP_ROWS &&
            clickedHex.col >= 0 && clickedHex.col < MAP_COLS) {

          Unit *clickedUnit = game.getUnitAt(clickedHex);

          if (game.selectedUnit) {
            // Try to move or attack
            if (game.map[clickedHex.row][clickedHex.col].isMoveSel) {
              moveUnit(game, game.selectedUnit, clickedHex);
              clearSelectionHighlights(game);
              highlightAttackRange(game, game.selectedUnit);
            } else if (game.map[clickedHex.row][clickedHex.col].isAttackSel) {
              if (clickedUnit) {
                performAttack(game, game.selectedUnit, clickedUnit);
                clearSelectionHighlights(game);
                game.selectedUnit = nullptr;
              }
            } else if (clickedUnit && clickedUnit->side == game.currentPlayer) {
              // Select new unit
              game.selectedUnit = clickedUnit;
              highlightMovementRange(game, game.selectedUnit);
              highlightAttackRange(game, game.selectedUnit);
            } else {
              // Deselect
              game.selectedUnit = nullptr;
              clearSelectionHighlights(game);
            }
          } else if (clickedUnit && clickedUnit->side == game.currentPlayer) {
            // Select unit
            game.selectedUnit = clickedUnit;
            highlightMovementRange(game, game.selectedUnit);
            highlightAttackRange(game, game.selectedUnit);
          }
        }
      }

      // Keyboard zoom controls (R = zoom in, F = zoom out)
      if (IsKeyPressed(KEY_R)) {
        float oldZoom = game.camera.zoom;
        float newZoom = oldZoom + 0.25f;
        newZoom = Clamp(newZoom, 0.5f, 2.0f);

        if (newZoom != oldZoom) {
          // Get screen center for zoom
          Vector2 centerPos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};

          // Calculate world position at center before zoom
          Point worldPosOld((centerPos.x - game.camera.offsetX) / oldZoom,
                           (centerPos.y - game.camera.offsetY) / oldZoom);

          // Apply zoom
          game.camera.zoom = newZoom;

          // Calculate world position at center after zoom
          Point worldPosNew((centerPos.x - game.camera.offsetX) / newZoom,
                           (centerPos.y - game.camera.offsetY) / newZoom);

          // Adjust offset to keep world position under center constant
          game.camera.offsetX += (worldPosNew.x - worldPosOld.x) * newZoom;
          game.camera.offsetY += (worldPosNew.y - worldPosOld.y) * newZoom;

          // Update zoom direction
          if (newZoom != 1.0f) {
            game.camera.zoomDirection = 1;  // Zooming in
          } else {
            game.camera.zoomDirection = 0;
          }
        }
      }

      if (IsKeyPressed(KEY_F)) {
        float oldZoom = game.camera.zoom;
        float newZoom = oldZoom - 0.25f;
        newZoom = Clamp(newZoom, 0.5f, 2.0f);

        if (newZoom != oldZoom) {
          // Get screen center for zoom
          Vector2 centerPos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};

          // Calculate world position at center before zoom
          Point worldPosOld((centerPos.x - game.camera.offsetX) / oldZoom,
                           (centerPos.y - game.camera.offsetY) / oldZoom);

          // Apply zoom
          game.camera.zoom = newZoom;

          // Calculate world position at center after zoom
          Point worldPosNew((centerPos.x - game.camera.offsetX) / newZoom,
                           (centerPos.y - game.camera.offsetY) / newZoom);

          // Adjust offset to keep world position under center constant
          game.camera.offsetX += (worldPosNew.x - worldPosOld.x) * newZoom;
          game.camera.offsetY += (worldPosNew.y - worldPosOld.y) * newZoom;

          // Update zoom direction
          if (newZoom != 1.0f) {
            game.camera.zoomDirection = -1;  // Zooming out
          } else {
            game.camera.zoomDirection = 0;
          }
        }
      }

      // Camera panning with arrow keys and WASD (absolute directions)
      float panSpeed = game.settings.panSpeed;
      if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        game.camera.offsetX -= panSpeed;  // Pan left
      if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        game.camera.offsetX += panSpeed;  // Pan right
      if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        game.camera.offsetY -= panSpeed;  // Pan up
      if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        game.camera.offsetY += panSpeed;  // Pan down
    } else {
      // Close menu with ESC (close dropdowns first if open)
      if (IsKeyPressed(KEY_ESCAPE)) {
        if (game.settings.resolutionDropdownEdit ||
            game.settings.fpsDropdownEdit ||
            game.settings.guiScaleDropdownEdit ||
            game.settings.styleThemeDropdownEdit) {
          // Close any open dropdowns first
          game.settings.resolutionDropdownEdit = false;
          game.settings.fpsDropdownEdit = false;
          game.settings.guiScaleDropdownEdit = false;
          game.settings.styleThemeDropdownEdit = false;
        } else {
          // Close the menu
          game.showOptionsMenu = false;
        }
      }
    }

    // Drawing
    BeginDrawing();
    ClearBackground(COLOR_BACKGROUND);

    drawMap(game);
    drawUI(game);

    // Draw options menu on top
    if (game.showOptionsMenu) {
      drawOptionsMenu(game, needsRestart);
    }

    // Draw FPS in bottom right corner
    DrawFPS(SCREEN_WIDTH - 100, SCREEN_HEIGHT - 30);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
