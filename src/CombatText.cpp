#include "Constants.hpp"
#include "GameLogic.hpp"
#include "Hex.hpp"
#include "Rendering.hpp"

#include <algorithm>
#include <cstdlib>

namespace gamelogic {

void spawnCombatText(GameState& game, const HexCoord& targetHex, const std::string& text, bool isStructure) {
	// Calculate random offset in normalized hex-relative coordinates
	// These offsets will be scaled by hex size when calculating screen position
	float randomX = (float)(std::rand() % 1000) / 1000.0f; // 0.0 to 1.0
	float randomY = (float)(std::rand() % 1000) / 1000.0f; // 0.0 to 1.0

	// Offset relative to hex center in hex-size units:
	// Horizontal: -0.75 to +0.75 hex widths (spans into adjacent hexes)
	// Vertical: 0 to -0.5 hex heights (upper half only)
	Vector2 hexOffset = {
	    (randomX - 0.5f) * 1.5f, // Range: -0.75 to +0.75
	    -randomY * 0.5f          // Range: 0 to -0.5 (negative is up)
	};

	// Use config timing values
	float fadeIn = game.settings.combatTextFadeInTime;
	float floatDur = game.settings.combatTextFloatTime;
	float floatSpeed = game.settings.combatTextFloatSpeed;

	game.combatTexts.emplace_back(targetHex, hexOffset, text, isStructure, fadeIn, floatDur, floatSpeed);
}

void updateCombatTexts(GameState& game, float deltaTime) {
	// Create layout with current camera state
	Layout layout = rendering::createHexLayout(HEX_SIZE, game.camera.offsetX,
	                                           game.camera.offsetY, game.camera.zoom);

	// Update all combat texts
	for (auto& ct : game.combatTexts) {
		ct.update(deltaTime);

		// Recalculate screen position from hex + offset + camera state
		OffsetCoord offset = rendering::gameCoordToOffset(ct.spawnHex);
		::Hex hexCube = OffsetToCube(offset);
		Point center = HexToPixel(layout, hexCube);

		// Calculate scaled offset based on current zoom
		float hexWidth = HEX_SIZE * game.camera.zoom * 2.0f;
		float hexHeight = HEX_SIZE * game.camera.zoom * 1.732f;

		// Apply hex-relative offset scaled by hex dimensions
		ct.position.x = (float)center.x + ct.hexOffset.x * hexWidth;
		ct.position.y = (float)center.y + ct.hexOffset.y * hexHeight - ct.floatOffset;
	}

	// Remove finished combat texts
	game.combatTexts.erase(
	    std::remove_if(game.combatTexts.begin(), game.combatTexts.end(),
	                   [](const CombatText& ct) { return ct.isFinished(); }),
	    game.combatTexts.end());
}

} // namespace gamelogic
