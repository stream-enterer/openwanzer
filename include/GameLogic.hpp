#ifndef OPENWANZER_GAME_LOGIC_HPP
#define OPENWANZER_GAME_LOGIC_HPP

#include <string>
#include <vector>
#include "hex.h"
#include "Enums.hpp"
#include "HexCoord.hpp"
#include "GameHex.hpp"
#include "Unit.hpp"
#include "GameState.hpp"

namespace GameLogic {

// ============================================================================
// UTILITY FUNCTIONS (utilities.cpp)
// ============================================================================

// Terrain utilities
int getTerrainIndex(TerrainType terrain);
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

void moveUnit(GameState& game, Unit* unit, const HexCoord& target, bool updateSpotting = true);

// ============================================================================
// COMBAT (combat.cpp)
// ============================================================================

int calculateKills(int atkVal, int defVal, const Unit* attacker, const Unit* defender);

void performAttack(GameState& game, Unit* attacker, Unit* defender);

// ============================================================================
// SYSTEMS (systems.cpp)
// ============================================================================

// Fog of War / Spotting
void setUnitSpotRange(GameState& game, Unit* unit, bool on);
void setSpotRangeAtPosition(GameState& game, int side, int spotRange, const HexCoord& pos, bool on);
void initializeAllSpotting(GameState& game);

// Turn management
void endTurn(GameState& game);

// ============================================================================
// ATTACK VISUALIZATION (attack_lines.cpp)
// ============================================================================

void updateAttackLines(GameState& game);

} // namespace GameLogic

#endif // OPENWANZER_GAME_LOGIC_HPP
