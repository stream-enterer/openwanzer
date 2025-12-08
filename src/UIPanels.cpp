#include "UIPanels.hpp"
#include "Constants.hpp"

namespace uipanel {

// Panel dimensions
const int TARGET_PANEL_WIDTH = 720;
const int TARGET_PANEL_HEIGHT = 200;
const int PLAYER_PANEL_WIDTH = 320;
const int PLAYER_PANEL_HEIGHT = 200;

// Internal layout constants
const int PAPERDOLL_PADDING = 10;
const int PAPERDOLL_SECTION_MARGIN = 2;
const int FRONT_PAPERDOLL_WIDTH = 120;
const int FRONT_PAPERDOLL_HEIGHT = 140;
const int REAR_PAPERDOLL_WIDTH = 100;
const int REAR_PAPERDOLL_HEIGHT = 140;

void initializeTargetPanel(GameState& game) {
	int targetPanelX = (SCREEN_WIDTH - TARGET_PANEL_WIDTH) / 2;
	int targetPanelY = 20;

	game.targetPanel.bounds = Rectangle {
	    (float)targetPanelX,
	    (float)targetPanelY,
	    (float)TARGET_PANEL_WIDTH,
	    (float)TARGET_PANEL_HEIGHT};
	game.targetPanel.defaultPosition = Vector2 {
	    (float)targetPanelX,
	    (float)targetPanelY};
	game.targetPanel.isVisible = false;
}

void initializePlayerPanel(GameState& game) {
	int playerPanelX = 20;
	int playerPanelY = SCREEN_HEIGHT - PLAYER_PANEL_HEIGHT - 20;

	game.playerPanel.bounds = Rectangle {
	    (float)playerPanelX,
	    (float)playerPanelY,
	    (float)PLAYER_PANEL_WIDTH,
	    (float)PLAYER_PANEL_HEIGHT};
	game.playerPanel.defaultPosition = Vector2 {
	    (float)playerPanelX,
	    (float)playerPanelY};
	game.playerPanel.isVisible = false;
}

void showTargetPanel(GameState& game, Unit* target, combatarcs::AttackArc arc) {
	game.targetPanel.targetUnit = target;
	game.targetPanel.currentArc = arc;
	game.targetPanel.isVisible = true;
	calculatePaperdollRegions(game.targetPanel);
}

void showPlayerPanel(GameState& game, Unit* player) {
	game.playerPanel.playerUnit = player;
	game.playerPanel.isVisible = true;
	calculatePaperdollRegions(game.playerPanel);
}

void hideTargetPanel(GameState& game) {
	game.targetPanel.isVisible = false;
	game.targetPanel.targetUnit = nullptr;
}

void hidePlayerPanel(GameState& game) {
	game.playerPanel.isVisible = false;
	game.playerPanel.playerUnit = nullptr;
}

void resetPanelPositions(GameState& game) {
	game.targetPanel.bounds.x = game.targetPanel.defaultPosition.x;
	game.targetPanel.bounds.y = game.targetPanel.defaultPosition.y;
	game.playerPanel.bounds.x = game.playerPanel.defaultPosition.x;
	game.playerPanel.bounds.y = game.playerPanel.defaultPosition.y;
}

void calculatePaperdollRegions(PaperdollPanel& panel) {
	// Calculate precise Rectangle bounds for 5-box cross layout
	// Layout (top-down view):
	//         [FRONT]
	//     [LEFT][CENTER][RIGHT]
	//         [REAR]

	float startX = panel.bounds.x + PAPERDOLL_PADDING;
	float startY = panel.bounds.y + 60; // After header section

	// Each box is 30x30 pixels
	float boxSize = 30.0f;
	float gap = 2.0f; // Small gap between boxes

	// Row 1: FRONT (centered above middle row)
	panel.boxFront = Rectangle {
	    startX + boxSize + gap, // Aligned with CENTER
	    startY,
	    boxSize,
	    boxSize};

	// Row 2: LEFT, CENTER, RIGHT
	panel.boxLeft = Rectangle {
	    startX,
	    startY + boxSize + gap,
	    boxSize,
	    boxSize};

	panel.boxCenter = Rectangle {
	    startX + boxSize + gap,
	    startY + boxSize + gap,
	    boxSize,
	    boxSize};

	panel.boxRight = Rectangle {
	    startX + (boxSize + gap) * 2,
	    startY + boxSize + gap,
	    boxSize,
	    boxSize};

	// Row 3: REAR (centered below middle row)
	panel.boxRear = Rectangle {
	    startX + boxSize + gap, // Aligned with CENTER
	    startY + (boxSize + gap) * 2,
	    boxSize,
	    boxSize};
}

} // namespace uipanel
