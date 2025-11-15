#include "game_logic.h"
#include "../core/constants.h"
#include "hex.h"
#include <cmath>
#include <cstdio>

namespace GameLogic {

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
// Internal representation uses mathematical convention (E=0°, S=90°, W=180°, N=270°)
// Display uses military compass convention (N=0°, E=90°, S=180°, W=270°)
std::string getFacingName(float facing) {
  // Normalize angle to 0-360
  while (facing < 0) facing += 360.0f;
  while (facing >= 360.0f) facing -= 360.0f;

  // Convert from mathematical convention to compass convention
  // Math: E=0°, rotates clockwise → Compass: N=0°, rotates clockwise
  float compass = 90.0f - facing;
  while (compass < 0) compass += 360.0f;
  while (compass >= 360.0f) compass -= 360.0f;

  // Determine compass direction based on 16-point compass rose
  const char* direction;
  if (compass >= 348.75f || compass < 11.25f) direction = "N";
  else if (compass >= 11.25f && compass < 33.75f) direction = "NNE";
  else if (compass >= 33.75f && compass < 56.25f) direction = "NE";
  else if (compass >= 56.25f && compass < 78.75f) direction = "ENE";
  else if (compass >= 78.75f && compass < 101.25f) direction = "E";
  else if (compass >= 101.25f && compass < 123.75f) direction = "ESE";
  else if (compass >= 123.75f && compass < 146.25f) direction = "SE";
  else if (compass >= 146.25f && compass < 168.75f) direction = "SSE";
  else if (compass >= 168.75f && compass < 191.25f) direction = "S";
  else if (compass >= 191.25f && compass < 213.75f) direction = "SSW";
  else if (compass >= 213.75f && compass < 236.25f) direction = "SW";
  else if (compass >= 236.25f && compass < 258.75f) direction = "WSW";
  else if (compass >= 258.75f && compass < 281.25f) direction = "W";
  else if (compass >= 281.25f && compass < 303.75f) direction = "WNW";
  else if (compass >= 303.75f && compass < 326.25f) direction = "NW";
  else direction = "NNW";

  // Format as "E (090°)" - display compass bearing
  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%s (%03d°)", direction, (int)compass);
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
  // All BattleTech mechs are hard targets
  return true;
}

bool isSea(const Unit* unit) {
  return unit->movMethod == MovMethod::DEEP_NAVAL ||
         unit->movMethod == MovMethod::COSTAL ||
         unit->movMethod == MovMethod::NAVAL;
}

bool isRecon(const Unit* unit) {
  // No recon units in BattleTech
  return false;
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
