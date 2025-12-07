#ifndef OPENWANZER_RENDERING_HPP
#define OPENWANZER_RENDERING_HPP

#include "GameState.hpp"
#include "Hex.hpp"
#include "Raylib.hpp"

namespace rendering {

// ============================================================================
// HEX COORDINATE CONVERSIONS (hex_drawing.cpp)
// ============================================================================

Layout createHexLayout(float hexSize, float offsetX, float offsetY, float zoom);
OffsetCoord gameCoordToOffset(const HexCoord& coord);
HexCoord offsetToGameCoord(const OffsetCoord& offset);

// ============================================================================
// HEX GEOMETRY & MAP RENDERING (hex_drawing.cpp)
// ============================================================================

void drawHexagon(const std::vector<Point>& corners, Color color, bool filled);
Color getTerrainColor(TerrainType terrain);
Color getUnitColor(int side);
std::string getUnitSymbol(UnitClass unitClass);

void drawMap(GameState& game);
void clearSelectionHighlights(GameState& game);

// ============================================================================
// UI & MENU RENDERING (ui_drawing.cpp)
// ============================================================================

void drawCombatLog(GameState& game);
void drawUnitInfoBox(GameState& game);
void drawUI(GameState& game);
void drawOptionsMenu(GameState& game, bool& needsRestart);

// ============================================================================
// COMBAT VISUALS (combat_visuals.cpp)
// ============================================================================

void drawTargetArcRing(GameState& game, Unit* unit);
void drawAttackerFiringCone(GameState& game);
void drawAttackLines(GameState& game);
void drawCombatTexts(GameState& game);

} // namespace rendering

#endif // OPENWANZER_RENDERING_HPP
