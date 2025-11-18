#include "Equipment.hpp"
#include <algorithm>
#include <cctype>

namespace equipment {

// List of structural parts that are ALWAYS locked
static const char* LOCKED_STRUCTURAL_PARTS[] = {
    // Leg actuators
    "emod_leg_hip",
    "emod_leg_upper",
    "emod_leg_lower",
    "emod_leg_foot",
    // Arm actuators
    "emod_arm_part_shoulder",
    "emod_arm_part_upper",
    "emod_arm_part_lower",
    "emod_arm_part_hand",
    // Core systems (using prefix matching)
    "Gear_Cockpit_",
    "Gear_Gyro_",
    "emod_engine_",
    "emod_structureslots_",
    "emod_armorslots_",
    "emod_engineslots_",
    "emod_kit_",
    "emod_engine_cooling",
};

// Constructor implementations
Equipment::Equipment()
    : componentDefID_(""),
      uiName_(""),
      details_(""),
      inventorySize_(1),
      tonnage_(0.0f),
      isLocked_(false),
      allowedLocations_("All"),
      disallowedLocations_(""),
      damage_(0),
      heat_(0),
      category_(EquipmentCategory::UNKNOWN) {
}

Equipment::Equipment(const std::string& id, const std::string& name, int slots, float tons)
    : componentDefID_(id),
      uiName_(name),
      details_(""),
      inventorySize_(slots),
      tonnage_(tons),
      isLocked_(false),
      allowedLocations_("All"),
      disallowedLocations_(""),
      damage_(0),
      heat_(0),
      category_(EquipmentCategory::UNKNOWN) {
	// Check if this is a structural item
	isLocked_ = IsStructuralItem();
}

bool Equipment::CanPlaceInLocation(const std::string& location) const {
	// Check disallowed locations first
	if (!disallowedLocations_.empty() && disallowedLocations_ != "None") {
		if (disallowedLocations_.find(location) != std::string::npos) {
			return false;
		}
	}

	// Check allowed locations
	if (allowedLocations_ == "All") {
		return true;
	}

	// Check if location is in allowed list
	if (allowedLocations_.find(location) != std::string::npos) {
		return true;
	}

	return false;
}

bool Equipment::IsStructuralItem() const {
	return IsLockedStructuralPart(componentDefID_);
}

Equipment* Equipment::CreateMockEquipment(const std::string& id, const std::string& name, int slots, float tons, EquipmentCategory category) {
	Equipment* eq = new Equipment(id, name, slots, tons);
	eq->SetCategory(category);
	eq->SetLocked(eq->IsStructuralItem());
	return eq;
}

// Helper function implementations
bool IsLockedStructuralPart(const std::string& componentDefID) {
	for (const char* part : LOCKED_STRUCTURAL_PARTS) {
		std::string partStr(part);
		// Check for exact match or prefix match (for items ending with _)
		if (partStr.back() == '_') {
			// Prefix match
			if (componentDefID.find(partStr) == 0) {
				return true;
			}
		} else {
			// Exact match
			if (componentDefID == partStr) {
				return true;
			}
		}
	}
	return false;
}

EquipmentCategory GetCategoryFromString(const std::string& categoryStr) {
	std::string lower = categoryStr;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

	if (lower == "weapon")
		return EquipmentCategory::WEAPON;
	if (lower == "heatsink")
		return EquipmentCategory::HEAT_SINK;
	if (lower == "upgrade")
		return EquipmentCategory::UPGRADE;
	if (lower == "ammo" || lower == "ammunition")
		return EquipmentCategory::AMMO;
	if (lower == "jumpjet")
		return EquipmentCategory::JUMP_JET;
	if (lower == "engine")
		return EquipmentCategory::ENGINE;
	if (lower == "gyro")
		return EquipmentCategory::GYRO;
	if (lower == "cockpit")
		return EquipmentCategory::COCKPIT;
	if (lower == "armor")
		return EquipmentCategory::ARMOR;
	if (lower == "structure")
		return EquipmentCategory::STRUCTURE;
	if (lower == "actuator")
		return EquipmentCategory::ACTUATOR;

	return EquipmentCategory::UNKNOWN;
}

std::string GetCategoryString(EquipmentCategory category) {
	switch (category) {
		case EquipmentCategory::WEAPON:
			return "Weapon";
		case EquipmentCategory::HEAT_SINK:
			return "HeatSink";
		case EquipmentCategory::UPGRADE:
			return "Upgrade";
		case EquipmentCategory::AMMO:
			return "Ammo";
		case EquipmentCategory::JUMP_JET:
			return "JumpJet";
		case EquipmentCategory::ENGINE:
			return "Engine";
		case EquipmentCategory::GYRO:
			return "Gyro";
		case EquipmentCategory::COCKPIT:
			return "Cockpit";
		case EquipmentCategory::ARMOR:
			return "Armor";
		case EquipmentCategory::STRUCTURE:
			return "Structure";
		case EquipmentCategory::ACTUATOR:
			return "Actuator";
		default:
			return "Unknown";
	}
}

} // namespace equipment
