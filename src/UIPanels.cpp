#include "UIPanels.hpp"
#include "Constants.hpp"

namespace uipanel {

// Panel dimensions (increased for 3D + net views)
const int TARGET_PANEL_WIDTH = 900;
const int TARGET_PANEL_HEIGHT = 350;
const int PLAYER_PANEL_WIDTH = 900;
const int PLAYER_PANEL_HEIGHT = 350;

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

	// Initialize 3D view (left side)
	game.targetPanel.polyView.trackball.camera = Camera3D {
	    .position = Vector3 {3.0f, 3.0f, 3.0f},
	    .target = Vector3 {0, 0, 0},
	    .up = Vector3 {0, 1, 0},
	    .fovy = 45.0f,
	    .projection = CAMERA_PERSPECTIVE};
	game.targetPanel.polyView.trackball.friction = 0.92f;
	game.targetPanel.polyView.trackball.isDragging = false;
	game.targetPanel.polyView.trackball.angularVelocity = {0, 0};
	game.targetPanel.polyView.trackball.previousMousePos = {0, 0};
	game.targetPanel.polyView.lockedToGridView = false;

	// Initialize net view (right side)
	game.targetPanel.netView.hoveredFaceIndex = -1;

	// Create render textures
	int viewWidth = TARGET_PANEL_WIDTH / 2 - 30;
	int viewHeight = TARGET_PANEL_HEIGHT - 80;
	game.targetPanel.polyView.Initialize(viewWidth, viewHeight);
	game.targetPanel.netView.Initialize(viewWidth, viewHeight);

	// Calculate viewports (will be updated in calculatePaperdollRegions)
	calculatePaperdollRegions(game.targetPanel);
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

	// Initialize 3D view (left side)
	game.playerPanel.polyView.trackball.camera = Camera3D {
	    .position = Vector3 {3.0f, 3.0f, 3.0f},
	    .target = Vector3 {0, 0, 0},
	    .up = Vector3 {0, 1, 0},
	    .fovy = 45.0f,
	    .projection = CAMERA_PERSPECTIVE};
	game.playerPanel.polyView.trackball.friction = 0.92f;
	game.playerPanel.polyView.trackball.isDragging = false;
	game.playerPanel.polyView.trackball.angularVelocity = {0, 0};
	game.playerPanel.polyView.trackball.previousMousePos = {0, 0};
	game.playerPanel.polyView.lockedToGridView = false;

	// Initialize net view (right side)
	game.playerPanel.netView.hoveredFaceIndex = -1;

	// Create render textures
	int viewWidth = PLAYER_PANEL_WIDTH / 2 - 30;
	int viewHeight = PLAYER_PANEL_HEIGHT - 80;
	game.playerPanel.polyView.Initialize(viewWidth, viewHeight);
	game.playerPanel.netView.Initialize(viewWidth, viewHeight);

	// Calculate viewports (will be updated in calculatePaperdollRegions)
	calculatePaperdollRegions(game.playerPanel);
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
	// NEW: 3D Polyhedron view layout
	float panelX = panel.bounds.x + 10;
	float panelY = panel.bounds.y + 40; // After header

	float viewWidth = (panel.bounds.width - 40) / 2;
	float viewHeight = panel.bounds.height - 80;

	// 3D view on left
	panel.polyViewport = Rectangle {panelX, panelY, viewWidth, viewHeight};

	// Net view on right
	panel.netViewport = Rectangle {panelX + viewWidth + 20, panelY, viewWidth, viewHeight};

	// Toggle button below 3D view
	panel.lockToggleBounds = Rectangle {panelX, panelY + viewHeight + 5, viewWidth, 20};

	// OLD: Keep 2D paperdoll regions for backward compatibility (not used anymore)
	float startX = panel.bounds.x + PAPERDOLL_PADDING;
	float startY = panel.bounds.y + 60;

	float sectionWidth = 25.0f;
	float sectionHeight = 30.0f;

	panel.frontHead = Rectangle {startX + sectionWidth * 2, startY, sectionWidth, sectionHeight};
	panel.frontLA = Rectangle {startX, startY + sectionHeight, sectionWidth, sectionHeight};
	panel.frontLT = Rectangle {startX + sectionWidth, startY + sectionHeight, sectionWidth, sectionHeight};
	panel.frontCT = Rectangle {startX + sectionWidth * 2, startY + sectionHeight, sectionWidth, sectionHeight};
	panel.frontRT = Rectangle {startX + sectionWidth * 3, startY + sectionHeight, sectionWidth, sectionHeight};
	panel.frontRA = Rectangle {startX + sectionWidth * 4, startY + sectionHeight, sectionWidth, sectionHeight};
	panel.frontLL = Rectangle {startX + sectionWidth, startY + sectionHeight * 2, sectionWidth, sectionHeight};
	panel.frontRL = Rectangle {startX + sectionWidth * 3, startY + sectionHeight * 2, sectionWidth, sectionHeight};

	float rearStartX = startX + FRONT_PAPERDOLL_WIDTH + 20;
	panel.rearLA = Rectangle {rearStartX, startY + sectionHeight, sectionWidth * 0.8f, sectionHeight};
	panel.rearLT = Rectangle {rearStartX + sectionWidth * 0.8f, startY + sectionHeight, sectionWidth, sectionHeight};
	panel.rearCT = Rectangle {rearStartX + sectionWidth * 1.8f, startY + sectionHeight, sectionWidth, sectionHeight};
	panel.rearRT = Rectangle {rearStartX + sectionWidth * 2.8f, startY + sectionHeight, sectionWidth, sectionHeight};
	panel.rearRA = Rectangle {rearStartX + sectionWidth * 3.8f, startY + sectionHeight, sectionWidth * 0.8f, sectionHeight};
}

} // namespace uipanel
