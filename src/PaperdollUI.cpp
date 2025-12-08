#include "PaperdollUI.hpp"
#include "ArmorLocation.hpp"
#include "CherryStyle.hpp"
#include "Constants.hpp"
#include "GameLogic.hpp"
#include "Raygui.hpp"
#include "Raylib.hpp"
#include "Raymath.hpp"
#include "UIPanels.hpp"

#include <cmath>
#include <string>

namespace paperdollui {

// Forward declarations
Rectangle getLocationRect(const PaperdollPanel& panel, ArmorLocation location);
void renderFlashOverlay(const PaperdollPanel& panel);

// ============================================================================
// COLORS AND CONSTANTS
// ============================================================================

// Armor health state colors (based on percentage)
Color getArmorColor(float percentage) {
	if (percentage >= 0.80f)
		return WHITE;
	else if (percentage >= 0.60f)
		return LIGHTGRAY;
	else if (percentage >= 0.40f)
		return GRAY;
	else if (percentage >= 0.20f)
		return DARKGRAY;
	else
		return Color {50, 50, 50, 255};
}

// Structure exposed state
const Color STRUCTURE_COLOR = ORANGE;
const int STRIPE_WIDTH = 3;

// Destroyed location
const Color DESTROYED_COLOR = BLACK;
const Color DESTROYED_OUTLINE = Color {40, 40, 40, 255};

// Weapon type colors
const Color MISSILE_COLOR = MAGENTA;
const Color ENERGY_COLOR = GREEN;
const Color BALLISTIC_COLOR = SKYBLUE; // Cyan
const Color ARTILLERY_COLOR = RED;
const Color MELEE_COLOR = WHITE;
const Color DISABLED_WEAPON_COLOR = DARKGRAY;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

Color getWeaponColor(WeaponType type) {
	switch (type) {
		case WeaponType::MISSILE:
			return MISSILE_COLOR;
		case WeaponType::ENERGY:
			return ENERGY_COLOR;
		case WeaponType::BALLISTIC:
			return BALLISTIC_COLOR;
		case WeaponType::ARTILLERY:
			return ARTILLERY_COLOR;
		case WeaponType::MELEE:
			return MELEE_COLOR;
		default:
			return WHITE;
	}
}

std::string getWeightClassName(Unit::WeightClass wClass) {
	switch (wClass) {
		case Unit::WeightClass::LIGHT:
			return "LIGHT";
		case Unit::WeightClass::MEDIUM:
			return "MEDIUM";
		case Unit::WeightClass::HEAVY:
			return "HEAVY";
		case Unit::WeightClass::ASSAULT:
			return "ASSAULT";
		default:
			return "UNKNOWN";
	}
}

// Draw diagonal stripes for structure exposed
void renderDiagonalStripes(Rectangle rect, Color stripeColor, int width) {
	for (int offset = -(int)rect.height; offset < rect.width; offset += width * 2) {
		Vector2 start = {rect.x + offset, rect.y};
		Vector2 end = {rect.x + offset + rect.height, rect.y + rect.height};
		DrawLineEx(start, end, (float)width, stripeColor);
	}
}

// ============================================================================
// BODY SECTION RENDERING
// ============================================================================

void renderBodySection(Rectangle rect, const Unit* unit, ArmorLocation location) {
	if (unit->locations.find(location) == unit->locations.end()) {
		return;
	}

	const LocationStatus& loc = unit->locations.at(location);

	// Determine state and color
	bool isDestroyed = (loc.currentArmor == 0 && loc.currentStructure == 0);
	bool isStructureExposed = (loc.currentArmor == 0 && loc.currentStructure > 0);

	if (isDestroyed) {
		// Black with grey outline
		DrawRectangleRec(rect, DESTROYED_COLOR);
		DrawRectangleLinesEx(rect, 1, DESTROYED_OUTLINE);
	} else if (isStructureExposed) {
		// Orange with diagonal stripes
		DrawRectangleRec(rect, STRUCTURE_COLOR);
		// Use scissor mode to clip diagonal stripes to the rectangle bounds
		BeginScissorMode((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height);
		renderDiagonalStripes(rect, BLACK, STRIPE_WIDTH);
		EndScissorMode();
		DrawRectangleLinesEx(rect, 1, DESTROYED_OUTLINE);
	} else {
		// Armor present - color based on percentage
		float armorPercent = (float)loc.currentArmor / loc.maxArmor;
		Color armorColor = getArmorColor(armorPercent);
		DrawRectangleRec(rect, armorColor);
		DrawRectangleLinesEx(rect, 1, Color {40, 40, 40, 255});
	}
}

// ============================================================================
// PAPERDOLL RENDERING (5-box cross layout)
// ============================================================================

void renderCrossPaperdoll(const PaperdollPanel& panel, const Unit* unit) {
	// Draw 5-box cross layout:
	//         [FRONT]
	//     [LEFT][CENTER][RIGHT]
	//         [REAR]
	renderBodySection(panel.boxFront, unit, ArmorLocation::FRONT);
	renderBodySection(panel.boxLeft, unit, ArmorLocation::LEFT);
	renderBodySection(panel.boxCenter, unit, ArmorLocation::CENTER);
	renderBodySection(panel.boxRight, unit, ArmorLocation::RIGHT);
	renderBodySection(panel.boxRear, unit, ArmorLocation::REAR);
}

void renderPaperdollLabels(const PaperdollPanel& panel) {
	// Draw single-letter labels inside or near each box
	float spacing = (float)cherrystyle::kFontSpacing;
	const int fontSize = 12; // Smaller font for labels

	// Calculate label positions (centered in each box)
	float frontLabelX = panel.boxFront.x + panel.boxFront.width / 2 - 3;
	float frontLabelY = panel.boxFront.y + panel.boxFront.height + 2;

	float leftLabelX = panel.boxLeft.x + panel.boxLeft.width / 2 - 3;
	float leftLabelY = panel.boxLeft.y + panel.boxLeft.height + 2;

	float centerLabelX = panel.boxCenter.x + panel.boxCenter.width / 2 - 3;
	float centerLabelY = panel.boxCenter.y + panel.boxCenter.height + 2;

	float rightLabelX = panel.boxRight.x + panel.boxRight.width / 2 - 3;
	float rightLabelY = panel.boxRight.y + panel.boxRight.height + 2;

	float rearLabelX = panel.boxRear.x + panel.boxRear.width / 2 - 3;
	float rearLabelY = panel.boxRear.y + panel.boxRear.height + 2;

	DrawTextEx(cherrystyle::CHERRY_FONT, "F", Vector2 {frontLabelX, frontLabelY}, (float)fontSize, spacing, GRAY);
	DrawTextEx(cherrystyle::CHERRY_FONT, "L", Vector2 {leftLabelX, leftLabelY}, (float)fontSize, spacing, GRAY);
	DrawTextEx(cherrystyle::CHERRY_FONT, "C", Vector2 {centerLabelX, centerLabelY}, (float)fontSize, spacing, GRAY);
	DrawTextEx(cherrystyle::CHERRY_FONT, "R", Vector2 {rightLabelX, rightLabelY}, (float)fontSize, spacing, GRAY);
	DrawTextEx(cherrystyle::CHERRY_FONT, "R", Vector2 {rearLabelX, rearLabelY}, (float)fontSize, spacing, GRAY);
}

// ============================================================================
// WEAPON LOADOUT
// ============================================================================

void renderWeaponLoadout(const PaperdollPanel& panel, const Unit* unit) {
	float x = panel.bounds.x + panel.bounds.width - 150; // Right side
	float y = panel.bounds.y + 30;

	float spacing = (float)cherrystyle::kFontSpacing;
	const int fontSize = cherrystyle::kFontSize;
	DrawTextEx(cherrystyle::CHERRY_FONT, "LOADOUT", Vector2 {x, y}, (float)fontSize, spacing, GRAY);
	y += 22;

	for (const Weapon& weapon : unit->weapons) {
		Color weaponColor = getWeaponColor(weapon.type);
		if (weapon.isDestroyed) {
			weaponColor = DISABLED_WEAPON_COLOR;
		}

		DrawTextEx(cherrystyle::CHERRY_FONT, weapon.name.c_str(), Vector2 {x, y}, (float)fontSize, spacing, weaponColor);

		// Draw damage number in grey next to weapon
		if (weapon.type == WeaponType::MELEE || weapon.type == WeaponType::ARTILLERY) {
			DrawTextEx(cherrystyle::CHERRY_FONT, TextFormat("%d", weapon.damage), Vector2 {x + 80, y}, (float)fontSize, spacing, GRAY);
		}

		y += 18;
	}
}

// ============================================================================
// STATUS BARS
// ============================================================================

void renderStatusBar(float x, float y, float width, float height,
                     int current, int max, Color barColor, const char* label) {
	// Background
	DrawRectangle((int)x, (int)y, (int)width, (int)height, DARKGRAY);

	// Filled portion
	float fillWidth = (max > 0) ? (width * current) / max : 0;
	DrawRectangle((int)x, (int)y, (int)fillWidth, (int)height, barColor);

	// Border
	DrawRectangleLines((int)x, (int)y, (int)width, (int)height, WHITE);

	// Text
	float spacing = (float)cherrystyle::kFontSpacing;
	const int fontSize = cherrystyle::kFontSize;
	DrawTextEx(cherrystyle::CHERRY_FONT, TextFormat("%d/%d", current, max), Vector2 {x + 5, y + 3}, (float)fontSize, spacing, WHITE);
	DrawTextEx(cherrystyle::CHERRY_FONT, label, Vector2 {x, y + height + 4}, (float)fontSize, spacing, GRAY);
}

// ============================================================================
// PANEL HEADER
// ============================================================================

void renderPanelHeader(const PaperdollPanel& panel, const Unit* unit, [[maybe_unused]] bool isTargetPanel) {
	float x = panel.bounds.x + 10;
	float y = panel.bounds.y + 10;

	float spacing = (float)cherrystyle::kFontSpacing;
	const int fontSize = cherrystyle::kFontSize;
	const int largeFontSize = 20;
	const int mediumFontSize = 18;

	// Line 1: Mech name and variant
	std::string mechName = getWeightClassName(unit->weightClass);
	std::string variant = "MK-I"; // Placeholder
	DrawTextEx(cherrystyle::CHERRY_FONT, TextFormat("%s - %s", mechName.c_str(), variant.c_str()),
	           Vector2 {x, y}, (float)largeFontSize, spacing, ORANGE);

	// Line 1 continued: S: and A: values
	int totalStructure = 0, currentStructure = 0;
	int totalArmor = 0, currentArmor = 0;

	for (const auto& loc : unit->locations) {
		totalArmor += loc.second.maxArmor;
		currentArmor += loc.second.currentArmor;
		totalStructure += loc.second.maxStructure;
		currentStructure += loc.second.currentStructure;
	}

	DrawTextEx(cherrystyle::CHERRY_FONT, TextFormat("S: %d/%d", currentStructure, totalStructure),
	           Vector2 {x + 300, y}, (float)mediumFontSize, spacing, WHITE);
	DrawTextEx(cherrystyle::CHERRY_FONT, TextFormat("A: %d/%d", currentArmor, totalArmor),
	           Vector2 {x + 450, y}, (float)mediumFontSize, spacing, WHITE);

	// Line 2: Mech weight class and faction/pilot info
	y += 28;
	DrawTextEx(cherrystyle::CHERRY_FONT, TextFormat("'MECH: %s", mechName.c_str()), Vector2 {x, y}, (float)fontSize, spacing, LIGHTGRAY);

	// Placeholder faction logo (just a small box)
	DrawRectangle((int)(x + 150), (int)y, 20, 20, DARKGRAY);
	DrawTextEx(cherrystyle::CHERRY_FONT, "PILOT", Vector2 {x + 180, y}, (float)fontSize, spacing, LIGHTGRAY);

	// Line 3: Heat and Shield bars (vertically stacked)
	y += 28;

	// Heat bar (placeholder values) - moved 15px right from original position
	float barX = x + 95;
	renderStatusBar(barX, y, 100, 24, 50, 500, RED, "HEAT");

	// Shield bar below heat bar (placeholder values) - cyan color
	renderStatusBar(barX, y + 42, 100, 24, 75, 100, SKYBLUE, "SHIELD");
}

// ============================================================================
// TOOLTIPS
// ============================================================================

void renderLocationTooltip(const PaperdollPanel& panel, const Unit* unit) {
	if (panel.hoveredLocation == ArmorLocation::NONE)
		return;

	if (unit->locations.find(panel.hoveredLocation) == unit->locations.end()) {
		return;
	}

	const LocationStatus& loc = unit->locations.at(panel.hoveredLocation);

	// Build tooltip text
	std::string locationName = locationToString(panel.hoveredLocation);
	std::string armorText = TextFormat("%d/%d", loc.currentArmor, loc.maxArmor);
	std::string structureText = TextFormat("%d/%d", loc.currentStructure, loc.maxStructure);

	// Calculate tooltip size (increased for larger font)
	int tooltipWidth = 140;
	int tooltipHeight = 70;

	// Position near mouse (with bounds checking)
	Vector2 pos = panel.tooltipPos;
	pos.x = Clamp(pos.x, 0, SCREEN_WIDTH - tooltipWidth);
	pos.y = Clamp(pos.y, 0, SCREEN_HEIGHT - tooltipHeight);

	Rectangle tooltipRect = {pos.x, pos.y, (float)tooltipWidth, (float)tooltipHeight};

	// Draw tooltip background
	DrawRectangleRec(tooltipRect, Color {10, 10, 10, 240});
	DrawRectangleLinesEx(tooltipRect, 1, LIGHTGRAY);

	// Draw text
	float spacing = (float)cherrystyle::kFontSpacing;
	const int fontSize = cherrystyle::kFontSize;
	DrawTextEx(cherrystyle::CHERRY_FONT, locationName.c_str(), Vector2 {pos.x + 5, pos.y + 5}, (float)fontSize, spacing, WHITE);

	bool isStructureExposed = (loc.currentArmor == 0 && loc.currentStructure > 0);
	Color structColor = isStructureExposed ? ORANGE : WHITE;

	DrawTextEx(cherrystyle::CHERRY_FONT, TextFormat("A: %s", armorText.c_str()),
	           Vector2 {pos.x + 5, pos.y + 25}, (float)fontSize, spacing, WHITE);
	DrawTextEx(cherrystyle::CHERRY_FONT, TextFormat("S: %s", structureText.c_str()),
	           Vector2 {pos.x + 5, pos.y + 45}, (float)fontSize, spacing, structColor);
}

// ============================================================================
// MAIN PANEL RENDERING
// ============================================================================

void renderTargetPanel(const GameState& game) {
	if (!game.targetPanel.isVisible || !game.targetPanel.targetUnit)
		return;

	const TargetPanel& panel = game.targetPanel;
	Unit* unit = panel.targetUnit;

	// 1. Draw panel background
	Color backgroundColor = GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_NORMAL));
	Color borderColor = GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_NORMAL));

	DrawRectangleRec(panel.bounds, backgroundColor);
	DrawRectangleLinesEx(panel.bounds, 2, borderColor);

	// 2. Draw header section
	renderPanelHeader(panel, unit, true);

	// 3. Draw 5-box cross paperdoll
	renderCrossPaperdoll(panel, unit);

	// 4. Draw flash overlay (hit animation)
	renderFlashOverlay(panel);

	// 5. Draw weapon loadout list
	renderWeaponLoadout(panel, unit);

	// 6. Draw tooltip if hovering over body part
	if (panel.showTooltip) {
		renderLocationTooltip(panel, unit);
	}
}

