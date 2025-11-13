#include "raylib.h"
#include "raymath.h"

#include "imgui.h"
#include "rlImGui.h"

#include "hex.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

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

// Camera state structure
struct CameraState {
  float offsetX;
  float offsetY;
  float zoom;           // 0.5 to 2.0 (50% to 200%)
  bool isPanning;
  Vector2 panStartMouse;
  Vector2 panStartOffset;

  CameraState()
      : offsetX(100.0f), offsetY(100.0f), zoom(1.0f),
        isPanning(false), panStartMouse{0, 0}, panStartOffset{0, 0} {}
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

  VideoSettings()
      : resolutionIndex(6), fullscreen(true), vsync(false), fpsIndex(6),
        hexSize(40.0f), panSpeed(5.0f), msaa(false) {}
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
  bool isMoveSel;    // highlighted for movement
  bool isAttackSel;  // highlighted for attack

  GameHex()
      : terrain(TerrainType::PLAINS), owner(-1), isVictoryHex(false),
        isDeployment(false), isMoveSel(false), isAttackSel(false) {
    isSpotted[0] = false;
    isSpotted[1] = false;
  }
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
        initiative(5), movementPoints(6), movesLeft(6), fuel(50), ammo(20),
        spotRange(2), hasMoved(false), hasFired(false), isCore(false) {}
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

