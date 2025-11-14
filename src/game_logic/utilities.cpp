#include "game_logic.h"
#include "../core/constants.h"
#include "hex.h"
#include <cmath>
#include <cstdio>

namespace GameLogic {

// ============================================================================
// CONSTANTS
// ============================================================================

// Terrain entrenchment levels (max entrenchment from terrain)
const int TERRAIN_ENTRENCHMENT[10] = {
    0,  // PLAINS
    3,  // CITY
    2,  // FOREST
    2,  // MOUNTAIN
    1,  // HILL
    0,  // DESERT
    0,  // SWAMP
    3,  // (unused - would be FORTIFICATION)
    0,  // WATER
    2   // ROUGH
};

// Unit entrenchment rates (how fast they entrench)
const int UNIT_ENTRENCH_RATE[6] = {
    3,  // INFANTRY (fast)
    1,  // TANK (slow)
    2,  // ARTILLERY (medium)
    2,  // RECON (medium)
    2,  // ANTI_TANK (medium)
    1   // AIR_DEFENSE (slow)
};

// ============================================================================
// TERRAIN UTILITY FUNCTIONS
// ============================================================================

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

// Get terrain entrenchment level
int getTerrainEntrenchment(TerrainType terrain) {
  int idx = static_cast<int>(terrain);
  if (idx >= 0 && idx < 10) {
    return TERRAIN_ENTRENCHMENT[idx];
  }
  return 0;
}

// Get terrain type as display string
std::string getTerrainName(TerrainType terrain) {
  switch (terrain) {
  case TerrainType::PLAINS: return "Plains";
  case TerrainType::FOREST: return "Forest";
  case TerrainType::MOUNTAIN: return "Mountain";
  case TerrainType::HILL: return "Hill";
  case TerrainType::DESERT: return "Desert";
  case TerrainType::SWAMP: return "Swamp";
  case TerrainType::CITY: return "City";
  case TerrainType::WATER: return "Water";
  case TerrainType::ROAD: return "Road";
  case TerrainType::ROUGH: return "Rough";
  default: return "Unknown";
  }
}

// Get movement cost for a given terrain and movement method
int getMovementCost(MovMethod movMethod, TerrainType terrain) {
  int movMethodIdx = static_cast<int>(movMethod);
  int terrainIdx = getTerrainIndex(terrain);
  if (movMethodIdx >= 0 && movMethodIdx < 12 && terrainIdx >= 0 && terrainIdx < 18) {
    return MOV_TABLE_DRY[movMethodIdx][terrainIdx];
  }
  return 255; // Impassable by default
}

// ============================================================================
// FACING AND COMPASS FUNCTIONS
// ============================================================================

// Convert facing angle (0-360 degrees) to hybrid intercardinal/geometric notation
std::string getFacingName(float facing) {
  // Normalize angle to 0-360
  while (facing < 0) facing += 360.0f;
  while (facing >= 360.0f) facing -= 360.0f;

  // Determine compass direction based on 16-point compass rose
  const char* direction;
  if (facing >= 348.75f || facing < 11.25f) direction = "N";
  else if (facing >= 11.25f && facing < 33.75f) direction = "NNE";
  else if (facing >= 33.75f && facing < 56.25f) direction = "NE";
  else if (facing >= 56.25f && facing < 78.75f) direction = "ENE";
  else if (facing >= 78.75f && facing < 101.25f) direction = "E";
  else if (facing >= 101.25f && facing < 123.75f) direction = "ESE";
  else if (facing >= 123.75f && facing < 146.25f) direction = "SE";
  else if (facing >= 146.25f && facing < 168.75f) direction = "SSE";
  else if (facing >= 168.75f && facing < 191.25f) direction = "S";
  else if (facing >= 191.25f && facing < 213.75f) direction = "SSW";
  else if (facing >= 213.75f && facing < 236.25f) direction = "SW";
  else if (facing >= 236.25f && facing < 258.75f) direction = "WSW";
  else if (facing >= 258.75f && facing < 281.25f) direction = "W";
  else if (facing >= 281.25f && facing < 303.75f) direction = "WNW";
  else if (facing >= 303.75f && facing < 326.25f) direction = "NW";
  else direction = "NNW";

  // Format as "NNE (018°)"
  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%s (%03d°)", direction, (int)facing);
  return std::string(buffer);
}

// Calculate facing angle from a hex center to a point on screen
// Note: This function needs Rendering::gameCoordToOffset, so it will be
// implemented in a separate file to avoid circular dependencies
float calculateFacingFromPoint(const HexCoord &center, const Point &targetPoint, Layout &layout) {
  OffsetCoord centerOffset = OffsetCoord(center.col, center.row);  // Direct conversion
  ::Hex centerCube = offset_to_cube(centerOffset);
  Point centerPixel = hex_to_pixel(layout, centerCube);

  // Calculate direction vector from center to target
  float dx = targetPoint.x - centerPixel.x;
  float dy = targetPoint.y - centerPixel.y;

  // Calculate exact angle using atan2
  // In screen coordinates: E=0°, S=90° (y-down), W=180°, N=270°
  float angle = atan2(dy, dx) * 180.0f / M_PI;

  // Normalize angle to 0-360
  if (angle < 0) angle += 360.0f;

  return angle;
}

// ============================================================================
// UNIT CLASSIFICATION QUERIES
// ============================================================================

bool isAir(const Unit* unit) {
  return unit->movMethod == MovMethod::AIR;
}

bool isHardTarget(const Unit* unit) {
  return unit->unitClass == UnitClass::TANK ||
         unit->unitClass == UnitClass::RECON;
}

bool isSea(const Unit* unit) {
  return unit->movMethod == MovMethod::DEEP_NAVAL ||
         unit->movMethod == MovMethod::COSTAL ||
         unit->movMethod == MovMethod::NAVAL;
}

// ============================================================================
// HEX DISTANCE CALCULATION
// ============================================================================

int hexDistance(const HexCoord& a, const HexCoord& b) {
  // Convert offset coordinates to cube coordinates
  OffsetCoord offsetA = {a.col, a.row};
  OffsetCoord offsetB = {b.col, b.row};

  Hex cubeA = offset_to_cube(offsetA);
  Hex cubeB = offset_to_cube(offsetB);

  // Manhattan distance in cube space
  return (abs(cubeA.q - cubeB.q) + abs(cubeA.r - cubeB.r) + abs(cubeA.s - cubeB.s)) / 2;
}

// Get all adjacent hexes (6 neighbors)
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

// Get all cells within a given range
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

// ============================================================================
// COMBAT LOG HELPERS
// ============================================================================

void addLogMessage(GameState& game, const std::string& message) {
  // Check if last message is the same (for deduplication)
  if (!game.combatLog.messages.empty()) {
    LogMessage& last = game.combatLog.messages.back();
    if (last.turn == game.currentTurn && last.message == message) {
      last.count++;
      return;
    }
  }

  // Add new message
  game.combatLog.messages.emplace_back(game.currentTurn, message);

  // Auto-scroll to bottom (most recent) by setting scroll to max
  // We'll calculate the actual max in the rendering function
  game.combatLog.scrollOffset = 999999.0f;  // Large value to force scroll to bottom
}

} // namespace GameLogic
