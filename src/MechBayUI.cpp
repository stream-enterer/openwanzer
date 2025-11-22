#include "MechBayUI.hpp"
#include "Config.hpp"
#include "Constants.hpp"
#include "FontManager.hpp"
#include "MechLoadout.hpp"
#include "Raygui.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace mechbayui {

// Static drag state (persists across frames)
static DragState gDragState;

// Scrollbar state for right section
struct ScrollbarState {
	float scrollOffset;    // Current scroll offset in pixels
	float maxScrollOffset; // Maximum scroll offset
	bool isDragging;       // Is scrollbar being dragged
	float dragStartY;      // Y position where drag started
	float dragStartOffset; // Scroll offset when drag started

	ScrollbarState()
	    : scrollOffset(0.0f), maxScrollOffset(0.0f), isDragging(false), dragStartY(0.0f), dragStartOffset(0.0f) {
	}
};

static ScrollbarState gScrollbar;

// Filter state for inventory search
struct InventoryFilterState {
	char searchText[64];                         // Current filter text
	bool isFocused;                              // Is textbox focused?
	equipment::EquipmentCategory activeCategory; // Active category filter (or UNKNOWN for all)

	InventoryFilterState()
	    : searchText {0}, isFocused(false), activeCategory(equipment::EquipmentCategory::UNKNOWN) {
	}

	void Clear() {
		searchText[0] = '\0';
	}

	bool IsEmpty() const {
		return searchText[0] == '\0';
	}
};

static InventoryFilterState gFilterState;

// Category filter groups
enum CategoryGroup {
	GROUP_ALL,
	GROUP_WEAPONS,
	GROUP_AMMO,
	GROUP_INTERNALS,
	GROUP_EQUIP
};

// Helper function to check if a category belongs to a group
bool IsCategoryInGroup(equipment::EquipmentCategory category, CategoryGroup group) {
	using namespace equipment;

	switch (group) {
		case GROUP_ALL:
			return true;
		case GROUP_WEAPONS:
			return category == EquipmentCategory::WEAPON;
		case GROUP_AMMO:
			return category == EquipmentCategory::AMMO;
		case GROUP_INTERNALS:
			return category == EquipmentCategory::ENGINE || category == EquipmentCategory::GYRO || category == EquipmentCategory::COCKPIT || category == EquipmentCategory::STRUCTURE || category == EquipmentCategory::ACTUATOR;
		case GROUP_EQUIP:
			return category == EquipmentCategory::HEAT_SINK || category == EquipmentCategory::UPGRADE || category == EquipmentCategory::JUMP_JET || category == EquipmentCategory::ARMOR;
		default:
			return false;
	}
}

