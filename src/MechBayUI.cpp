#include "MechBayUI.hpp"
#include <cstdio>
#include "Config.hpp"
#include "Constants.hpp"
#include "MechLoadout.hpp"
#include "rl/raygui.h"

namespace mechbayui {

// Static drag state (persists across frames)
static DragState gDragState;

// Helper function implementations
Color GetEquipmentColor(equipment::EquipmentCategory category, bool isLocked) {
	if (isLocked) {
		return Color {100, 100, 100, 255}; // Gray for locked items
	}

	switch (category) {
		case equipment::EquipmentCategory::WEAPON:
			return Color {255, 140, 60, 255}; // Orange
		case equipment::EquipmentCategory::HEAT_SINK:
			return Color {100, 200, 220, 255}; // Cyan
		case equipment::EquipmentCategory::UPGRADE:
			return Color {200, 180, 60, 255}; // Yellow
		case equipment::EquipmentCategory::AMMO:
			return Color {160, 120, 200, 255}; // Purple
		case equipment::EquipmentCategory::JUMP_JET:
			return Color {255, 140, 60, 255}; // Orange
		case equipment::EquipmentCategory::ENGINE:
			return Color {255, 80, 80, 255}; // Red
		case equipment::EquipmentCategory::GYRO:
			return Color {255, 140, 60, 255}; // Orange
		case equipment::EquipmentCategory::COCKPIT:
			return Color {80, 180, 100, 255}; // Green
		case equipment::EquipmentCategory::ARMOR:
			return Color {80, 120, 200, 255}; // Blue
		case equipment::EquipmentCategory::STRUCTURE:
			return Color {80, 180, 100, 255}; // Green
		case equipment::EquipmentCategory::ACTUATOR:
			return Color {80, 180, 100, 255}; // Green
		default:
			return Color {120, 120, 120, 255}; // Gray
	}
}

std::string GetSizeString(int inventorySize) {
	if (inventorySize <= 1)
		return "S";
	else if (inventorySize <= 3)
		return "M";
	else
		return "L";
}

void RenderMechBayScreen(GameState& game) {
	if (!game.mechLoadout) {
		return; // No loadout initialized
	}

	mechloadout::MechLoadout* loadout = game.mechLoadout.get();

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
	snprintf(mechHeader, sizeof(mechHeader), "%s", loadout->GetChassisName().c_str());
	GuiLabel(Rectangle {(float)leftPanelX, (float)yPos, (float)leftPanelWidth, 24}, mechHeader);
	yPos += 26;

	// Tonnage
	float currentTonnage = loadout->GetCurrentTonnage();
	float maxTonnage = loadout->GetMaxTonnage();
	char tonnageText[64];
	snprintf(tonnageText, sizeof(tonnageText), "%.2f / %.2f TONS", currentTonnage, maxTonnage);

	// Color based on tonnage (green if under, red if over)
	Color tonnageColor = (currentTonnage <= maxTonnage) ? Color {80, 255, 80, 255} : Color {255, 80, 80, 255};
	DrawText(tonnageText, leftPanelX, yPos, fontSize + 2, tonnageColor);
	yPos += 30;

	// Inventory section
	GuiLabel(Rectangle {(float)leftPanelX, (float)yPos, (float)leftPanelWidth, 20}, "INVENTORY");
	yPos += 22;

	// Inventory list header
	GuiLabel(Rectangle {(float)leftPanelX, (float)yPos, 30, (float)lineHeight}, "QTY");
	GuiLabel(Rectangle {(float)(leftPanelX + 35), (float)yPos, (float)(leftPanelWidth * 0.40f), (float)lineHeight}, "NAME");
	GuiLabel(Rectangle {(float)(leftPanelX + leftPanelWidth * 0.45f), (float)yPos, 40, (float)lineHeight}, "SIZE");
	GuiLabel(Rectangle {(float)(leftPanelX + leftPanelWidth * 0.60f), (float)yPos, 50, (float)lineHeight}, "TONS");
	GuiLabel(Rectangle {(float)(leftPanelX + leftPanelWidth * 0.75f), (float)yPos, 40, (float)lineHeight}, "DMG");
	yPos += lineHeight;

	// Render inventory items
	const auto& inventory = loadout->GetInventory();
	int inventoryYStart = yPos;
	for (const auto& pair : inventory) {
		const std::string& componentDefID = pair.first;
		int quantity = pair.second;

		// Get equipment from database
		equipment::Equipment* eq = loadout->GetEquipmentByID(componentDefID);
		if (!eq)
			continue;

		// Skip locked/structural items in inventory display
		if (eq->IsLocked())
			continue;

		Rectangle itemBounds = {(float)leftPanelX, (float)yPos, (float)leftPanelWidth, (float)(lineHeight - 2)};

		// Check if this item is being dragged
		bool isBeingDragged = (gDragState.isDragging && gDragState.draggedEquipment == eq && gDragState.sourceLocation == "inventory");

		if (!isBeingDragged) {
			// Draw background color
			Color bgColor = GetEquipmentColor(eq->GetCategory(), false);
			DrawRectangleRec(itemBounds, bgColor);

			// Handle mouse interaction (start drag)
			if (CheckCollisionPointRec(GetMousePosition(), itemBounds)) {
				// Highlight on hover
				DrawRectangleLines((int)itemBounds.x, (int)itemBounds.y, (int)itemBounds.width, (int)itemBounds.height, WHITE);

				// Start drag on mouse button down
				if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
					gDragState.isDragging = true;
					gDragState.draggedEquipment = eq;
					gDragState.sourceLocation = "inventory";
					gDragState.sourceIndex = -1;
					gDragState.dragOffset = {GetMouseX() - itemBounds.x, GetMouseY() - itemBounds.y};
					gDragState.dragBounds = itemBounds;
				}
			}

			// Quantity (infinity symbol if -1)
			char qtyText[16];
			if (quantity < 0) {
				snprintf(qtyText, sizeof(qtyText), "∞");
			} else {
				snprintf(qtyText, sizeof(qtyText), "%d", quantity);
			}
			DrawText(qtyText, leftPanelX + 5, yPos + 2, fontSize, WHITE);

			// Name
			DrawText(eq->GetUIName().c_str(), leftPanelX + 35, yPos + 2, fontSize, WHITE);

			// Size
			std::string sizeStr = GetSizeString(eq->GetInventorySize());
			DrawText(sizeStr.c_str(), leftPanelX + leftPanelWidth * 0.45f, yPos + 2, fontSize, WHITE);

			// Tonnage
			char tonsText[16];
			snprintf(tonsText, sizeof(tonsText), "%.1f", eq->GetTonnage());
			DrawText(tonsText, leftPanelX + leftPanelWidth * 0.60f, yPos + 2, fontSize, WHITE);

			// Damage (for weapons only)
			if (eq->GetCategory() == equipment::EquipmentCategory::WEAPON) {
				char dmgText[16];
				snprintf(dmgText, sizeof(dmgText), "%d", eq->GetDamage());
				DrawText(dmgText, leftPanelX + leftPanelWidth * 0.75f, yPos + 2, fontSize, WHITE);
			}
		}

		yPos += lineHeight;
	}

