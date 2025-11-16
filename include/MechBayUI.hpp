#ifndef OPENWANZER_MECHBAY_UI_HPP
#define OPENWANZER_MECHBAY_UI_HPP

#include <string>
#include <vector>
#include "GameState.hpp"
#include "rl/raylib.h"

namespace mechbayui {

// Weapon inventory entry
struct WeaponEntry {
	int quantity;          // Quantity available (use -1 for infinity symbol)
	std::string name;      // Weapon name
	std::string size;      // Size (e.g., "S", "M", "L")
	float tonnage;         // Weight in tons
	int damage;            // Damage value
	int rpm;               // Rate of fire (rounds per minute)
	int range;             // Range in meters
	Color backgroundColor; // Background color for type indication
};

// Equipment slot entry
struct EquipmentSlot {
	std::string label;     // Equipment label (e.g., "ARMOR: LIGHT FERRO A")
	Color backgroundColor; // Background color for slot type
	bool isEmpty;          // Whether slot is empty
};

// Body section (torso, arm, leg)
struct BodySection {
	std::string name;                 // Section name (e.g., "RIGHT TORSO", "HEAD")
	int armorFront;                   // Front armor value
	int armorRear;                    // Rear armor value (-1 if not applicable)
	int structure;                    // Internal structure value
	std::vector<EquipmentSlot> slots; // Equipment slots in this section
};

// Mech statistics entry
struct MechStat {
	std::string label; // Stat label
	std::string value; // Stat value (as string for flexibility)
};

// Main MechBay data structure
struct MechBayData {
	std::string mechName;                  // Mech name (e.g., "ANNIHILATOR ANH-3A")
	float tonnage;                         // Current tonnage
	float maxTonnage;                      // Maximum tonnage
	std::vector<MechStat> stats;           // Mech statistics
	std::vector<WeaponEntry> inventory;    // Weapon inventory
	std::vector<BodySection> bodySections; // Body sections (torsos, arms, legs, head)
};

// Initialize mock MechBay data
MechBayData InitializeMockMechBayData();

// Render the MechBay screen
void RenderMechBayScreen(GameState& game);

} // namespace mechbayui

#endif // OPENWANZER_MECHBAY_UI_HPP