void renderPlayerPanel(const GameState& game) {
	if (!game.playerPanel.isVisible || !game.playerPanel.playerUnit)
		return;

	const PlayerPanel& panel = game.playerPanel;
	Unit* unit = panel.playerUnit;

	// 1. Draw panel background
	Color backgroundColor = GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_NORMAL));
	Color borderColor = GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_NORMAL));

	DrawRectangleRec(panel.bounds, backgroundColor);
	DrawRectangleLinesEx(panel.bounds, 2, borderColor);

	// 2. Draw header section (simplified for player panel)
	renderPanelHeader(panel, unit, false);

	// 3. Draw 5-box cross paperdoll
	renderCrossPaperdoll(panel, unit);

	// 4. Draw flash overlay (hit animation)
	renderFlashOverlay(panel);

	// 5. Draw tooltip if hovering over body part
	if (panel.showTooltip) {
		renderLocationTooltip(panel, unit);
	}
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

void updatePanelTooltip(PaperdollPanel& panel, Vector2 mousePos) {
	panel.hoveredLocation = ArmorLocation::NONE;
	panel.showTooltip = false;

	// Check each box in the 5-box cross layout for hover
	if (CheckCollisionPointRec(mousePos, panel.boxFront)) {
		panel.hoveredLocation = ArmorLocation::FRONT;
	} else if (CheckCollisionPointRec(mousePos, panel.boxLeft)) {
		panel.hoveredLocation = ArmorLocation::LEFT;
	} else if (CheckCollisionPointRec(mousePos, panel.boxCenter)) {
		panel.hoveredLocation = ArmorLocation::CENTER;
	} else if (CheckCollisionPointRec(mousePos, panel.boxRight)) {
		panel.hoveredLocation = ArmorLocation::RIGHT;
	} else if (CheckCollisionPointRec(mousePos, panel.boxRear)) {
		panel.hoveredLocation = ArmorLocation::REAR;
	}

	if (panel.hoveredLocation != ArmorLocation::NONE) {
		panel.showTooltip = true;
		panel.tooltipPos = mousePos;
	}
}

