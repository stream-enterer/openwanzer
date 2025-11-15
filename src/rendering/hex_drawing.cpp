#include <cmath>
#include <string>
#include <vector>
#include "../core/constants.h"
#include "../game_logic/game_logic.h"
#include "hex.h"
#include "raylib.h"
#include "raymath.h"
#include "rendering.h"

namespace Rendering {

// ============================================================================
// HEX LAYOUT AND COORDINATE CONVERSION
// ============================================================================

Layout createHexLayout(float hexSize, float offsetX, float offsetY, float zoom) {
	Point size(hexSize * zoom, hexSize * zoom);
	Point origin(offsetX, offsetY);
	return Layout(layout_pointy, size, origin);
}

// Convert our game's row/col to hex library's offset coordinates
OffsetCoord gameCoordToOffset(const HexCoord &coord) {
	return OffsetCoord(coord.col, coord.row);
}

// Convert hex library's offset coordinates to our game's row/col
HexCoord offsetToGameCoord(const OffsetCoord &offset) {
	return HexCoord {offset.row, offset.col};
}

// ============================================================================
// HEX DRAWING
// ============================================================================

// Draw a hexagon using raylib
void drawHexagon(const std::vector<Point> &corners, Color color, bool filled) {
	if (filled) {
		// Draw filled hexagon using triangles from center
		Vector2 center = {0, 0};
		for (const auto &corner : corners) {
			center.x += corner.x;
			center.y += corner.y;
		}
		center.x /= corners.size();
		center.y /= corners.size();

		for (size_t i = 0; i < corners.size(); i++) {
			size_t next = (i + 1) % corners.size();
			DrawTriangle(
			    Vector2 {(float)corners[next].x, (float)corners[next].y},
			    Vector2 {(float)corners[i].x, (float)corners[i].y},
			    center,
			    color);
		}
	} else {
		// Draw hexagon outline
		for (size_t i = 0; i < corners.size(); i++) {
			size_t next = (i + 1) % corners.size();
			DrawLineEx(
			    Vector2 {(float)corners[i].x, (float)corners[i].y},
			    Vector2 {(float)corners[next].x, (float)corners[next].y},
			    2.0f, color);
		}
	}
}

// ============================================================================
// COLORS AND SYMBOLS
// ============================================================================

Color getTerrainColor(TerrainType terrain) {
	switch (terrain) {
		case TerrainType::PLAINS:
			return Color {144, 186, 96, 255}; // Light green grass
		case TerrainType::FOREST:
			return Color {34, 102, 34, 255}; // Dark green woods
		case TerrainType::MOUNTAIN:
			return Color {120, 100, 80, 255}; // Gray-brown peaks
		case TerrainType::HILL:
			return Color {160, 140, 100, 255}; // Tan hills
		case TerrainType::DESERT:
			return Color {220, 200, 140, 255}; // Sandy yellow
		case TerrainType::SWAMP:
			return Color {100, 120, 80, 255}; // Murky green-brown
		case TerrainType::CITY:
			return Color {140, 140, 140, 255}; // Gray urban
		case TerrainType::WATER:
			return Color {80, 140, 200, 255}; // Blue water
		case TerrainType::ROAD:
			return Color {100, 100, 100, 255}; // Dark gray pavement
		case TerrainType::ROUGH:
			return Color {130, 110, 90, 255}; // Brown rocky
		default:
			return GRAY;
	}
}

Color getUnitColor(int side) {
	return side == 0 ? Color {200, 0, 0, 255} : Color {0, 0, 200, 255};
}

std::string getUnitSymbol(UnitClass uClass) {
	switch (uClass) {
		case UnitClass::LIGHT:
			return "LT";
		case UnitClass::MEDIUM:
			return "MD";
		case UnitClass::HEAVY:
			return "HV";
		case UnitClass::ASSAULT:
			return "AS";
		default:
			return "???";
	}
}

// ============================================================================
// MAIN MAP RENDERING
// ============================================================================

void drawMap(GameState &game) {
	Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
	                                game.camera.offsetY, game.camera.zoom);