// Fuzzy matching algorithm
// Returns -1 for no match, or a positive score (higher is better)
int FuzzyMatch(const char* needle, const char* haystack) {
	if (!needle || !haystack)
		return -1;

	// Empty needle matches everything with max score
	if (needle[0] == '\0')
		return 10000;

	// Convert to lowercase for case-insensitive matching
	char needleLower[256];
	char haystackLower[256];

	// Convert needle to lowercase
	int i = 0;
	while (needle[i] && i < 255) {
		needleLower[i] = std::tolower(needle[i]);
		i++;
	}
	needleLower[i] = '\0';

	// Convert haystack to lowercase
	i = 0;
	while (haystack[i] && i < 255) {
		haystackLower[i] = std::tolower(haystack[i]);
		i++;
	}
	haystackLower[i] = '\0';

	int score = 0;
	int haystackIndex = 0;
	int needleLen = strlen(needleLower);
	int haystackLen = strlen(haystackLower);

	for (int needleIndex = 0; needleIndex < needleLen; needleIndex++) {
		char needleChar = needleLower[needleIndex];
		bool found = false;

		// Search for this character in the remaining haystack
		for (int j = haystackIndex; j < haystackLen; j++) {
			if (haystackLower[j] == needleChar) {
				// Found a match
				found = true;
				int charPos = j;

				// Award points based on match characteristics
				if (charPos == 0) {
					score += 100; // Start of string bonus
				}

				if (charPos > 0 && (haystack[charPos - 1] == ' ' || haystack[charPos - 1] == '-' || haystack[charPos - 1] == '_')) {
					score += 50; // Word boundary bonus
				}

				// Penalty for distance from previous match
				score -= (charPos - haystackIndex) * 2;

				haystackIndex = charPos + 1;
				break;
			}
		}

		if (!found) {
			return -1; // No match
		}
	}

	return score;
}

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

	// Sync filter focus state to game state (so Main.cpp can check it)
	game.mechbayFilterFocused = gFilterState.isFocused;

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

	// Font cache
	float spacing = fontmanager::FONT_CACHE.GetSpacing();
	Font font11 = fontmanager::FONT_CACHE.GetFont(fontSize - 1);
	Font font12 = fontmanager::FONT_CACHE.GetFont(fontSize);
	Font font14 = fontmanager::FONT_CACHE.GetFont(fontSize + 2);
	Font font16 = fontmanager::FONT_CACHE.GetFont(fontSize + 4);

	// Content area (inside panel)
	int contentX = modalX + padding;
	int contentY = modalY + headerHeight;
	int contentWidth = modalWidth - padding * 2;
	int contentHeight = modalHeight - headerHeight - padding - 50; // Reserve space for buttons

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
	DrawTextEx(font14, tonnageText, Vector2 {(float)leftPanelX, (float)yPos}, (float)(fontSize + 2), spacing, tonnageColor);
	yPos += 30;

	// Inventory section
	GuiLabel(Rectangle {(float)leftPanelX, (float)yPos, (float)leftPanelWidth, 20}, "INVENTORY");
	yPos += 22;

	// ===== FILTER INPUT BOX =====
	const int filterHeight = 20;
	const int filterMargin = 5;
	const int iconSize = 16;

	// Lens icon
	GuiDrawIcon(ICON_LENS, leftPanelX + filterMargin, yPos + 2, 1, GRAY);

	// Filter textbox
	Rectangle filterBounds = {
	    (float)(leftPanelX + iconSize + filterMargin * 2),
	    (float)yPos,
	    (float)(leftPanelWidth - iconSize - filterMargin * 3),
	    (float)filterHeight};

	// Draw textbox
	if (GuiTextBox(filterBounds, gFilterState.searchText, sizeof(gFilterState.searchText), gFilterState.isFocused)) {
		// Toggle focus when clicked
		gFilterState.isFocused = !gFilterState.isFocused;
	}

	// Yellow border when focused
	if (gFilterState.isFocused) {
		DrawRectangleLinesEx(filterBounds, 2.0f, Color {255, 255, 0, 255});
	}

	yPos += filterHeight + filterMargin;

	// ===== CATEGORY FILTER BUTTONS =====
	const char* categoryLabels[] = {"WEAPONS", "AMMO", "INTRNL", "EQUIP"};
	CategoryGroup categoryGroups[] = {GROUP_WEAPONS, GROUP_AMMO, GROUP_INTERNALS, GROUP_EQUIP};
	static CategoryGroup activeGroup = GROUP_ALL;

	int filterButtonWidth = (leftPanelWidth - 6) / 4;
	int filterButtonHeight = 20;

	for (int i = 0; i < 4; i++) {
		int buttonX = leftPanelX + i * (filterButtonWidth + 2);
		bool isActive = (activeGroup == categoryGroups[i]);

		Rectangle buttonBounds = {(float)buttonX, (float)yPos, (float)filterButtonWidth, (float)filterButtonHeight};

		if (GuiButton(buttonBounds, categoryLabels[i])) {
			// Toggle: clicking active button deactivates it
			if (isActive) {
				activeGroup = GROUP_ALL;
			} else {
				activeGroup = categoryGroups[i];
			}
		}

		// Highlight active button with yellow border
		if (isActive) {
			DrawRectangleLinesEx(buttonBounds, 2.0f, Color {255, 255, 0, 255});
		}
	}

	yPos += filterButtonHeight + filterMargin;

	// ===== INPUT HANDLING FOR FILTER =====
	// Handle auto-focus on typing (printable characters)
	int keyPressed = GetCharPressed();
	if (keyPressed > 0 && keyPressed < 127 && !gFilterState.isFocused) {
		// Auto-focus and insert character
		gFilterState.isFocused = true;
		int len = strlen(gFilterState.searchText);
		if (len < (int)sizeof(gFilterState.searchText) - 1) {
			gFilterState.searchText[len] = (char)keyPressed;
			gFilterState.searchText[len + 1] = '\0';
		}
	}

	// Handle ESC key
	if (IsKeyPressed(KEY_ESCAPE)) {
		if (gFilterState.isFocused) {
			if (!gFilterState.IsEmpty()) {
				// Clear text if not empty
				gFilterState.Clear();
			} else {
				// Defocus if empty
				gFilterState.isFocused = false;
			}
		}
		// Note: If not focused, ESC will be handled by the existing MechBay close logic
	}

	// Handle mouse click outside filter box to defocus
	if (gFilterState.isFocused && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		Vector2 mousePos = GetMousePosition();
		if (!CheckCollisionPointRec(mousePos, filterBounds)) {
			gFilterState.isFocused = false;
		}
	}

	// Inventory list header
	GuiLabel(Rectangle {(float)leftPanelX, (float)yPos, 30, (float)lineHeight}, "QTY");
	GuiLabel(Rectangle {(float)(leftPanelX + 35), (float)yPos, (float)(leftPanelWidth * 0.40f), (float)lineHeight}, "NAME");
	GuiLabel(Rectangle {(float)(leftPanelX + leftPanelWidth * 0.45f), (float)yPos, 40, (float)lineHeight}, "SIZE");
	GuiLabel(Rectangle {(float)(leftPanelX + leftPanelWidth * 0.60f), (float)yPos, 50, (float)lineHeight}, "TONS");
	GuiLabel(Rectangle {(float)(leftPanelX + leftPanelWidth * 0.75f), (float)yPos, 40, (float)lineHeight}, "DMG");
	yPos += lineHeight;

	// ===== FILTER AND RENDER INVENTORY ITEMS =====
	const auto& inventory = loadout->GetInventory();
	int inventoryYStart = yPos;

	// Build filtered list with scores
	struct FilteredItem {
		std::string componentDefID;
		equipment::Equipment* equipment;
		int quantity;
		int score;

		bool operator<(const FilteredItem& other) const {
			return score > other.score; // Sort descending by score
		}
	};

	std::vector<FilteredItem> filteredItems;

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

		// Apply fuzzy text filter
		int textScore = FuzzyMatch(gFilterState.searchText, eq->GetUIName().c_str());
		if (textScore < 0)
			continue; // No match

		// Apply category filter
		bool categoryMatch = IsCategoryInGroup(eq->GetCategory(), activeGroup);
		if (!categoryMatch)
			continue;

		// Add to filtered list
		filteredItems.push_back({componentDefID, eq, quantity, textScore});
	}

	// Sort by score (best matches first)
	std::sort(filteredItems.begin(), filteredItems.end());

	// Render filtered inventory items
	for (const auto& item : filteredItems) {
		equipment::Equipment* eq = item.equipment;
		int quantity = item.quantity;

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
			DrawTextEx(font12, qtyText, Vector2 {(float)(leftPanelX + 5), (float)(yPos + 2)}, (float)fontSize, spacing, WHITE);

			// Name
			DrawTextEx(font12, eq->GetUIName().c_str(), Vector2 {(float)(leftPanelX + 35), (float)(yPos + 2)}, (float)fontSize, spacing, WHITE);

			// Size
			std::string sizeStr = GetSizeString(eq->GetInventorySize());
			DrawTextEx(font12, sizeStr.c_str(), Vector2 {leftPanelX + leftPanelWidth * 0.45f, (float)(yPos + 2)}, (float)fontSize, spacing, WHITE);

			// Tonnage
			char tonsText[16];
			snprintf(tonsText, sizeof(tonsText), "%.1f", eq->GetTonnage());
			DrawTextEx(font12, tonsText, Vector2 {leftPanelX + leftPanelWidth * 0.60f, (float)(yPos + 2)}, (float)fontSize, spacing, WHITE);

			// Damage (for weapons only)
			if (eq->GetCategory() == equipment::EquipmentCategory::WEAPON) {
				char dmgText[16];
				snprintf(dmgText, sizeof(dmgText), "%d", eq->GetDamage());
				DrawTextEx(font12, dmgText, Vector2 {leftPanelX + leftPanelWidth * 0.75f, (float)(yPos + 2)}, (float)fontSize, spacing, WHITE);
			}
		}

		yPos += lineHeight;
	}

	// ===== RIGHT SECTION (~65% width, 3x3 grid layout with scrollbar) =====
	const int scrollbarWidth = 16;
	int rightSectionX = contentX + leftPanelWidth + padding * 2;
	int rightSectionWidth = contentWidth - leftPanelWidth - padding * 2 - scrollbarWidth;
	int rightSectionHeight = contentHeight;

	// Define scrollable area viewport
	Rectangle scrollViewport = {
	    (float)rightSectionX,
	    (float)contentY,
	    (float)rightSectionWidth,
	    (float)rightSectionHeight};

	// Calculate total content height (all body parts)
	int columnWidth = rightSectionWidth / 3;
	int totalContentHeight = 0;

	// Helper lambda to calculate body section height
	auto calculateBodySectionHeight = [&](const std::string& location) -> int {
		mechloadout::BodyPartSlot* bodyPart = loadout->GetBodyPart(location);
		if (!bodyPart)
			return 0;

		int height = 22;                           // Section name
		height += 18;                              // Armor
		height += 18;                              // Structure
		height += 22;                              // Spacing
		height += bodyPart->maxSlots * lineHeight; // All slots (occupied + empty)
		return height;
	};

	using namespace mechloadout;

	// Calculate max height for each row (3 columns) with spacing
	const int rowSpacing = 20; // Spacing between rows

	int row1MaxHeight = std::max({calculateBodySectionHeight(LOC_RIGHT_TORSO),
	                              calculateBodySectionHeight(LOC_HEAD) + calculateBodySectionHeight(LOC_CENTER_TORSO) + 10,
	                              calculateBodySectionHeight(LOC_LEFT_TORSO)});

	int row2MaxHeight = std::max({calculateBodySectionHeight(LOC_RIGHT_ARM),
	                              100, // Center tonnage display
	                              calculateBodySectionHeight(LOC_LEFT_ARM)});

	int row3MaxHeight = std::max({calculateBodySectionHeight(LOC_RIGHT_LEG),
	                              0, // Center empty
	                              calculateBodySectionHeight(LOC_LEFT_LEG)});

	totalContentHeight = row1MaxHeight + rowSpacing + row2MaxHeight + rowSpacing + row3MaxHeight + rowSpacing;

	// Update scrollbar max offset
	gScrollbar.maxScrollOffset = std::max(0.0f, (float)(totalContentHeight - rightSectionHeight));

	// Handle scrollbar interactions
	Rectangle scrollbarBounds = {
	    (float)(rightSectionX + rightSectionWidth),
	    (float)contentY,
	    (float)scrollbarWidth,
	    (float)rightSectionHeight};

	// Mouse wheel scrolling (when over scrollable area)
	if (CheckCollisionPointRec(GetMousePosition(), scrollViewport) || CheckCollisionPointRec(GetMousePosition(), scrollbarBounds)) {
		float mouseWheel = GetMouseWheelMove();
		if (mouseWheel != 0) {
			gScrollbar.scrollOffset -= mouseWheel * 30.0f; // 30 pixels per wheel tick
			gScrollbar.scrollOffset = std::max(0.0f, std::min(gScrollbar.scrollOffset, gScrollbar.maxScrollOffset));
		}
	}

	// Scrollbar rendering and interaction
	if (gScrollbar.maxScrollOffset > 0) {
		// Draw scrollbar track
		DrawRectangleRec(scrollbarBounds, Color {40, 40, 40, 255});

		// Calculate scrollbar thumb size and position
		const float thumbMargin = 2.0f; // Margin on all sides
		float viewportRatio = (float)rightSectionHeight / (float)totalContentHeight;
		float availableThumbSpace = rightSectionHeight - (thumbMargin * 2);
		float thumbHeight = std::max(30.0f, availableThumbSpace * viewportRatio);
		float thumbY = contentY + thumbMargin + (gScrollbar.scrollOffset / gScrollbar.maxScrollOffset) * (availableThumbSpace - thumbHeight);

		Rectangle thumbBounds = {
		    scrollbarBounds.x + thumbMargin,
		    thumbY,
		    scrollbarBounds.width - (thumbMargin * 2),
		    thumbHeight};

		// Handle scrollbar thumb dragging
		if (gScrollbar.isDragging) {
			if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
				float mouseDeltaY = GetMouseY() - gScrollbar.dragStartY;
				float scrollDelta = mouseDeltaY / (rightSectionHeight - thumbHeight) * gScrollbar.maxScrollOffset;
				gScrollbar.scrollOffset = gScrollbar.dragStartOffset + scrollDelta;
				gScrollbar.scrollOffset = std::max(0.0f, std::min(gScrollbar.scrollOffset, gScrollbar.maxScrollOffset));
			} else {
				gScrollbar.isDragging = false;
			}
		} else {
			// Start dragging thumb
			if (CheckCollisionPointRec(GetMousePosition(), thumbBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
				gScrollbar.isDragging = true;
				gScrollbar.dragStartY = GetMouseY();
				gScrollbar.dragStartOffset = gScrollbar.scrollOffset;
			}
			// Click on track (above or below thumb)
			else if (CheckCollisionPointRec(GetMousePosition(), scrollbarBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
				float mouseY = GetMouseY();
				if (mouseY < thumbY) {
					// Click above thumb - page up
					gScrollbar.scrollOffset -= rightSectionHeight * 0.8f;
				} else if (mouseY > thumbY + thumbHeight) {
					// Click below thumb - page down
					gScrollbar.scrollOffset += rightSectionHeight * 0.8f;
				}
				gScrollbar.scrollOffset = std::max(0.0f, std::min(gScrollbar.scrollOffset, gScrollbar.maxScrollOffset));
			}
		}

		// Draw scrollbar thumb
		Color thumbColor = gScrollbar.isDragging ? Color {120, 120, 120, 255} : Color {80, 80, 80, 255};
		if (!gScrollbar.isDragging && CheckCollisionPointRec(GetMousePosition(), thumbBounds)) {
			thumbColor = Color {100, 100, 100, 255}; // Hover
		}
		DrawRectangleRec(thumbBounds, thumbColor);

		// Draw scrollbar border (on top of everything to ensure visibility)
		DrawRectangleLinesEx(scrollbarBounds, 1.0f, Color {80, 80, 80, 255});
	}

	// Begin scissor mode for scrollable content
	BeginScissorMode((int)scrollViewport.x, (int)scrollViewport.y, (int)scrollViewport.width, (int)scrollViewport.height);

	// Apply scroll offset to rendering
	int scrollOffsetInt = (int)gScrollbar.scrollOffset;
	int renderStartY = contentY - scrollOffsetInt;

	// Helper lambda to render a body section with all slots
	auto renderBodySection = [&](const std::string& location, int colX, int colY, int colWidth) -> int {
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

		int slotsStartY = sectionY; // Track where slots start

		// Render all slots (occupied + empty)
		int currentSlotIndex = 0;
		int equipmentIndex = 0;

		for (int slot = 0; slot < bodyPart->maxSlots; slot++) {
			// Find equipment at this slot
			equipment::Equipment* eq = nullptr;
			int equipmentSlotSpan = 0;

			if (equipmentIndex < (int)bodyPart->equipment.size()) {
				equipment::Equipment* candidateEq = bodyPart->equipment[equipmentIndex];
				if (candidateEq && currentSlotIndex == slot) {
					eq = candidateEq;
					equipmentSlotSpan = eq->GetInventorySize();
					equipmentIndex++;
				}
			}

			if (eq) {
				// Draw occupied slot(s)
				int slotHeight = lineHeight * equipmentSlotSpan;
				Rectangle slotBounds = {(float)colX, (float)sectionY, (float)(colWidth - 6), (float)(slotHeight - 2)};

				// Check if this item is being dragged
				bool isBeingDragged = (gDragState.isDragging && gDragState.draggedEquipment == eq && gDragState.sourceLocation == location);

				if (isBeingDragged) {
					// Draw empty cell appearance when dragging from this slot
					for (int i = 0; i < equipmentSlotSpan; i++) {
						Rectangle emptySlot = {(float)colX, (float)(sectionY + i * lineHeight), (float)(colWidth - 6), (float)(lineHeight - 2)};
						DrawRectangleRec(emptySlot, Color {60, 60, 60, 255});
						DrawRectangleLinesEx(emptySlot, 1.0f, Color {70, 70, 70, 255});
					}
				} else {
					// Draw colored background
					Color bgColor = GetEquipmentColor(eq->GetCategory(), eq->IsLocked());
					DrawRectangleRec(slotBounds, bgColor);

					// Handle mouse interaction (start drag if not locked)
					if (!eq->IsLocked() && CheckCollisionPointRec(GetMousePosition(), slotBounds)) {
						// Highlight on hover (inset by 1px to prevent clipping)
						DrawRectangle((int)slotBounds.x + 1, (int)slotBounds.y + 1, (int)slotBounds.width - 2, (int)slotBounds.height - 2, Color {255, 255, 255, 80});
						DrawRectangleLinesEx(Rectangle {slotBounds.x + 1, slotBounds.y + 1, slotBounds.width - 2, slotBounds.height - 2}, 1.0f, WHITE);

						// Start drag on mouse button down
						if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
							gDragState.isDragging = true;
							gDragState.draggedEquipment = eq;
							gDragState.sourceLocation = location;
							gDragState.sourceIndex = equipmentIndex - 1;
							gDragState.dragOffset = {GetMouseX() - slotBounds.x, GetMouseY() - slotBounds.y};
							gDragState.dragBounds = slotBounds;
						}
					}

					// Draw slot label
					DrawTextEx(font11, eq->GetUIName().c_str(), Vector2 {(float)(colX + 4), (float)(sectionY + 2)}, (float)(fontSize - 1), spacing, WHITE);
				}

				sectionY += slotHeight;
				currentSlotIndex += equipmentSlotSpan;
				slot += equipmentSlotSpan - 1; // Skip the slots this item occupies
			} else {
				// Draw empty slot (match occupied slot sizing)
				Rectangle emptySlot = {(float)colX, (float)sectionY, (float)(colWidth - 6), (float)(lineHeight - 2)};
				// Draw with dark background and visible grey border
				DrawRectangleRec(emptySlot, Color {60, 60, 60, 255});
				DrawRectangleLinesEx(emptySlot, 1.0f, Color {70, 70, 70, 255});

				sectionY += lineHeight;
				currentSlotIndex++;
			}
		}

		// Handle drop on entire equipment area (not headers)
		Rectangle dropZone = {
		    (float)colX,
		    (float)slotsStartY,
		    (float)(colWidth - 6),
		    (float)(sectionY - slotsStartY)};

		if (gDragState.isDragging && CheckCollisionPointRec(GetMousePosition(), dropZone)) {
			// Highlight drop zone
			DrawRectangle(colX, slotsStartY, colWidth - 6, sectionY - slotsStartY, Color {255, 255, 0, 50});

			// Handle drop
			if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
				// Append at the end of the equipment list (after any locked items)
				int dropSlotIndex = (int)bodyPart->equipment.size();

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

				// Only reset drag state if placement succeeded
				if (success) {
					gDragState.Reset();
				}
			}
		}

		return sectionY;
	};

	// 3x3 Grid layout
	int col1X = rightSectionX;
	int col2X = rightSectionX + columnWidth;
	int col3X = rightSectionX + columnWidth * 2;

	int row1Y = renderStartY;
	int row2Y = renderStartY + row1MaxHeight + rowSpacing;
	int row3Y = renderStartY + row1MaxHeight + rowSpacing + row2MaxHeight + rowSpacing;

	// Row 1 - Torso section
	// LEFT: RIGHT TORSO
	renderBodySection(LOC_RIGHT_TORSO, col1X, row1Y, columnWidth - 8);

	// CENTER: HEAD stacked on top of CENTER TORSO
	int centerY = row1Y;
	centerY = renderBodySection(LOC_HEAD, col2X, centerY, columnWidth - 8) + 10;
	renderBodySection(LOC_CENTER_TORSO, col2X, centerY, columnWidth - 8);

	// RIGHT: LEFT TORSO
	renderBodySection(LOC_LEFT_TORSO, col3X, row1Y, columnWidth - 8);

	// Row 2 - Arms section
	// LEFT: RIGHT ARM
	renderBodySection(LOC_RIGHT_ARM, col1X, row2Y, columnWidth - 8);

	// CENTER: Tonnage display (not draggable)
	int tonnageDisplayY = row2Y + row2MaxHeight / 2;
	DrawTextEx(font14, "TONNAGE", Vector2 {(float)(col2X + columnWidth / 2 - 40), (float)(tonnageDisplayY - 20)}, (float)(fontSize + 2), spacing, WHITE);
	DrawTextEx(font16, tonnageText, Vector2 {(float)(col2X + columnWidth / 2 - 50), (float)tonnageDisplayY}, (float)(fontSize + 4), spacing, tonnageColor);

	// RIGHT: LEFT ARM
	renderBodySection(LOC_LEFT_ARM, col3X, row2Y, columnWidth - 8);

	// Row 3 - Legs section
	// LEFT: RIGHT LEG
	renderBodySection(LOC_RIGHT_LEG, col1X, row3Y, columnWidth - 8);

	// CENTER: Empty (intentionally left blank)

	// RIGHT: LEFT LEG
	renderBodySection(LOC_LEFT_LEG, col3X, row3Y, columnWidth - 8);

	// End scissor mode
	EndScissorMode();

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
			DrawTextEx(font11, eq->GetUIName().c_str(), Vector2 {dragX + 4, dragY + 2}, (float)(fontSize - 1), spacing, WHITE);
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

		// Auto-release to inventory if dropped outside valid zones and drag state still active
		// (drop zones in renderBodySection will reset drag state if successful)
		if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && gDragState.isDragging) {
			// Return to inventory
			if (gDragState.sourceLocation != "inventory") {
				// Remove from body part
				loadout->RemoveEquipment(gDragState.sourceLocation, gDragState.sourceIndex);
				// RemoveEquipment already adds back to inventory
			}
			gDragState.Reset();
		}
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
		// Reset filter state
		gFilterState.Clear();
		gFilterState.isFocused = false;
	}

	if (GuiButton(Rectangle {(float)applyButtonX, (float)buttonY, (float)buttonWidth, (float)buttonHeight}, "APPLY")) {
		// Save current state
		loadout->SaveState();
		game.showMechbayScreen = false;
		// Reset filter state
		gFilterState.Clear();
		gFilterState.isFocused = false;
	}
}

} // namespace mechbayui
