#include "game_logic.h"
#include "../core/constants.h"
#include <algorithm>
#include <string>

// Forward declaration for Rendering function
namespace Rendering {
  void clearSelectionHighlights(GameState& game);
}

namespace GameLogic {

// ============================================================================
// ZONE OF CONTROL
// ============================================================================

void setUnitZOC(GameState &game, Unit *unit, bool on) {
  if (!unit || isAir(unit)) return;

  std::vector<HexCoord> adjacent = getAdjacent(unit->position.row, unit->position.col);

  for (const auto& adj : adjacent) {
    game.map[adj.row][adj.col].setZOC(unit->side, on);
  }
}

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

// ============================================================================
// FOG OF WAR / SPOTTING
// ============================================================================

void setUnitSpotRange(GameState &game, Unit *unit, bool on) {
  if (!unit) return;

  HexCoord pos = unit->position;
  int range = unit->spotRange;
  std::vector<HexCoord> cells = getCellsInRange(pos.row, pos.col, range);

  for (const auto& cell : cells) {
    GameHex& hex = game.map[cell.row][cell.col];
    hex.setSpotted(unit->side, on);
  }
}

void initializeAllSpotting(GameState &game) {
  // Clear all spotting first
  for (int row = 0; row < MAP_ROWS; row++) {
    for (int col = 0; col < MAP_COLS; col++) {
      game.map[row][col].spotted[0] = 0;
      game.map[row][col].spotted[1] = 0;
    }
  }

  // Set spotting for all units
  for (auto &unit : game.units) {
    setUnitSpotRange(game, unit.get(), true);
  }
}

// ============================================================================
// ENTRENCHMENT
// ============================================================================

void entrenchUnit(GameState &game, Unit *unit) {
  if (!unit) return;

  GameHex &hex = game.map[unit->position.row][unit->position.col];
  int terrainEntrench = getTerrainEntrenchment(hex.terrain);
  int uc = static_cast<int>(unit->unitClass);

  if (unit->entrenchment >= terrainEntrench) {
    // Slow gain above terrain level (ticks system)
    int level = unit->entrenchment - terrainEntrench;
    int nextThreshold = 9 * level + 4;
    int expBars = unit->experience / 100;

    // Add ticks based on experience, terrain, and unit type
    unit->entrenchTicks += expBars + (terrainEntrench + 1) * UNIT_ENTRENCH_RATE[uc];

    // Check if we've gained a level
    while (unit->entrenchTicks >= nextThreshold &&
           unit->entrenchment < terrainEntrench + 5) {
      unit->entrenchTicks -= nextThreshold;
      unit->entrenchment++;
      level++;
      nextThreshold = 9 * level + 4;
    }
  } else {
    // Instant gain to terrain level
    unit->entrenchment = terrainEntrench;
    unit->entrenchTicks = 0;
  }

  // Max entrenchment is 5
  unit->entrenchment = std::min(5, unit->entrenchment);
}

// ============================================================================
// TURN MANAGEMENT
// ============================================================================

void endTurn(GameState &game) {
  // Process units ending their turn
  for (auto &unit : game.units) {
    if (unit->side == game.currentPlayer) {
      // Gain entrenchment if didn't move
      if (!unit->hasMoved) {
        entrenchUnit(game, unit.get());
      } else {
        // Lost entrenchment by moving
        unit->entrenchment = 0;
        unit->entrenchTicks = 0;
      }

      // Reset hits at end of turn
      unit->hits = 0;
    }
  }

  // Log turn end
  std::string playerName = game.currentPlayer == 0 ? "Axis" : "Allied";
  addLogMessage(game, playerName + " turn ended");

  // Switch player
  game.currentPlayer = 1 - game.currentPlayer;

  // If both players have moved, advance turn
  if (game.currentPlayer == 0) {
    game.currentTurn++;
    addLogMessage(game, "--- Turn " + std::to_string(game.currentTurn) + " ---");
  }

  // Log new player turn
  playerName = game.currentPlayer == 0 ? "Axis" : "Allied";
  addLogMessage(game, playerName + " turn begins");

  // Reset actions for units about to start their turn
  for (auto &unit : game.units) {
    if (unit->side == game.currentPlayer) {
      unit->hasMoved = false;
      unit->hasFired = false;
      unit->movesLeft = unit->movementPoints;
    }
  }

  // Clear selection and movement state
  game.selectedUnit = nullptr;
  game.movementSel.reset();
  Rendering::clearSelectionHighlights(game);
}

} // namespace GameLogic
