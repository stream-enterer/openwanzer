#include "raylib.h"
#include "raymath.h"

// Suppress warnings from raygui.h (external library)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wstringop-overflow"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#pragma GCC diagnostic pop

#include <algorithm>
#include <cmath>
#include <memory>
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
float HEX_WIDTH = HEX_SIZE * 2.0f;
float HEX_HEIGHT = sqrtf(3.0f) * HEX_SIZE;
int MAP_ROWS = DEFAULT_MAP_ROWS;
int MAP_COLS = DEFAULT_MAP_COLS;

// Enums
enum class TerrainType { CLEAR, FOREST, MOUNTAIN, CITY, WATER, ROAD };

enum class UnitClass {
  INFANTRY,
  TANK,
  ARTILLERY,
  RECON,
  ANTI_TANK,
  AIR_DEFENSE
};

enum class Side { AXIS = 0, ALLIED = 1 };

// Settings Structure
struct VideoSettings {
  int resolutionIndex;
  bool fullscreen;
  bool vsync;
  int fpsIndex;
  float hexSize;
  float panSpeed;
  bool msaa;
  bool resolutionDropdownEdit;
  bool fpsDropdownEdit;

  VideoSettings()
      : resolutionIndex(6), fullscreen(true), vsync(false), fpsIndex(6),
        hexSize(40.0f), panSpeed(5.0f), msaa(false),
        resolutionDropdownEdit(false), fpsDropdownEdit(false) {}
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

struct Hex {
  HexCoord coord;
  TerrainType terrain;
  int owner; // -1 = neutral, 0 = axis, 1 = allied
  bool isVictoryHex;
  bool isDeployment;
  bool isSpotted[2]; // spotted by each side
  bool isMoveSel;    // highlighted for movement
  bool isAttackSel;  // highlighted for attack

  Hex()
      : terrain(TerrainType::CLEAR), owner(-1), isVictoryHex(false),
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
  std::vector<std::vector<Hex>> map;
  std::vector<std::unique_ptr<Unit>> units;
  Unit *selectedUnit;
  int currentTurn;
  int currentPlayer; // 0 or 1
  int maxTurns;
  bool showOptionsMenu;
  VideoSettings settings;
  GameLayout layout;

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

