#ifndef OPENWANZER_MECHBAY_UI_HPP
#define OPENWANZER_MECHBAY_UI_HPP

#include <string>
#include <vector>
#include "Equipment.hpp"
#include "GameState.hpp"
#include "rl/raylib.h"

namespace mechbayui {

// Drag state for equipment items
struct DragState {
	bool isDragging;
	equipment::Equipment* draggedEquipment;
	std::string sourceLocation; // "inventory" or body part name (e.g., "CenterTorso")
	int sourceIndex;            // -1 for inventory, else index in body part
	Vector2 dragOffset;         // Mouse offset from item top-left
	Rectangle dragBounds;       // Current dragged item bounds (for rendering)

	DragState()
	    : isDragging(false),
	      draggedEquipment(nullptr),
	      sourceLocation(""),
	      sourceIndex(-1),
	      dragOffset {0, 0},
	      dragBounds {0, 0, 0, 0} {
	}

	void Reset() {
		isDragging = false;
		draggedEquipment = nullptr;
		sourceLocation = "";
		sourceIndex = -1;
		dragOffset = {0, 0};
		dragBounds = {0, 0, 0, 0};
	}
};

// Helper function to get color based on equipment category
Color GetEquipmentColor(equipment::EquipmentCategory category, bool isLocked);

// Helper function to get size string based on inventory size
std::string GetSizeString(int inventorySize);

// Render the MechBay screen with drag-and-drop support
void RenderMechBayScreen(GameState& game);

// Handle input for drag-and-drop
void HandleMechBayInput(GameState& game, DragState& dragState);

} // namespace mechbayui

#endif // OPENWANZER_MECHBAY_UI_HPP
