#include "GameLogic.hpp"
#include "Constants.hpp"
#include <algorithm>
#include <string>

// Forward declaration for Rendering function
namespace rendering {
  void clearSelectionHighlights(GameState& game);
}

namespace gamelogic {

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

void setSpotRangeAtPosition(GameState &game, int side, int spotRange, const HexCoord &pos, bool on) {
  std::vector<HexCoord> cells = getCellsInRange(pos.row, pos.col, spotRange);

  for (const auto& cell : cells) {
    GameHex& hex = game.map[cell.row][cell.col];
    hex.setSpotted(side, on);
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
// TURN MANAGEMENT
// ============================================================================

void endTurn(GameState &game) {
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
  rendering::clearSelectionHighlights(game);

  // Clear attack lines when ending turn
  game.attackLines.clear();
  game.showAttackLines = false;
}

} // namespace gamelogic
