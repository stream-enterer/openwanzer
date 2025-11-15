#include "game_logic.h"
#include "../core/constants.h"
#include <algorithm>
#include <string>

// Forward declaration for Rendering function
namespace Rendering {
  void clearSelectionHighlights(GameState& game);
}

namespace GameLogic {

// BFS pathfinding - returns path from start to goal
std::vector<HexCoord> findPath(GameState &game, Unit *unit, const HexCoord &start, const HexCoord &goal) {
  if (!unit) return {};
  if (start == goal) return {start};

  int movMethodIdx = static_cast<int>(unit->movMethod);

  // BFS with parent tracking
  struct PathNode {
    HexCoord coord;
    int movementUsed;  // Total movement used to reach this node
    HexCoord parent;
    bool hasParent;
  };

  std::vector<PathNode> queue;
  std::vector<PathNode> visited;

  // Start node
  queue.push_back({start, 0, {-1, -1}, false});
  visited.push_back({start, 0, {-1, -1}, false});

  while (!queue.empty()) {
    PathNode current = queue.front();
    queue.erase(queue.begin());  // Remove first element

    // Check if we reached the goal
    if (current.coord == goal) {
      // Reconstruct path
      std::vector<HexCoord> path;
      HexCoord c = goal;

      while (true) {
        path.push_back(c);
        bool found = false;
        for (const auto& v : visited) {
          if (v.coord == c && v.hasParent) {
            c = v.parent;
            found = true;
            break;
          }
        }
        if (!found) break;
        if (c == start) {
          path.push_back(start);
          break;
        }
      }

      // Reverse path so it goes from start to goal
      std::reverse(path.begin(), path.end());
      return path;
    }

    // Explore neighbors
    std::vector<HexCoord> adjacent = getAdjacent(current.coord.row, current.coord.col);

    for (const auto& adj : adjacent) {
      // Get terrain cost
      GameHex& hex = game.map[adj.row][adj.col];
      int terrainIdx = getTerrainIndex(hex.terrain);
      int cost = MOV_TABLE_DRY[movMethodIdx][terrainIdx];

      // Skip impassable terrain
      if (cost >= 255) continue;

      int newMovementUsed = current.movementUsed + cost;

      // For cost 254, it stops movement
      if (cost == 254) newMovementUsed = 999;  // Very high cost

      // Check if we can afford this movement
      if (newMovementUsed > unit->movesLeft * 2) continue;  // Allow some extra for pathfinding flexibility

      // Check if another unit occupies this hex
      // Allow only if it's the goal and it's an enemy (for attacking)
      Unit *occupant = game.getUnitAt(adj);
      if (occupant && !(adj == goal && occupant->side != unit->side)) continue;

      // Check if we've already visited this with lower cost
      bool alreadyVisited = false;
      for (const auto& v : visited) {
        if (v.coord == adj && v.movementUsed <= newMovementUsed) {
          alreadyVisited = true;
          break;
        }
      }

      if (!alreadyVisited) {
        PathNode newNode = {adj, newMovementUsed, current.coord, true};
        queue.push_back(newNode);
        visited.push_back(newNode);
      }
    }
  }

  // No path found
  return {};
}

void highlightMovementRange(GameState &game, Unit *unit) {
  Rendering::clearSelectionHighlights(game);
  if (!unit)
    return;

  int maxRange = unit->movesLeft;
  int movMethodIdx = static_cast<int>(unit->movMethod);

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

      // Skip if we can't afford to enter
      if (newRemaining < 0) continue;

      // Check if another unit occupies this hex (block all units, not just enemies)
      Unit *occupant = game.getUnitAt(adj);
      if (occupant) continue;

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

  // Highlight all reachable cells (including starting position)
  for (const auto& v : visited) {
    game.map[v.first.row][v.first.col].isMoveSel = true;
  }
}

void highlightAttackRange(GameState &game, Unit *unit) {
  if (!unit || unit->hasFired)
    return;

  int range = 1; // Default attack range

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

void moveUnit(GameState &game, Unit *unit, const HexCoord &target, bool updateSpotting) {
  if (!unit)
    return;

  // Calculate actual movement cost based on terrain
  int movMethodIdx = static_cast<int>(unit->movMethod);
  GameHex& targetHex = game.map[target.row][target.col];
  int terrainIdx = getTerrainIndex(targetHex.terrain);
  int cost = MOV_TABLE_DRY[movMethodIdx][terrainIdx];

  // Don't move if impassable
  if (cost >= 255) {
    addLogMessage(game, "Terrain is impassable");
    return;
  }

  // For difficult terrain (cost 254), we can enter but it uses all remaining moves
  if (cost == 254) cost = unit->movesLeft;

  // Only move if we have enough movement points
  if (cost <= unit->movesLeft) {
    // Clear spotting from old position
    if (updateSpotting) {
      setUnitSpotRange(game, unit, false);
    }

    // Move unit
    unit->position = target;
    unit->movesLeft = 0;  // One move per turn - all movement used up
    unit->hasMoved = true;

    // Set spotting at new position
    if (updateSpotting) {
      setUnitSpotRange(game, unit, true);
    }

    // Log movement
    std::string unitName = unit->name + " (" + (unit->side == 0 ? "Axis" : "Allied") + ")";
    addLogMessage(game, unitName + " moves to (" + std::to_string(target.row) + "," + std::to_string(target.col) + ")");
  } else {
    addLogMessage(game, "Not enough movement points");
  }
}

} // namespace GameLogic
