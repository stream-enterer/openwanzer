#include "Constants.hpp"
#include "GameLogic.hpp"
#include "Hex.hpp"
#include "Rendering.hpp"

#include <algorithm>
#include <cstdlib>

namespace gamelogic {

void spawnCombatText(GameState& game, const HexCoord& targetHex, const std::string& text, bool isStructure) {
	// Get pixel position of target hex
	Layout layout = rendering::createHexLayout(HEX_SIZE, game.camera.offsetX,
	                                           game.camera.offsetY, game.camera.zoom);

	OffsetCoord offset = rendering::gameCoordToOffset(targetHex);
	::Hex hexCube = OffsetToCube(offset);
	Point center = HexToPixel(layout, hexCube);

	// Calculate hex dimensions for positioning
	float hexWidth = HEX_SIZE * game.camera.zoom * 2.0f;    // Full width of hex
	float hexHeight = HEX_SIZE * game.camera.zoom * 1.732f; // Approximate height

	// Random position within upper half of hex
	// Horizontal bounds extend halfway into adjacent hexes (1.5x hex width total)
	float horizontalRange = hexWidth * 1.5f;
	float verticalRange = hexHeight * 0.5f; // Upper half only

	float randomX = (float)(std::rand() % 1000) / 1000.0f; // 0.0 to 1.0
	float randomY = (float)(std::rand() % 1000) / 1000.0f; // 0.0 to 1.0

	float posX = (float)center.x + (randomX - 0.5f) * horizontalRange;
	float posY = (float)center.y - (randomY * verticalRange); // Upper half (negative Y is up)

	Vector2 position = {posX, posY};

	// Use config timing values
	float fadeIn = game.settings.combatTextFadeInTime;
	float floatDur = game.settings.combatTextFloatTime;
	float floatSpeed = game.settings.combatTextFloatSpeed;

	game.combatTexts.emplace_back(position, text, isStructure, fadeIn, floatDur, floatSpeed);
}

void updateCombatTexts(GameState& game, float deltaTime) {
	// Update all combat texts and remove finished ones
	for (auto& text : game.combatTexts) {
		text.update(deltaTime);
	}

	// Remove finished combat texts
	game.combatTexts.erase(
	    std::remove_if(game.combatTexts.begin(), game.combatTexts.end(),
	                   [](const CombatText& ct) { return ct.isFinished(); }),
	    game.combatTexts.end());
}

} // namespace gamelogic