	// Draw hexes (all hexes always visible)
	for (int row = 0; row < MAP_ROWS; row++) {
		for (int col = 0; col < MAP_COLS; col++) {
			GameHex &hex = game.map[row][col];

			OffsetCoord offset = gameCoordToOffset(hex.coord);
			::Hex cubeHex = offset_to_cube(offset);

			std::vector<Point> corners = polygon_corners(layout, cubeHex);

			// Draw terrain
			Color terrainColor = getTerrainColor(hex.terrain);
			drawHexagon(corners, terrainColor, true);

			// Draw hex outline
			drawHexagon(corners, COLOR_GRID, false);

			// Draw movement/attack selection highlights
			if (hex.isMoveSel) {
				std::vector<Point> innerCorners;
				Point center = hex_to_pixel(layout, cubeHex);
				for (int i = 0; i < 6; i++) {
					Point offset = hex_corner_offset(layout, i);
					float scale = 0.85f;
					innerCorners.push_back(Point(center.x + offset.x * scale,
					                             center.y + offset.y * scale));
				}
				drawHexagon(innerCorners, Color {0, 255, 0, 100}, true);
			}
			if (hex.isAttackSel) {
				std::vector<Point> innerCorners;
				Point center = hex_to_pixel(layout, cubeHex);
				for (int i = 0; i < 6; i++) {
					Point offset = hex_corner_offset(layout, i);
					float scale = 0.85f;
					innerCorners.push_back(Point(center.x + offset.x * scale,
					                             center.y + offset.y * scale));
				}
				drawHexagon(innerCorners, Color {255, 0, 0, 100}, true);
			}
		}
	}

	// Draw units (friendly units always visible, enemy units only if spotted)
	for (auto &unit : game.units) {
		GameHex &unitHex = game.map[unit->position.row][unit->position.col];

		// Hide enemy units that aren't spotted (FOG OF WAR)
		if (unit->side != game.currentPlayer && !unitHex.isSpotted(game.currentPlayer))
			continue;

		OffsetCoord offset = gameCoordToOffset(unit->position);
		::Hex cubeHex = offset_to_cube(offset);
		Point center = hex_to_pixel(layout, cubeHex);

		float unitWidth = 40 * game.camera.zoom;
		float unitHeight = 30 * game.camera.zoom;

		// Use facing angle directly (0-360 degrees)
		float rotation = unit->facing;

		// Draw unit rectangle with rotation
		Color unitColor = getUnitColor(unit->side);
		Rectangle unitRect = {(float)center.x, (float)center.y, unitWidth, unitHeight};
		Vector2 origin = {unitWidth / 2, unitHeight / 2};
		DrawRectanglePro(unitRect, origin, rotation, unitColor);

		// Draw unit symbol (rotated with unit)
		std::string symbol = getUnitSymbol(unit->unitClass);
		int fontSize = (int)(10 * game.camera.zoom);
		if (fontSize >= 8) { // Only draw text if it's readable
			int textWidth = MeasureText(symbol.c_str(), fontSize);

			// Note: Text is drawn unrotated at center for readability
			// TODO: When using sprite textures, use DrawTexturePro with rotation parameter
			// Example: DrawTexturePro(texture, sourceRec, destRec, origin, rotation, WHITE);
			DrawText(symbol.c_str(),
			         (int)(center.x - textWidth / 2),
			         (int)(center.y - fontSize / 2 - 5),
			         fontSize, WHITE);

			// Draw health percentage
			std::string health = std::to_string(unit->getOverallHealthPercent()) + "%";
			fontSize = (int)(12 * game.camera.zoom);
			textWidth = MeasureText(health.c_str(), fontSize);
			DrawText(health.c_str(),
			         (int)(center.x - textWidth / 2),
			         (int)(center.y + 5 * game.camera.zoom),
			         fontSize, YELLOW);
		}

	}

	// Draw movement zone outline (yellow contiguous border)
	if (game.selectedUnit && !game.selectedUnit->hasMoved) {
		Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
		                                game.camera.offsetY, game.camera.zoom);