void handlePaperdollPanelDrag(GameState& game) {
	Vector2 mousePos = GetMousePosition();

	// Start dragging on left click
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		// Check target panel
		if (game.targetPanel.isVisible && CheckCollisionPointRec(mousePos, game.targetPanel.bounds)) {
			game.targetPanel.isDragging = true;
			game.targetPanel.dragOffset.x = mousePos.x - game.targetPanel.bounds.x;
			game.targetPanel.dragOffset.y = mousePos.y - game.targetPanel.bounds.y;
		}

		// Check player panel
		if (game.playerPanel.isVisible && CheckCollisionPointRec(mousePos, game.playerPanel.bounds)) {
			game.playerPanel.isDragging = true;
			game.playerPanel.dragOffset.x = mousePos.x - game.playerPanel.bounds.x;
			game.playerPanel.dragOffset.y = mousePos.y - game.playerPanel.bounds.y;
		}
	}

	// Stop dragging on release
	if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
		game.targetPanel.isDragging = false;
		game.playerPanel.isDragging = false;
	}

	// Update position while dragging, clamping to screen bounds
	if (game.targetPanel.isDragging) {
		float newX = mousePos.x - game.targetPanel.dragOffset.x;
		float newY = mousePos.y - game.targetPanel.dragOffset.y;

		// Clamp to screen bounds (keep panel fully visible)
		newX = Clamp(newX, 0, SCREEN_WIDTH - game.targetPanel.bounds.width);
		newY = Clamp(newY, 0, SCREEN_HEIGHT - game.targetPanel.bounds.height);

		game.targetPanel.bounds.x = newX;
		game.targetPanel.bounds.y = newY;
		// Recalculate paperdoll regions so they move with the panel
		uipanel::calculatePaperdollRegions(game.targetPanel);
	}

	if (game.playerPanel.isDragging) {
		float newX = mousePos.x - game.playerPanel.dragOffset.x;
		float newY = mousePos.y - game.playerPanel.dragOffset.y;

		// Clamp to screen bounds (keep panel fully visible)
		newX = Clamp(newX, 0, SCREEN_WIDTH - game.playerPanel.bounds.width);
		newY = Clamp(newY, 0, SCREEN_HEIGHT - game.playerPanel.bounds.height);

		game.playerPanel.bounds.x = newX;
		game.playerPanel.bounds.y = newY;
		// Recalculate paperdoll regions so they move with the panel
		uipanel::calculatePaperdollRegions(game.playerPanel);
	}
}