        // Random terrain generation
        int randVal = GetRandomValue(0, 100);
        if (randVal < 60)
          map[row][col].terrain = TerrainType::CLEAR;
        else if (randVal < 75)
          map[row][col].terrain = TerrainType::FOREST;
        else if (randVal < 85)
          map[row][col].terrain = TerrainType::MOUNTAIN;
        else if (randVal < 92)
          map[row][col].terrain = TerrainType::CITY;
        else
          map[row][col].terrain = TerrainType::WATER;

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

// Hex drawing functions
Vector2 hexToPixel(int row, int col, float offsetX, float offsetY) {
  float x = offsetX + col * HEX_WIDTH * 0.75f;
  float y = offsetY + row * HEX_HEIGHT + (col % 2) * (HEX_HEIGHT * 0.5f);
  return {x, y};
}

HexCoord pixelToHex(float x, float y, float offsetX, float offsetY) {
  // Approximate hex coordinate (simplified axial to cube conversion)
  float adjustedX = (x - offsetX) / (HEX_WIDTH * 0.75f);
  int col = (int)roundf(adjustedX);

  float adjustedY =
      (y - offsetY - (col % 2) * (HEX_HEIGHT * 0.5f)) / HEX_HEIGHT;
  int row = (int)roundf(adjustedY);

  // Clamp to map bounds
  row = Clamp(row, 0, MAP_ROWS - 1);
  col = Clamp(col, 0, MAP_COLS - 1);

  return {row, col};
}

void drawHexagon(Vector2 center, float size, Color color, bool filled) {
  Vector2 points[6];
  for (int i = 0; i < 6; i++) {
    float angle = (60.0f * i - 30.0f) * DEG2RAD;
    points[i].x = center.x + size * cosf(angle);
    points[i].y = center.y + size * sinf(angle);
  }

  if (filled) {
    // Draw filled hexagon using triangles
    for (int i = 1; i < 5; i++) {
      DrawTriangle(points[0], points[i], points[i + 1], color);
    }
  } else {
    // Draw hexagon outline
    for (int i = 0; i < 6; i++) {
      DrawLineEx(points[i], points[(i + 1) % 6], 2.0f, color);
    }
  }
}

Color getTerrainColor(TerrainType terrain) {
  switch (terrain) {
  case TerrainType::CLEAR:
    return Color{220, 220, 180, 255};
  case TerrainType::FOREST:
    return Color{34, 139, 34, 255};
  case TerrainType::MOUNTAIN:
    return Color{139, 90, 43, 255};
  case TerrainType::CITY:
    return Color{128, 128, 128, 255};
  case TerrainType::WATER:
    return Color{64, 164, 223, 255};
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

void drawMap(GameState &game, float offsetX, float offsetY) {
  // Draw hexes
  for (int row = 0; row < MAP_ROWS; row++) {
    for (int col = 0; col < MAP_COLS; col++) {
      Hex &hex = game.map[row][col];
      Vector2 center = hexToPixel(row, col, offsetX, offsetY);

      // Draw terrain
      Color terrainColor = getTerrainColor(hex.terrain);
      drawHexagon(center, HEX_SIZE, terrainColor, true);

      // Draw hex outline
      drawHexagon(center, HEX_SIZE, BLACK, false);

      // Draw victory hex marker
      if (hex.isVictoryHex) {
        DrawCircle((int)center.x, (int)center.y, 8, GOLD);
      }

      // Draw movement/attack selection highlights
      if (hex.isMoveSel) {
        drawHexagon(center, HEX_SIZE - 3, Color{0, 255, 0, 100}, true);
      }
      if (hex.isAttackSel) {
        drawHexagon(center, HEX_SIZE - 3, Color{255, 0, 0, 100}, true);
      }
    }
  }

  // Draw units
  for (auto &unit : game.units) {
    Vector2 center =
        hexToPixel(unit->position.row, unit->position.col, offsetX, offsetY);

    // Draw unit square
    Color unitColor = getUnitColor(unit->side);
    DrawRectangle((int)center.x - 20, (int)center.y - 15, 40, 30, unitColor);
    DrawRectangleLines((int)center.x - 20, (int)center.y - 15, 40, 30, BLACK);

    // Draw unit symbol
    std::string symbol = getUnitSymbol(unit->unitClass);
    DrawText(symbol.c_str(), (int)center.x - 15, (int)center.y - 20, 10, WHITE);

    // Draw strength
    std::string strength = std::to_string(unit->strength);
    DrawText(strength.c_str(), (int)center.x - 5, (int)center.y + 5, 12,
             YELLOW);

    // Draw selection highlight
    if (unit.get() == game.selectedUnit) {
      DrawRectangleLines((int)center.x - 22, (int)center.y - 17, 44, 34,
                         YELLOW);
      DrawRectangleLines((int)center.x - 23, (int)center.y - 18, 46, 36,
                         YELLOW);
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

// Calculate Manhattan distance for hex grid
int hexDistance(const HexCoord &a, const HexCoord &b) {
  // Convert to cube coordinates for distance calculation
  int ax = a.col;
  int az = a.row - (a.col - (a.col & 1)) / 2;
  int ay = -ax - az;

  int bx = b.col;
  int bz = b.row - (b.col - (b.col & 1)) / 2;
  int by = -bx - bz;

  return (abs(ax - bx) + abs(ay - by) + abs(az - bz)) / 2;
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

  // Controls help (help bar)
  if (!game.showOptionsMenu) {
    DrawText(
        "SPACE - End Turn | Left Click - Select/Move/Attack | ESC - Options",
        (int)game.layout.helpBar.x + 10,
        (int)game.layout.helpBar.y + 5, 16, WHITE);
  }
}

void drawOptionsMenu(GameState &game, bool &needsRestart) {
  int menuWidth = 600;
  int menuHeight = 550;
  int menuX = (SCREEN_WIDTH - menuWidth) / 2;
  int menuY = (SCREEN_HEIGHT - menuHeight) / 2;

  // Draw background overlay
  DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 180});

  // Draw menu panel
  DrawRectangle(menuX, menuY, menuWidth, menuHeight, Color{40, 40, 40, 255});
  DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, GOLD);

  // Title
  DrawText("VIDEO OPTIONS", menuX + 20, menuY + 15, 30, GOLD);

  int y = menuY + 70;
  int labelX = menuX + 30;
  int controlX = menuX + 250;
  int controlWidth = 300;

  // Store positions for dropdowns to draw them last
  int resolutionY = y;
  y += 50;
  int fullscreenY = y;
  y += 50;
  int vsyncY = y;
  y += 50;
  int fpsY = y;
  y += 50;

  // Draw labels and non-dropdown controls first
  // Resolution label
  DrawText("Resolution:", labelX, resolutionY, 20, WHITE);

  // Fullscreen
  DrawText("Fullscreen:", labelX, fullscreenY, 20, WHITE);
  if (!game.settings.resolutionDropdownEdit) {
    GuiCheckBox(Rectangle{(float)controlX, (float)fullscreenY - 5, 30, 30}, "",
                &game.settings.fullscreen);
  }

  // VSync
  DrawText("VSync:", labelX, vsyncY, 20, WHITE);
  if (!game.settings.resolutionDropdownEdit) {
    GuiCheckBox(Rectangle{(float)controlX, (float)vsyncY - 5, 30, 30}, "",
                &game.settings.vsync);
  }

  // FPS Target label
  DrawText("FPS Target:", labelX, fpsY, 20, WHITE);
  std::string currentFps =
      game.settings.fpsIndex == 6
          ? "Unlimited"
          : std::to_string(FPS_VALUES[game.settings.fpsIndex]);
  if (!game.settings.resolutionDropdownEdit && !game.settings.fpsDropdownEdit) {
    DrawText(currentFps.c_str(), controlX + controlWidth + 15, fpsY, 20, GRAY);
  }

  // MSAA
  DrawText("Anti-Aliasing (4x):", labelX, y, 20, WHITE);
  bool oldMsaa = game.settings.msaa;
  if (!game.settings.resolutionDropdownEdit && !game.settings.fpsDropdownEdit) {
    GuiCheckBox(Rectangle{(float)controlX, (float)y - 5, 30, 30}, "",
                &game.settings.msaa);
    if (game.settings.msaa != oldMsaa)
      needsRestart = true;
  }
  y += 50;

  // Hex Size Slider
  DrawText("Hex Size:", labelX, y, 20, WHITE);
  if (!game.settings.resolutionDropdownEdit && !game.settings.fpsDropdownEdit) {
    GuiSlider(Rectangle{(float)controlX, (float)y, (float)controlWidth, 20}, "20",
              "80", &game.settings.hexSize, 20, 80);
    std::string hexSizeStr = std::to_string((int)game.settings.hexSize);
    DrawText(hexSizeStr.c_str(), controlX + controlWidth + 15, y, 20, GRAY);
  }
  y += 50;

  // Pan Speed Slider
  DrawText("Camera Pan Speed:", labelX, y, 20, WHITE);
  if (!game.settings.resolutionDropdownEdit && !game.settings.fpsDropdownEdit) {
    GuiSlider(Rectangle{(float)controlX, (float)y, (float)controlWidth, 20}, "1",
              "20", &game.settings.panSpeed, 1, 20);
    std::string panSpeedStr = std::to_string((int)game.settings.panSpeed);
    DrawText(panSpeedStr.c_str(), controlX + controlWidth + 15, y, 20, GRAY);
  }
  y += 60;

  // Buttons
  int buttonY = menuY + menuHeight - 70;
  if (GuiButton(Rectangle{(float)menuX + 30, (float)buttonY, 150, 40},
                "Apply")) {
    // Close any open dropdowns
    game.settings.resolutionDropdownEdit = false;
    game.settings.fpsDropdownEdit = false;

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
    HEX_WIDTH = HEX_SIZE * 2.0f;
    HEX_HEIGHT = sqrtf(3.0f) * HEX_SIZE;

    game.showOptionsMenu = false;
  }

  if (GuiButton(Rectangle{(float)menuX + 220, (float)buttonY, 150, 40},
                "Cancel")) {
    // Close any open dropdowns
    game.settings.resolutionDropdownEdit = false;
    game.settings.fpsDropdownEdit = false;
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
  }

  // Draw dropdowns last so they appear on top
  // Resolution dropdown
  std::string resLabels;
  for (int i = 0; i < RESOLUTION_COUNT; i++) {
    if (i > 0)
      resLabels += ";";
    resLabels += RESOLUTIONS[i].label;
  }
  if (GuiDropdownBox(
          Rectangle{(float)controlX, (float)resolutionY - 5, (float)controlWidth, 30},
          resLabels.c_str(), &game.settings.resolutionIndex,
          game.settings.resolutionDropdownEdit)) {
    game.settings.resolutionDropdownEdit =
        !game.settings.resolutionDropdownEdit;
  }

  // FPS Target dropdown
  if (GuiDropdownBox(
          Rectangle{(float)controlX, (float)fpsY - 5, (float)controlWidth, 30},
          FPS_LABELS, &game.settings.fpsIndex, game.settings.fpsDropdownEdit)) {
    game.settings.fpsDropdownEdit = !game.settings.fpsDropdownEdit;
  }

  // Restart warning
  if (needsRestart) {
    DrawText("Note: MSAA requires restart to take effect", menuX + 30,
             menuY + menuHeight - 25, 14, YELLOW);
  }
}

int main() {
  // Set config flags before window creation
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
             "Panzer General 2 Prototype - Raylib + RayGUI");
  SetTargetFPS(60);

  // Disable ESC key to exit - we use it for menu control
  SetExitKey(KEY_NULL);

  GameState game;

  // Add some initial units
  game.addUnit(UnitClass::INFANTRY, 0, 2, 2);
  game.addUnit(UnitClass::TANK, 0, 2, 3);
  game.addUnit(UnitClass::ARTILLERY, 0, 1, 2);

  game.addUnit(UnitClass::INFANTRY, 1, 8, 10);
  game.addUnit(UnitClass::TANK, 1, 9, 10);
  game.addUnit(UnitClass::RECON, 1, 8, 11);

  float cameraOffsetX = 100.0f;
  float cameraOffsetY = 100.0f;

  bool needsRestart = false;

  while (!WindowShouldClose()) {
    // Input handling (only when menu is closed)
    if (!game.showOptionsMenu) {
      if (IsKeyPressed(KEY_SPACE)) {
        endTurn(game);
      }

      if (IsKeyPressed(KEY_ESCAPE)) {
        game.showOptionsMenu = true;
      }

      // Mouse input
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        HexCoord clickedHex =
            pixelToHex(mousePos.x, mousePos.y, cameraOffsetX, cameraOffsetY);

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

      // Camera panning with arrow keys
      float panSpeed = game.settings.panSpeed;
      if (IsKeyDown(KEY_LEFT))
        cameraOffsetX += panSpeed;
      if (IsKeyDown(KEY_RIGHT))
        cameraOffsetX -= panSpeed;
      if (IsKeyDown(KEY_UP))
        cameraOffsetY += panSpeed;
      if (IsKeyDown(KEY_DOWN))
        cameraOffsetY -= panSpeed;
    } else {
      // Close menu with ESC (close dropdowns first if open)
      if (IsKeyPressed(KEY_ESCAPE)) {
        if (game.settings.resolutionDropdownEdit ||
            game.settings.fpsDropdownEdit) {
          // Close any open dropdowns first
          game.settings.resolutionDropdownEdit = false;
          game.settings.fpsDropdownEdit = false;
        } else {
          // Close the menu
          game.showOptionsMenu = false;
        }
      }
    }

    // Drawing
    BeginDrawing();
    ClearBackground(Color{30, 30, 30, 255});

    drawMap(game, cameraOffsetX, cameraOffsetY);
    drawUI(game);

    // Draw options menu on top
    if (game.showOptionsMenu) {
      drawOptionsMenu(game, needsRestart);
    }

    DrawFPS(SCREEN_WIDTH - 80, 10);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
