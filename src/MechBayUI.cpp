#include "MechBayUI.hpp"
#include <cstdio>
#include "Config.hpp"
#include "Constants.hpp"
#include "rl/raygui.h"

namespace mechbayui {

MechBayData InitializeMockMechBayData() {
	MechBayData data;

	// Mech info
	data.mechName = "ANNIHILATOR ANH-3A";
	data.tonnage = 99.98f;
	data.maxTonnage = 100.0f;

	// Stats
	data.stats = {
	    {"ARMOR", "572"},
	    {"STRUCTURE", "264"},
	    {"ALPHA DAMAGE", "156"},
	    {"WEAPON HEAT/s", "12.4"},
	    {"WEAPON DPS", "82.6"},
	    {"HEAT EFFICIENCY", "85%"},
	    {"TOP SPEED", "48.6 km/h"},
	    {"MAX RANGE", "900m"},
	};

	// Weapon inventory (mock data with infinity symbol for unlimited qty)
	data.inventory = {
	    {-1, "AC/20 AMMO", "S", 1.0f, 100, 12, 270, Color {160, 120, 200, 255}}, // Purple - STD
	    {-1, "LRM 20 AMMO", "S", 1.0f, 4, 240, 1000, Color {160, 120, 200, 255}},
	    {-1, "MACHINE GUN AMMO", "S", 0.5f, 2, 600, 200, Color {100, 200, 220, 255}}, // Cyan - Light
	    {-1, "MEDIUM LASER", "M", 1.0f, 25, 150, 500, Color {255, 140, 60, 255}},     // Orange - XL
	    {-1, "AC/20", "L", 14.0f, 100, 12, 270, Color {255, 80, 80, 255}},            // Red - XXL
	    {-1, "LRM 20", "L", 10.0f, 4, 240, 1000, Color {255, 80, 80, 255}},
	    {-1, "HEAT SINK", "S", 1.0f, 0, 0, 0, Color {100, 200, 220, 255}},
	    {-1, "JUMP JET", "M", 2.0f, 0, 0, 0, Color {255, 140, 60, 255}},
	};

	// Body sections with equipment slots
	// Colors for different equipment types
	Color greenSlot = Color {80, 180, 100, 255};  // Easy/standard
	Color blueSlot = Color {80, 120, 200, 255};   // Lasers
	Color orangeSlot = Color {255, 140, 60, 255}; // Heat/XL
	Color redSlot = Color {200, 80, 80, 255};     // Hardened/XXL
	Color yellowSlot = Color {200, 180, 60, 255}; // Assault

	// HEAD
	BodySection head;
	head.name = "HEAD";
	head.armorFront = 42;
	head.armorRear = -1; // No rear armor for head
	head.structure = 20;
	head.slots = {
	    {"SENSORS", greenSlot, false},
	    {"LIFE SUPPORT", greenSlot, false},
	    {"COCKPIT", greenSlot, false},
	};
	data.bodySections.push_back(head);

	// CENTER TORSO
	BodySection centerTorso;
	centerTorso.name = "CENTER TORSO";
	centerTorso.armorFront = 92;
	centerTorso.armorRear = 35;
	centerTorso.structure = 42;
	centerTorso.slots = {
	    {"ENGINE", redSlot, false},
	    {"ENGINE", redSlot, false},
	    {"ENGINE", redSlot, false},
	    {"GYRO", orangeSlot, false},
	    {"ARMOR: LIGHT FERRO A", blueSlot, false},
	    {"TC: MODULAR", yellowSlot, false},
	};
	data.bodySections.push_back(centerTorso);

	// RIGHT TORSO
	BodySection rightTorso;
	rightTorso.name = "RIGHT TORSO";
	rightTorso.armorFront = 68;
	rightTorso.armorRear = 30;
	rightTorso.structure = 32;
	rightTorso.slots = {
	    {"AC/20", redSlot, false},
	    {"AC/20", redSlot, false},
	    {"AC/20 AMMO", yellowSlot, false},
	    {"LRM 20", orangeSlot, false},
	    {"LRM 20 AMMO", greenSlot, false},
	    {"HEAT SINK", blueSlot, false},
	};
	data.bodySections.push_back(rightTorso);

	// LEFT TORSO
	BodySection leftTorso;
	leftTorso.name = "LEFT TORSO";
	leftTorso.armorFront = 68;
	leftTorso.armorRear = 30;
	leftTorso.structure = 32;
	leftTorso.slots = {
	    {"AC/20", redSlot, false},
	    {"AC/20", redSlot, false},
	    {"AC/20 AMMO", yellowSlot, false},
	    {"LRM 20", orangeSlot, false},
	    {"LRM 20 AMMO", greenSlot, false},
	    {"HEAT SINK", blueSlot, false},
	};
	data.bodySections.push_back(leftTorso);

	// RIGHT ARM
	BodySection rightArm;
	rightArm.name = "RIGHT ARM";
	rightArm.armorFront = 52;
	rightArm.armorRear = -1;
	rightArm.structure = 28;
	rightArm.slots = {
	    {"SHOULDER", greenSlot, false},
	    {"UPPER ARM", greenSlot, false},
	    {"LOWER ARM", greenSlot, false},
	    {"HAND", greenSlot, false},
	    {"MEDIUM LASER", orangeSlot, false},
	    {"MEDIUM LASER", orangeSlot, false},
	};
	data.bodySections.push_back(rightArm);

	// LEFT ARM
	BodySection leftArm;
	leftArm.name = "LEFT ARM";
	leftArm.armorFront = 52;
	leftArm.armorRear = -1;
	leftArm.structure = 28;
	leftArm.slots = {
	    {"SHOULDER", greenSlot, false},
	    {"UPPER ARM", greenSlot, false},
	    {"LOWER ARM", greenSlot, false},
	    {"HAND", greenSlot, false},
	    {"MEDIUM LASER", orangeSlot, false},
	    {"MEDIUM LASER", orangeSlot, false},
	};
	data.bodySections.push_back(leftArm);

	// RIGHT LEG
	BodySection rightLeg;
	rightLeg.name = "RIGHT LEG";
	rightLeg.armorFront = 62;
	rightLeg.armorRear = -1;
	rightLeg.structure = 35;
	rightLeg.slots = {
	    {"HIP", greenSlot, false},
	    {"UPPER LEG", greenSlot, false},
	    {"LOWER LEG", greenSlot, false},
	    {"FOOT", greenSlot, false},
	    {"JUMP JET", orangeSlot, false},
	};
	data.bodySections.push_back(rightLeg);

	// LEFT LEG
	BodySection leftLeg;
	leftLeg.name = "LEFT LEG";
	leftLeg.armorFront = 62;
	leftLeg.armorRear = -1;
	leftLeg.structure = 35;
	leftLeg.slots = {
	    {"HIP", greenSlot, false},
	    {"UPPER LEG", greenSlot, false},
	    {"LOWER LEG", greenSlot, false},
	    {"FOOT", greenSlot, false},
	    {"JUMP JET", orangeSlot, false},
	};
	data.bodySections.push_back(leftLeg);

	return data;
}

void RenderMechBayScreen(GameState& game) {
	// Calculate screen dimensions
	int screenWidth = SCREEN_WIDTH;
	int screenHeight = SCREEN_HEIGHT;

	// Modal takes up ~75% of screen
	int modalWidth = (int)(screenWidth * 0.75f);
	int modalHeight = (int)(screenHeight * 0.75f);
	int modalX = (screenWidth - modalWidth) / 2;
	int modalY = (screenHeight - modalHeight) / 2;

	// Draw semi-transparent background overlay
	DrawRectangle(0, 0, screenWidth, screenHeight, Color {0, 0, 0, 180});

	// Draw main panel background
	GuiPanel(Rectangle {(float)modalX, (float)modalY, (float)modalWidth, (float)modalHeight}, "MECHBAY");

	// Get mock data
	MechBayData data = InitializeMockMechBayData();

	// Layout constants
	const int padding = 12;
	const int headerHeight = 40;
	const int fontSize = 12;
	const int lineHeight = 20;

	// Content area (inside panel)
	int contentX = modalX + padding;
	int contentY = modalY + headerHeight;
	int contentWidth = modalWidth - padding * 2;
	int contentHeight = modalHeight - headerHeight - padding;

	// ===== LEFT PANEL (~30% width) =====
	int leftPanelWidth = (int)(contentWidth * 0.30f);
	int leftPanelX = contentX;
	int leftPanelY = contentY;

	// Mech name header
	int yPos = leftPanelY;
	char mechHeader[128];
	snprintf(mechHeader, sizeof(mechHeader), "%s", data.mechName.c_str());
	GuiLabel(Rectangle {(float)leftPanelX, (float)yPos, (float)leftPanelWidth, 24}, mechHeader);
	yPos += 26;

	// Tonnage
	char tonnageText[64];
	snprintf(tonnageText, sizeof(tonnageText), "%.2f / %.2f TONS", data.tonnage, data.maxTonnage);
	GuiLabel(Rectangle {(float)leftPanelX, (float)yPos, (float)leftPanelWidth, 20}, tonnageText);
	yPos += 30;

	// Stats section
	GuiLabel(Rectangle {(float)leftPanelX, (float)yPos, (float)leftPanelWidth, 20}, "STATS");
	yPos += 22;

	// Draw stats in 2-column grid
	int statLabelWidth = leftPanelWidth * 0.60f;
	int statValueWidth = leftPanelWidth * 0.35f;
	for (const auto& stat : data.stats) {
		char labelText[64];
		snprintf(labelText, sizeof(labelText), "%s:", stat.label.c_str());
		GuiLabel(Rectangle {(float)leftPanelX, (float)yPos, (float)statLabelWidth, (float)lineHeight}, labelText);
		GuiLabel(Rectangle {(float)(leftPanelX + statLabelWidth), (float)yPos, (float)statValueWidth, (float)lineHeight}, stat.value.c_str());
		yPos += lineHeight;
	}

	yPos += 10;

	// Inventory section
	GuiLabel(Rectangle {(float)leftPanelX, (float)yPos, (float)leftPanelWidth, 20}, "INVENTORY");
	yPos += 22;

	// Filter buttons (icons placeholder)
	const char* filterLabels[] = {"ALL", "W", "M", "E", "B", "EQ"};
	int buttonWidth = (leftPanelWidth - 5 * 4) / 6; // 6 buttons with 4px spacing
	for (int i = 0; i < 6; i++) {
		GuiButton(Rectangle {(float)(leftPanelX + i * (buttonWidth + 4)), (float)yPos, (float)buttonWidth, 24}, filterLabels[i]);
	}
	yPos += 28;

	// Weapon list header
	GuiLabel(Rectangle {(float)leftPanelX, (float)yPos, 30, (float)lineHeight}, "QTY");
	GuiLabel(Rectangle {(float)(leftPanelX + 35), (float)yPos, (float)(leftPanelWidth * 0.40f), (float)lineHeight}, "NAME");
	GuiLabel(Rectangle {(float)(leftPanelX + leftPanelWidth * 0.45f), (float)yPos, 40, (float)lineHeight}, "SIZE");
	GuiLabel(Rectangle {(float)(leftPanelX + leftPanelWidth * 0.60f), (float)yPos, 50, (float)lineHeight}, "TONS");
	GuiLabel(Rectangle {(float)(leftPanelX + leftPanelWidth * 0.75f), (float)yPos, 40, (float)lineHeight}, "DMG");
	yPos += lineHeight;

	// Weapon entries
	int maxWeaponsToShow = 6;
	for (int i = 0; i < (int)data.inventory.size() && i < maxWeaponsToShow; i++) {
		const auto& weapon = data.inventory[i];

		// Draw background color
		DrawRectangle(leftPanelX, yPos, leftPanelWidth, lineHeight - 2, weapon.backgroundColor);

		// Quantity (infinity symbol if -1)
		char qtyText[16];
		if (weapon.quantity < 0) {
			snprintf(qtyText, sizeof(qtyText), "∞");
		} else {
			snprintf(qtyText, sizeof(qtyText), "%d", weapon.quantity);
		}
		DrawText(qtyText, leftPanelX + 5, yPos + 2, fontSize, WHITE);

		// Name
		DrawText(weapon.name.c_str(), leftPanelX + 35, yPos + 2, fontSize, WHITE);

		// Size
		DrawText(weapon.size.c_str(), leftPanelX + leftPanelWidth * 0.45f, yPos + 2, fontSize, WHITE);

		// Tonnage
		char tonsText[16];
		snprintf(tonsText, sizeof(tonsText), "%.1f", weapon.tonnage);
		DrawText(tonsText, leftPanelX + leftPanelWidth * 0.60f, yPos + 2, fontSize, WHITE);

		// Damage
		char dmgText[16];
		snprintf(dmgText, sizeof(dmgText), "%d", weapon.damage);
		DrawText(dmgText, leftPanelX + leftPanelWidth * 0.75f, yPos + 2, fontSize, WHITE);

		yPos += lineHeight;
	}

	// ===== RIGHT SECTION (~65% width, 3x3 grid layout) =====
	int rightSectionX = contentX + leftPanelWidth + padding * 2;
	int rightSectionWidth = contentWidth - leftPanelWidth - padding * 2;
	int columnWidth = rightSectionWidth / 3;
	int rowHeight = contentHeight / 3;

	// Helper function to render a body section
	auto renderBodySection = [&](const BodySection& section, int colX, int colY, int colWidth, int maxHeight) {
		int sectionY = colY;

		// Section name header
		GuiLabel(Rectangle {(float)colX, (float)sectionY, (float)colWidth, 20}, section.name.c_str());
		sectionY += 22;

		// Armor section
		char armorFrontText[64];
		snprintf(armorFrontText, sizeof(armorFrontText), "FRONT: %d", section.armorFront);
		GuiLabel(Rectangle {(float)colX, (float)sectionY, (float)colWidth, 18}, armorFrontText);
		sectionY += 18;

		if (section.armorRear >= 0) {
			char armorRearText[64];
			snprintf(armorRearText, sizeof(armorRearText), "REAR: %d", section.armorRear);
			GuiLabel(Rectangle {(float)colX, (float)sectionY, (float)colWidth, 18}, armorRearText);
			sectionY += 18;
		}

		// Structure
		char structText[64];
		snprintf(structText, sizeof(structText), "STRUCT: %d", section.structure);
		GuiLabel(Rectangle {(float)colX, (float)sectionY, (float)colWidth, 18}, structText);
		sectionY += 22;

		// Equipment slots
		for (const auto& slot : section.slots) {
			// Draw colored background
			DrawRectangle(colX, sectionY, colWidth - 4, lineHeight - 2, slot.backgroundColor);

			// Draw slot label
			DrawText(slot.label.c_str(), colX + 4, sectionY + 2, fontSize - 1, WHITE);

			sectionY += lineHeight;
		}

		return sectionY;
	};

	// 3x3 Grid layout
	// Calculate grid positions
	int col1X = rightSectionX;
	int col2X = rightSectionX + columnWidth;
	int col3X = rightSectionX + columnWidth * 2;

	int row1Y = contentY;
	int row2Y = contentY + rowHeight;
	int row3Y = contentY + rowHeight * 2;

	// Row 1 - Torso section
	// LEFT: RIGHT TORSO (index 2)
	if (data.bodySections.size() > 2) {
		renderBodySection(data.bodySections[2], col1X, row1Y, columnWidth - 8, rowHeight);
	}

	// CENTER: HEAD (index 0) stacked on top of CENTER TORSO (index 1)
	int centerY = row1Y;
	if (data.bodySections.size() > 0) {
		centerY = renderBodySection(data.bodySections[0], col2X, centerY, columnWidth - 8, rowHeight / 2) + 10;
	}
	if (data.bodySections.size() > 1) {
		renderBodySection(data.bodySections[1], col2X, centerY, columnWidth - 8, rowHeight / 2);
	}

	// RIGHT: LEFT TORSO (index 3)
	if (data.bodySections.size() > 3) {
		renderBodySection(data.bodySections[3], col3X, row1Y, columnWidth - 8, rowHeight);
	}

	// Row 2 - Arms section
	// LEFT: RIGHT ARM (index 4)
	if (data.bodySections.size() > 4) {
		renderBodySection(data.bodySections[4], col1X, row2Y, columnWidth - 8, rowHeight);
	}

	// CENTER: Empty (intentionally left blank)

	// RIGHT: LEFT ARM (index 5)
	if (data.bodySections.size() > 5) {
		renderBodySection(data.bodySections[5], col3X, row2Y, columnWidth - 8, rowHeight);
	}

	// Row 3 - Legs section
	// LEFT: RIGHT LEG (index 6)
	if (data.bodySections.size() > 6) {
		renderBodySection(data.bodySections[6], col1X, row3Y, columnWidth - 8, rowHeight);
	}

	// CENTER: Empty (intentionally left blank)

	// RIGHT: LEFT LEG (index 7)
	if (data.bodySections.size() > 7) {
		renderBodySection(data.bodySections[7], col3X, row3Y, columnWidth - 8, rowHeight);
	}

	// ===== CLOSE BUTTON (X in top-right corner) =====
	int closeButtonSize = 30;
	int closeButtonX = modalX + modalWidth - closeButtonSize - 8;
	int closeButtonY = modalY + 8;
	if (GuiButton(Rectangle {(float)closeButtonX, (float)closeButtonY, (float)closeButtonSize, (float)closeButtonSize}, "X")) {
		game.showMechbayScreen = false;
	}
}

} // namespace mechbayui