		// Find edge hexes (hexes with at least one neighbor that's not moveable)
		for (int row = 0; row < MAP_ROWS; row++) {
			for (int col = 0; col < MAP_COLS; col++) {
				if (!game.map[row][col].isMoveSel)
					continue;

				HexCoord coord = {row, col};
				OffsetCoord offset = gameCoordToOffset(coord);
				::Hex cubeHex = offset_to_cube(offset);

				// Check each of the 6 edges
				for (int dir = 0; dir < 6; dir++) {
					::Hex neighbor = hex_neighbor(cubeHex, dir);
					OffsetCoord neighborOffset = cube_to_offset(neighbor);

					// CRITICAL: Convert offset coordinates back to game coordinates before map lookup
					HexCoord neighborCoord = offsetToGameCoord(neighborOffset);

					bool drawEdge = false;

					// Draw edge if neighbor is out of bounds or not in movement range
					if (neighborCoord.row < 0 || neighborCoord.row >= MAP_ROWS || neighborCoord.col < 0 || neighborCoord.col >= MAP_COLS) {
						drawEdge = true;
					} else if (!game.map[neighborCoord.row][neighborCoord.col].isMoveSel) {
						drawEdge = true;
					}

					if (drawEdge) {
						// Draw the edge between this hex and its neighbor
						// CRITICAL: Direction numbering doesn't match edge numbering!
						// For pointy-top hexes, direction → edge mapping is: dir → (5 - dir)
						// Direction 0 (E) uses edge 5, Direction 1 (SE) uses edge 4, etc.
						std::vector<Point> corners = polygon_corners(layout, cubeHex);
						int edgeIndex = (5 - dir + 6) % 6; // Correct edge for this direction
						Point p1 = corners[edgeIndex];
						Point p2 = corners[(edgeIndex + 1) % 6];
						DrawLineEx(Vector2 {(float)p1.x, (float)p1.y},
						           Vector2 {(float)p2.x, (float)p2.y},
						           3.0f * game.camera.zoom, YELLOW);
					}
				}
			}
		}
	}

	// Draw path preview (semi-transparent snake showing planned path)
	// Only show in Phase 1 (before moving)
	if (game.selectedUnit && !game.movementSel.isFacingSelection && !game.selectedUnit->hasMoved) {
		Vector2 mousePos = GetMousePosition();
		Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
		                                game.camera.offsetY, game.camera.zoom);
		Point mousePoint(mousePos.x, mousePos.y);
		FractionalHex fracHex = pixel_to_hex(layout, mousePoint);
		::Hex cubeHex = hex_round(fracHex);
		OffsetCoord offset = cube_to_offset(cubeHex);
		HexCoord hoveredHex = offsetToGameCoord(offset);

		// Only show path if hovering over a valid movement hex
		if (hoveredHex.row >= 0 && hoveredHex.row < MAP_ROWS && hoveredHex.col >= 0 && hoveredHex.col < MAP_COLS && game.map[hoveredHex.row][hoveredHex.col].isMoveSel) {
			// Get path from unit position to hovered hex
			std::vector<HexCoord> path = GameLogic::findPath(game, game.selectedUnit,
			                                                 game.selectedUnit->position,
			                                                 hoveredHex);

			if (!path.empty() && path.size() > 1) {
				// Draw path as semi-transparent hexes
				for (size_t i = 1; i < path.size(); i++) { // Start at 1 to skip unit's current position
					OffsetCoord pathOffset = gameCoordToOffset(path[i]);
					::Hex pathCube = offset_to_cube(pathOffset);
					std::vector<Point> corners = polygon_corners(layout, pathCube);

					// Draw semi-transparent yellow fill
					drawHexagon(corners, Color {255, 255, 0, 80}, true);
				}

				// Draw target hex with slightly more opacity
				OffsetCoord targetOffset = gameCoordToOffset(hoveredHex);
				::Hex targetCube = offset_to_cube(targetOffset);
				std::vector<Point> corners = polygon_corners(layout, targetCube);
				drawHexagon(corners, Color {255, 255, 0, 120}, true);
			}
		}
	}

	// Draw combat visualization: attack lines (colored by arc)
	drawAttackLines(game);

	// Draw target arc ring for selected unit (when not in facing selection mode)
	if (game.selectedUnit && !game.movementSel.isFacingSelection) {
		drawTargetArcRing(game, game.selectedUnit);
	}

	// Draw attacker firing cone during facing selection
	drawAttackerFiringCone(game);
}

void clearSelectionHighlights(GameState &game) {
	for (int row = 0; row < MAP_ROWS; row++) {
		for (int col = 0; col < MAP_COLS; col++) {
			game.map[row][col].isMoveSel = false;
			game.map[row][col].isAttackSel = false;
		}
	}
}

} // namespace Rendering
