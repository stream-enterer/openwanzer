#ifndef OPENWANZER_GAME_LOGIC_H
#define OPENWANZER_GAME_LOGIC_H

#include <string>
#include <vector>
#include "hex.h"
#include "../core/enums.h"
#include "../core/hex_coord.h"
#include "../core/game_hex.h"
#include "../core/unit.h"
#include "../core/game_state.h"

namespace GameLogic {

// ============================================================================
// CONSTANTS
// ============================================================================

// Terrain entrenchment levels (max entrenchment from terrain)
extern const int TERRAIN_ENTRENCHMENT[10];

// Unit entrenchment rates (how fast they entrench)
extern const int UNIT_ENTRENCH_RATE[6];

// ============================================================================
// UTILITY FUNCTIONS (utilities.cpp)
// ============================================================================

// Terrain utilities
int getTerrainIndex(TerrainType terrain);
int getTerrainEntrenchment(TerrainType terrain);
std::string getTerrainName(TerrainType terrain);
int getMovementCost(MovMethod movMethod, TerrainType terrain);

// Facing utilities
std::string getFacingName(float facing);
float calculateFacingFromPoint(const HexCoord& center, const Point& targetPoint, Layout& layout);

// Unit classification queries
bool isAir(const Unit* unit);
bool isHardTarget(const Unit* unit);
bool isSea(const Unit* unit);
bool isRecon(const Unit* unit);

// Hex math and helpers
int hexDistance(const HexCoord& a, const HexCoord& b);
std::vector<HexCoord> getAdjacent(int row, int col);
std::vector<HexCoord> getCellsInRange(int row, int col, int range);

// Combat log helpers
void addLogMessage(GameState& game, const std::string& message);

// ============================================================================
// PATHFINDING (pathfinding.cpp)
// ============================================================================

std::vector<HexCoord> findPath(GameState& game, Unit* unit,
                                const HexCoord& start, const HexCoord& goal);

void highlightMovementRange(GameState& game, Unit* unit);

void highlightAttackRange(GameState& game, Unit* unit);

void moveUnit(GameState& game, Unit* unit, const HexCoord& target);

// ============================================================================
// COMBAT (combat.cpp)
// ============================================================================

int calculateKills(int atkVal, int defVal, const Unit* attacker, const Unit* defender);

void performAttack(GameState& game, Unit* attacker, Unit* defender);

// ============================================================================
// SYSTEMS (systems.cpp)
// ============================================================================

// Zone of Control
void setUnitZOC(GameState& game, Unit* unit, bool on);
void initializeAllZOC(GameState& game);

// Fog of War / Spotting
void setUnitSpotRange(GameState& game, Unit* unit, bool on);
void initializeAllSpotting(GameState& game);

// Entrenchment
void entrenchUnit(GameState& game, Unit* unit);

// Turn management
void endTurn(GameState& game);

} // namespace GameLogic

#endif // OPENWANZER_GAME_LOGIC_H