void handlePaperdollTooltips(GameState& game) {
	Vector2 mousePos = GetMousePosition();

	// Update target panel tooltips
	if (game.targetPanel.isVisible) {
		updatePanelTooltip(game.targetPanel, mousePos);
	}

	// Update player panel tooltips
	if (game.playerPanel.isVisible) {
		updatePanelTooltip(game.playerPanel, mousePos);
	}
}

// ============================================================================
// FLASH OVERLAY
// ============================================================================

// Get the rectangle for a given armor location in a panel
Rectangle getLocationRect(const PaperdollPanel& panel, ArmorLocation location) {
	switch (location) {
		case ArmorLocation::FRONT:
			return panel.boxFront;
		case ArmorLocation::LEFT:
			return panel.boxLeft;
		case ArmorLocation::CENTER:
			return panel.boxCenter;
		case ArmorLocation::RIGHT:
			return panel.boxRight;
		case ArmorLocation::REAR:
			return panel.boxRear;
		default:
			return Rectangle {0, 0, 0, 0};
	}
}

void renderFlashOverlay(const PaperdollPanel& panel) {
	if (panel.flashFrame < 0 || panel.flashLocation == ArmorLocation::NONE)
		return;

	Rectangle rect = getLocationRect(panel, panel.flashLocation);
	if (rect.width <= 0 || rect.height <= 0)
		return;

	// Shrink rectangle slightly to stay inside borders
	Rectangle innerRect = {
	    rect.x + 1,
	    rect.y + 1,
	    rect.width - 2,
	    rect.height - 2};

	unsigned char alpha = panel.getFlashAlpha();
	Color flashColor = Color {0, 0, 0, alpha};
	DrawRectangleRec(innerRect, flashColor);
}

void triggerHitFlash(GameState& game, Unit* unit, ArmorLocation location) {
	// Check if unit matches target panel
	if (game.targetPanel.isVisible && game.targetPanel.targetUnit == unit) {
		game.targetPanel.startFlash(location);
	}

	// Check if unit matches player panel
	if (game.playerPanel.isVisible && game.playerPanel.playerUnit == unit) {
		game.playerPanel.startFlash(location);
	}
}

void updatePanelFlashes(GameState& game) {
	game.targetPanel.updateFlash();
	game.playerPanel.updateFlash();
}

} // namespace paperdollui