	// ===== RIGHT SECTION (~65% width, 3x3 grid layout) =====
	int rightSectionX = contentX + leftPanelWidth + padding * 2;
	int rightSectionWidth = contentWidth - leftPanelWidth - padding * 2;
	int columnWidth = rightSectionWidth / 3;
	int rowHeight = contentHeight / 3;

	// Helper lambda to render a body section
	auto renderBodySection = [&](const std::string& location, int colX, int colY, int colWidth, int maxHeight) {
		mechloadout::BodyPartSlot* bodyPart = loadout->GetBodyPart(location);
		if (!bodyPart)
			return colY;

		int sectionY = colY;

		// Section name header
		GuiLabel(Rectangle {(float)colX, (float)sectionY, (float)colWidth, 20}, bodyPart->location.c_str());
		sectionY += 22;

		// Armor/Structure info (mock data for now)
		char armorText[64];
		snprintf(armorText, sizeof(armorText), "ARMOR: 50");
		GuiLabel(Rectangle {(float)colX, (float)sectionY, (float)colWidth, 18}, armorText);
		sectionY += 18;

		char structText[64];
		snprintf(structText, sizeof(structText), "STRUCT: 25");
		GuiLabel(Rectangle {(float)colX, (float)sectionY, (float)colWidth, 18}, structText);
		sectionY += 22;

		// Equipment slots
		int slotIndex = 0;
		for (equipment::Equipment* eq : bodyPart->equipment) {
			if (!eq)
				continue;

			// Calculate slot height based on inventory size
			int slotHeight = lineHeight * eq->GetInventorySize();

			Rectangle slotBounds = {(float)colX, (float)sectionY, (float)(colWidth - 4), (float)(slotHeight - 2)};

			// Check if this item is being dragged
			bool isBeingDragged = (gDragState.isDragging && gDragState.draggedEquipment == eq && gDragState.sourceLocation == location);

			if (!isBeingDragged) {
				// Draw colored background
				Color bgColor = GetEquipmentColor(eq->GetCategory(), eq->IsLocked());
				DrawRectangleRec(slotBounds, bgColor);

				// Handle mouse interaction (start drag if not locked)
				if (!eq->IsLocked() && CheckCollisionPointRec(GetMousePosition(), slotBounds)) {
					// Highlight on hover
					DrawRectangleLines((int)slotBounds.x, (int)slotBounds.y, (int)slotBounds.width, (int)slotBounds.height, WHITE);

					// Start drag on mouse button down
					if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
						gDragState.isDragging = true;
						gDragState.draggedEquipment = eq;
						gDragState.sourceLocation = location;
						gDragState.sourceIndex = slotIndex;
						gDragState.dragOffset = {GetMouseX() - slotBounds.x, GetMouseY() - slotBounds.y};
						gDragState.dragBounds = slotBounds;
					}
				}

				// Draw slot label
				DrawText(eq->GetUIName().c_str(), colX + 4, sectionY + 2, fontSize - 1, WHITE);
			}

			sectionY += slotHeight;
			slotIndex++;
		}