    // Set unit name based on class
    switch (uClass) {
    case UnitClass::INFANTRY:
      unit->name = "Infantry";
      break;
    case UnitClass::TANK:
      unit->name = "Tank";
      break;
    case UnitClass::ARTILLERY:
      unit->name = "Artillery";
      break;
    case UnitClass::RECON:
      unit->name = "Recon";
      break;
    case UnitClass::ANTI_TANK:
      unit->name = "Anti-Tank";
      break;
    case UnitClass::AIR_DEFENSE:
      unit->name = "Air Defense";
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

// Global font and shader for SDF rendering
Font g_fontSDF = { 0 };
Shader g_sdfShader = { 0 };

// Helper function to draw text with SDF font
void DrawTextSDF(const char* text, int posX, int posY, float fontSize, Color color) {
  if (g_fontSDF.texture.id == 0) {
    // Fallback to regular text if font not loaded
    DrawText(text, posX, posY, (int)fontSize, color);
    return;
  }

  BeginShaderMode(g_sdfShader);
  DrawTextEx(g_fontSDF, text, Vector2{(float)posX, (float)posY}, fontSize, 1.0f, color);
  EndShaderMode();
}

void drawMap(GameState &game) {
  Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
                                  game.camera.offsetY, game.camera.zoom);

  // Draw hexes
  for (int row = 0; row < MAP_ROWS; row++) {
    for (int col = 0; col < MAP_COLS; col++) {
      GameHex &hex = game.map[row][col];
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

  // Draw units
  for (auto &unit : game.units) {
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
    float fontSize = 10.0f * game.camera.zoom;
    if (fontSize >= 8.0f) {  // Only draw text if it's readable
      Vector2 textSize = MeasureTextEx(g_fontSDF, symbol.c_str(), fontSize, 1.0f);
      DrawTextSDF(symbol.c_str(),
                  (int)(center.x - textSize.x / 2),
                  (int)(center.y - unitHeight / 2 + 2),
                  fontSize, WHITE);

      // Draw strength
      std::string strength = std::to_string(unit->strength);
      fontSize = 12.0f * game.camera.zoom;
      textSize = MeasureTextEx(g_fontSDF, strength.c_str(), fontSize, 1.0f);
      DrawTextSDF(strength.c_str(),
                  (int)(center.x - textSize.x / 2),
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

void highlightMovementRange(GameState &game, Unit *unit) {
  clearSelectionHighlights(game);
  if (!unit)
    return;

  int range = unit->movesLeft;
  for (int row = 0; row < MAP_ROWS; row++) {
    for (int col = 0; col < MAP_COLS; col++) {
      HexCoord target = {row, col};
      int dist = hexDistance(unit->position, target);

      if (dist > 0 && dist <= range) {
        // Check if hex is valid for movement
        Unit *occupant = game.getUnitAt(target);
        if (!occupant || occupant->side == unit->side) {
          game.map[row][col].isMoveSel = true;
        }
      }
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

  int distance = hexDistance(unit->position, target);
  if (distance <= unit->movesLeft) {
    unit->position = target;
    unit->movesLeft -= distance;
    unit->hasMoved = true;

    // Reduce fuel
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

// Forward declarations
void saveConfig(const VideoSettings& settings);
void loadConfig(VideoSettings& settings);

void drawUI(GameState &game) {
  // Turn info panel (status bar)
  DrawRectangleRec(game.layout.statusBar, Color{40, 40, 40, 240});

  std::string turnText = "Turn: " + std::to_string(game.currentTurn) + "/" +
                         std::to_string(game.maxTurns);
  DrawTextSDF(turnText.c_str(), 10, 10, 20, WHITE);

  std::string playerText = game.currentPlayer == 0 ? "Axis" : "Allied";
  playerText = "Current: " + playerText;
  DrawTextSDF(playerText.c_str(), 200, 10, 20,
              game.currentPlayer == 0 ? RED : BLUE);

  // Zoom indicator
  char zoomText[32];
  snprintf(zoomText, sizeof(zoomText), "Zoom: %.0f%%", game.camera.zoom * 100);
  DrawTextSDF(zoomText, 400, 10, 20, WHITE);

  // Unit info panel
  if (game.selectedUnit && !game.showOptionsMenu) {
    DrawRectangleRec(game.layout.unitPanel, Color{40, 40, 40, 240});

    Unit *unit = game.selectedUnit;
    int y = (int)game.layout.unitPanel.y + 10;
    int x = (int)game.layout.unitPanel.x + 10;

    DrawTextSDF(unit->name.c_str(), x, y, 20, WHITE);
    y += 30;

    std::string info = "Strength: " + std::to_string(unit->strength) + "/" +
                       std::to_string(unit->maxStrength);
    DrawTextSDF(info.c_str(), x, y, 16, WHITE);
    y += 25;

    info = "Experience: " + std::string(unit->experience, '*');
    DrawTextSDF(info.c_str(), x, y, 16, YELLOW);
    y += 25;

    info = "Moves: " + std::to_string(unit->movesLeft) + "/" +
           std::to_string(unit->movementPoints);
    DrawTextSDF(info.c_str(), x, y, 16, WHITE);
    y += 25;

    info = "Fuel: " + std::to_string(unit->fuel);
    DrawTextSDF(info.c_str(), x, y, 16, WHITE);
    y += 25;

    info = "Ammo: " + std::to_string(unit->ammo);
    DrawTextSDF(info.c_str(), x, y, 16, WHITE);
    y += 25;

    info = "Entrenchment: " + std::to_string(unit->entrenchment);
    DrawTextSDF(info.c_str(), x, y, 16, WHITE);
    y += 35;

    DrawTextSDF("Stats:", x, y, 16, GRAY);
    y += 20;
    info = "Hard Atk: " + std::to_string(unit->hardAttack);
    DrawTextSDF(info.c_str(), x, y, 14, WHITE);
    y += 20;
    info = "Soft Atk: " + std::to_string(unit->softAttack);
    DrawTextSDF(info.c_str(), x, y, 14, WHITE);
    y += 20;
    info = "Defense: " + std::to_string(unit->groundDefense);
    DrawTextSDF(info.c_str(), x, y, 14, WHITE);
  }
}

void drawOptionsMenu(GameState &game, bool &needsRestart) {
  // Set up ImGui window
  ImGui::SetNextWindowSize(ImVec2(600, 550), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowPos(ImVec2((SCREEN_WIDTH - 600) / 2.0f, (SCREEN_HEIGHT - 550) / 2.0f), ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Video Options", &game.showOptionsMenu, ImGuiWindowFlags_NoResize)) {
    ImGui::Spacing();

    // Resolution dropdown
    ImGui::Text("Resolution:");
    ImGui::SameLine(200);
    if (ImGui::BeginCombo("##resolution", RESOLUTIONS[game.settings.resolutionIndex].label)) {
      for (int i = 0; i < RESOLUTION_COUNT; i++) {
        bool isSelected = (game.settings.resolutionIndex == i);
        if (ImGui::Selectable(RESOLUTIONS[i].label, isSelected)) {
          game.settings.resolutionIndex = i;
        }
        if (isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    ImGui::Spacing();

    // Fullscreen checkbox
    ImGui::Text("Fullscreen:");
    ImGui::SameLine(200);
    ImGui::Checkbox("##fullscreen", &game.settings.fullscreen);

    ImGui::Spacing();

    // VSync checkbox
    ImGui::Text("VSync:");
    ImGui::SameLine(200);
    ImGui::Checkbox("##vsync", &game.settings.vsync);

    ImGui::Spacing();

    // FPS Target dropdown
    ImGui::Text("FPS Target:");
    ImGui::SameLine(200);
    const char* fpsLabel = game.settings.fpsIndex == 6 ? "Unlimited" :
                           (std::to_string(FPS_VALUES[game.settings.fpsIndex])).c_str();
    if (ImGui::BeginCombo("##fps", fpsLabel)) {
      const char* fpsOptions[] = {"30", "60", "75", "120", "144", "240", "Unlimited"};
      for (int i = 0; i < 7; i++) {
        bool isSelected = (game.settings.fpsIndex == i);
        if (ImGui::Selectable(fpsOptions[i], isSelected)) {
          game.settings.fpsIndex = i;
        }
        if (isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    ImGui::Spacing();

    // MSAA checkbox
    ImGui::Text("Anti-Aliasing (4x):");
    ImGui::SameLine(200);
    bool oldMsaa = game.settings.msaa;
    ImGui::Checkbox("##msaa", &game.settings.msaa);
    if (game.settings.msaa != oldMsaa) {
      needsRestart = true;
    }

    ImGui::Spacing();

    // Hex Size slider
    ImGui::Text("Hex Size:");
    ImGui::SameLine(200);
    ImGui::SetNextItemWidth(300);
    ImGui::SliderFloat("##hexsize", &game.settings.hexSize, 20.0f, 80.0f, "%.0f");

    ImGui::Spacing();

    // Pan Speed slider
    ImGui::Text("Camera Pan Speed:");
    ImGui::SameLine(200);
    ImGui::SetNextItemWidth(300);
    ImGui::SliderFloat("##panspeed", &game.settings.panSpeed, 1.0f, 20.0f, "%.0f");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Buttons
    if (ImGui::Button("Apply", ImVec2(150, 40))) {
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

      // Save config to file
      saveConfig(game.settings);

      game.showOptionsMenu = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(150, 40))) {
      game.showOptionsMenu = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Defaults", ImVec2(150, 40))) {
      game.settings.resolutionIndex = 6; // 1920x1080
      game.settings.fullscreen = true;
      game.settings.vsync = false;
      game.settings.fpsIndex = 6; // Unlimited FPS
      game.settings.hexSize = 40.0f;
      game.settings.panSpeed = 5.0f;
      game.settings.msaa = false;
    }

    // Restart warning
    if (needsRestart) {
      ImGui::Spacing();
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Note: MSAA requires restart to take effect");
    }
  }
  ImGui::End();
}

// Handle mouse zoom - simple and straightforward
void handleZoom(GameState &game) {
  float wheelMove = GetMouseWheelMove();

  if (wheelMove != 0) {
    float oldZoom = game.camera.zoom;
    float zoomDelta = wheelMove * 0.1f;  // 10% per scroll

    float newZoom = oldZoom + zoomDelta;

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
      }
    } catch (const std::exception& e) {
      // Ignore malformed values
      TraceLog(LOG_WARNING, TextFormat("Failed to parse config value: %s", key.c_str()));
    }
  }

  configFile.close();
  TraceLog(LOG_INFO, "Config loaded from config.txt");
}

int main() {
  // Create temporary settings to load config before window init
  VideoSettings tempSettings;
  loadConfig(tempSettings);

  // Set config flags before window creation
  unsigned int flags = FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE;
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

  // Load DejaVu Sans font with SDF support
  int fileSize = 0;
  unsigned char *fileData = LoadFileData("resources/fonts/DejaVuSans.ttf", &fileSize);

  g_fontSDF.baseSize = 16;
  g_fontSDF.glyphCount = 95;
  g_fontSDF.glyphs = LoadFontData(fileData, fileSize, 16, 0, 95, FONT_SDF);

  Image atlas = GenImageFontAtlas(g_fontSDF.glyphs, &g_fontSDF.recs, 95, 16, 0, 1);
  g_fontSDF.texture = LoadTextureFromImage(atlas);
  UnloadImage(atlas);
  UnloadFileData(fileData);

  // Load SDF shader
  g_sdfShader = LoadShader(0, "resources/shaders/glsl330/sdf.fs");
  SetTextureFilter(g_fontSDF.texture, TEXTURE_FILTER_BILINEAR);

  // Initialize ImGui
  rlImGuiSetup(true);

  GameState game;
  // Apply loaded settings to game state
  game.settings = tempSettings;

  // Add some initial units
  game.addUnit(UnitClass::INFANTRY, 0, 2, 2);
  game.addUnit(UnitClass::TANK, 0, 2, 3);
  game.addUnit(UnitClass::ARTILLERY, 0, 1, 2);

  game.addUnit(UnitClass::INFANTRY, 1, 8, 10);
  game.addUnit(UnitClass::TANK, 1, 9, 10);
  game.addUnit(UnitClass::RECON, 1, 8, 11);

  bool needsRestart = false;

  while (!WindowShouldClose()) {
    // Begin ImGui frame
    rlImGuiBegin();

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

      // Toggle options menu with O key
      if (IsKeyPressed(KEY_O)) {
        game.showOptionsMenu = !game.showOptionsMenu;
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
    }

    // Drawing
    BeginDrawing();
    ClearBackground(COLOR_BACKGROUND);

    drawMap(game);
    drawUI(game);

    // Draw FPS in bottom right corner with SDF font
    char fpsText[32];
    snprintf(fpsText, sizeof(fpsText), "FPS: %d", GetFPS());
    DrawTextSDF(fpsText, SCREEN_WIDTH - 100, SCREEN_HEIGHT - 30, 20, COLOR_FPS);

    // Draw options menu on top using ImGui
    if (game.showOptionsMenu) {
      drawOptionsMenu(game, needsRestart);
    }

    // End ImGui frame
    rlImGuiEnd();

    EndDrawing();
  }

  // Shutdown ImGui
  rlImGuiShutdown();

  UnloadFont(g_fontSDF);  // Unload SDF font
  UnloadShader(g_sdfShader);  // Unload SDF shader
  CloseWindow();
  return 0;
}
