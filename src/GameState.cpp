#include "GameState.hpp"
#include "Constants.hpp"
#include "rl/raylib.h"

// Resolution options
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

// GameLayout implementation
void GameLayout::recalculate(int w, int h) {
  statusBar = {0, 0, (float)w, 40};
  unitPanel = {(float)w - 250, 50, 250, 300};
  helpBar = {0, (float)h - 30, (float)w, 30};
  playArea = {0, 40, (float)w - 250, (float)h - 70};
}

GameLayout::GameLayout() {
  recalculate(SCREEN_WIDTH, SCREEN_HEIGHT);
}

// CombatLog implementation
CombatLog::CombatLog()
    : scrollOffset(0.0f), maxScrollOffset(0.0f), isHovering(false),
      isDragging(false), dragOffset{0, 0} {
  // Lower right corner, 350px wide, 400px tall, 10px margin from edges
  bounds = {SCREEN_WIDTH - 360.0f, SCREEN_HEIGHT - 410.0f, 350.0f, 400.0f};
  initialBounds = bounds;
}

void CombatLog::recalculateBounds(int screenWidth, int screenHeight) {
  // Position in lower right (preserving offset from initial if dragged)
  float defaultX = screenWidth - 360.0f;
  float defaultY = screenHeight - 410.0f;
  float offsetX = bounds.x - initialBounds.x;
  float offsetY = bounds.y - initialBounds.y;

  initialBounds = {defaultX, defaultY, 350.0f, 400.0f};
  bounds = {defaultX + offsetX, defaultY + offsetY, 350.0f, 400.0f};
}

void CombatLog::resetPosition() {
  bounds = initialBounds;
}

// UnitInfoBox implementation
UnitInfoBox::UnitInfoBox()
    : isHovering(false), isDragging(false), dragOffset{0, 0} {
  // Upper right corner, below status bar (40px), 250px wide, 300px tall
  bounds = {SCREEN_WIDTH - 250.0f, 50.0f, 250.0f, 300.0f};
  initialBounds = bounds;
}

void UnitInfoBox::recalculateBounds(int screenWidth, [[maybe_unused]] int screenHeight) {
  // Position in upper right (preserving offset from initial if dragged)
  float defaultX = screenWidth - 250.0f;
  float defaultY = 50.0f;
  float offsetX = bounds.x - initialBounds.x;
  float offsetY = bounds.y - initialBounds.y;

  initialBounds = {defaultX, defaultY, 250.0f, 300.0f};
  bounds = {defaultX + offsetX, defaultY + offsetY, 250.0f, 300.0f};
}

void UnitInfoBox::resetPosition() {
  bounds = initialBounds;
}

// MovementSelection implementation
void MovementSelection::reset() {
  isFacingSelection = false;
  oldPosition = {-1, -1};
  oldMovesLeft = 0;
  oldHasMoved = false;
  selectedFacing = 0.0f;
}

// GameState implementation
GameState::GameState()
    : selectedUnit(nullptr), currentTurn(1), currentPlayer(0), maxTurns(20),
      showOptionsMenu(false), showAttackLines(false) {
  initializeMap();
}

void GameState::initializeMap() {
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
    }
  }
}

Unit *GameState::getUnitAt(const HexCoord &coord) {
  for (auto &unit : units) {
    if (unit->position == coord) {
      return unit.get();
    }
  }
  return nullptr;
}

void GameState::addUnit(UnitClass uClass, int side, int row, int col) {
  auto unit = std::make_unique<Unit>();
  unit->unitClass = uClass;
  unit->side = side;
  unit->position = {row, col};

  // Set weight class based on unit class
  Unit::WeightClass wClass = Unit::WeightClass::MEDIUM;  // Default to medium
  switch (uClass) {
  case UnitClass::LIGHT:
    wClass = Unit::WeightClass::LIGHT;
    unit->name = "Light Mech";
    unit->movMethod = MovMethod::LEG;
    break;
  case UnitClass::MEDIUM:
    wClass = Unit::WeightClass::MEDIUM;
    unit->name = "Medium Mech";
    unit->movMethod = MovMethod::WHEELED;
    break;
  case UnitClass::HEAVY:
    wClass = Unit::WeightClass::HEAVY;
    unit->name = "Heavy Mech";
    unit->movMethod = MovMethod::HALF_TRACKED;
    break;
  case UnitClass::ASSAULT:
    wClass = Unit::WeightClass::ASSAULT;
    unit->name = "Assault Mech";
    unit->movMethod = MovMethod::TRACKED;
    break;
  }

  // Initialize armor locations for this weight class
  unit->initializeLocations(wClass);

  // Initialize weapons for this weight class
  unit->initializeWeapons();

  // Initialize unit facing based on side (using degrees: E=0°, S=90°, W=180°, N=270°)
  // Axis units (left side) face generally East (toward right/enemy)
  // Allied units (right side) face generally West (toward left/enemy)
  if (side == 0) {
    unit->facing = 0.0f;  // East (0°) - facing toward the right side of map
  } else {
    unit->facing = 180.0f;  // West (180°) - facing toward the left side of map
  }

  units.push_back(std::move(unit));
}