		// Draw empty slots (for drop zones)
		int freeSlots = bodyPart->GetFreeSlots();
		if (freeSlots > 0 && !gDragState.isDragging) {
			// Draw a visual indicator for free space
			int emptySlotHeight = lineHeight;
			DrawRectangle(colX, sectionY, colWidth - 4, emptySlotHeight - 2, Color {40, 40, 40, 128});
			char freeText[32];
			snprintf(freeText, sizeof(freeText), "(%d free)", freeSlots);
			DrawText(freeText, colX + 4, sectionY + 2, fontSize - 2, Color {150, 150, 150, 255});
		}

		// Handle drop on entire body section area
		if (gDragState.isDragging && CheckCollisionPointRec(GetMousePosition(), Rectangle {(float)colX, (float)colY, (float)colWidth, (float)maxHeight})) {
			// Highlight drop zone
			DrawRectangle(colX, colY, colWidth, maxHeight, Color {255, 255, 0, 50});

			// Handle drop
			if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
				// Calculate drop slot index based on Y position
				int dropSlotIndex = (GetMouseY() - (colY + 22 + 18 + 18 + 22)) / lineHeight;
				if (dropSlotIndex < 0)
					dropSlotIndex = 0;

				// Attempt to place equipment
				bool success = false;
				if (gDragState.sourceLocation == "inventory") {
					// From inventory to body part
					success = loadout->PlaceEquipment(gDragState.draggedEquipment, location, dropSlotIndex);
					if (success) {
						loadout->RemoveFromInventory(gDragState.draggedEquipment->GetComponentDefID());
					}
				} else {
					// From body part to body part (or same body part)
					success = loadout->MoveEquipment(gDragState.sourceLocation, gDragState.sourceIndex, location, dropSlotIndex);
				}

				gDragState.Reset();
			}
		}

		return sectionY;
	};

	// 3x3 Grid layout
	int col1X = rightSectionX;
	int col2X = rightSectionX + columnWidth;
	int col3X = rightSectionX + columnWidth * 2;

	int row1Y = contentY;
	int row2Y = contentY + rowHeight;
	int row3Y = contentY + rowHeight * 2;

	using namespace mechloadout;

	// Row 1 - Torso section
	// LEFT: RIGHT TORSO
	renderBodySection(LOC_RIGHT_TORSO, col1X, row1Y, columnWidth - 8, rowHeight);

	// CENTER: HEAD stacked on top of CENTER TORSO
	int centerY = row1Y;
	centerY = renderBodySection(LOC_HEAD, col2X, centerY, columnWidth - 8, rowHeight / 2) + 10;
	renderBodySection(LOC_CENTER_TORSO, col2X, centerY, columnWidth - 8, rowHeight / 2);

	// RIGHT: LEFT TORSO
	renderBodySection(LOC_LEFT_TORSO, col3X, row1Y, columnWidth - 8, rowHeight);

	// Row 2 - Arms section
	// LEFT: RIGHT ARM
	renderBodySection(LOC_RIGHT_ARM, col1X, row2Y, columnWidth - 8, rowHeight);

	// CENTER: Empty (intentionally left blank for tonnage display)
	// Draw tonnage in the center
	int tonnageDisplayY = row2Y + rowHeight / 2;
	DrawText("TONNAGE", col2X + columnWidth / 2 - 40, tonnageDisplayY - 20, fontSize + 2, WHITE);
	DrawText(tonnageText, col2X + columnWidth / 2 - 50, tonnageDisplayY, fontSize + 4, tonnageColor);

	// RIGHT: LEFT ARM
	renderBodySection(LOC_LEFT_ARM, col3X, row2Y, columnWidth - 8, rowHeight);

	// Row 3 - Legs section
	// LEFT: RIGHT LEG
	renderBodySection(LOC_RIGHT_LEG, col1X, row3Y, columnWidth - 8, rowHeight);

	// CENTER: Empty (intentionally left blank)

	// RIGHT: LEFT LEG
	renderBodySection(LOC_LEFT_LEG, col3X, row3Y, columnWidth - 8, rowHeight);

	// ===== DRAGGED ITEM RENDERING (on top of everything) =====
	if (gDragState.isDragging) {
		Vector2 mousePos = GetMousePosition();
		float dragX = mousePos.x - gDragState.dragOffset.x;
		float dragY = mousePos.y - gDragState.dragOffset.y;

		equipment::Equipment* eq = gDragState.draggedEquipment;
		if (eq) {
			int dragHeight = lineHeight * eq->GetInventorySize();
			Rectangle dragRect = {dragX, dragY, gDragState.dragBounds.width, (float)dragHeight};

			// Draw dragged item (fully opaque, matching body part appearance)
			Color bgColor = GetEquipmentColor(eq->GetCategory(), false);
			DrawRectangleRec(dragRect, bgColor);
			DrawRectangleLines((int)dragRect.x, (int)dragRect.y, (int)dragRect.width, (int)dragRect.height, WHITE);
			DrawText(eq->GetUIName().c_str(), (int)dragX + 4, (int)dragY + 2, fontSize - 1, WHITE);
		}

		// Handle drop on inventory (return to inventory)
		Rectangle inventoryArea = {(float)leftPanelX, (float)inventoryYStart, (float)leftPanelWidth, (float)(yPos - inventoryYStart)};
		if (CheckCollisionPointRec(mousePos, inventoryArea)) {
			// Highlight inventory as drop zone
			DrawRectangle(leftPanelX, inventoryYStart, leftPanelWidth, yPos - inventoryYStart, Color {255, 255, 0, 50});

			if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
				// Return to inventory
				if (gDragState.sourceLocation != "inventory") {
					// Remove from body part
					loadout->RemoveEquipment(gDragState.sourceLocation, gDragState.sourceIndex);
					// RemoveEquipment already adds back to inventory
				}
				gDragState.Reset();
			}
		}

		// Cancel drag on right-click or ESC
		if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) || IsKeyPressed(KEY_ESCAPE)) {
			gDragState.Reset();
		}

		// Note: Drop handling is done in renderBodySection lambda above
		// If mouse is released without hitting inventory or body section, drag will auto-cancel
	}

	// ===== APPLY/CANCEL BUTTONS =====
	int buttonWidth = 100;
	int buttonHeight = 30;
	int buttonY = modalY + modalHeight - buttonHeight - 10;
	int cancelButtonX = modalX + modalWidth - buttonWidth - 120;
	int applyButtonX = modalX + modalWidth - buttonWidth - 10;

	if (GuiButton(Rectangle {(float)cancelButtonX, (float)buttonY, (float)buttonWidth, (float)buttonHeight}, "CANCEL")) {
		// Restore saved state
		loadout->RestoreState();
		game.showMechbayScreen = false;
	}

	if (GuiButton(Rectangle {(float)applyButtonX, (float)buttonY, (float)buttonWidth, (float)buttonHeight}, "APPLY")) {
		// Save current state
		loadout->SaveState();
		game.showMechbayScreen = false;
	}

	// ===== CLOSE BUTTON (X in top-right corner) =====
	int closeButtonSize = 30;
	int closeButtonX = modalX + modalWidth - closeButtonSize - 8;
	int closeButtonY = modalY + 8;
	if (GuiButton(Rectangle {(float)closeButtonX, (float)closeButtonY, (float)closeButtonSize, (float)closeButtonSize}, "X")) {
		// Cancel (restore saved state)
		loadout->RestoreState();
		game.showMechbayScreen = false;
	}
}

} // namespace mechbayui
