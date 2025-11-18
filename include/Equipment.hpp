#ifndef OPENWANZER_EQUIPMENT_HPP
#define OPENWANZER_EQUIPMENT_HPP

#include <string>
#include <vector>

namespace equipment {

// Equipment category types
enum class EquipmentCategory {
	WEAPON,
	HEAT_SINK,
	UPGRADE,
	AMMO,
	JUMP_JET,
	ENGINE,
	GYRO,
	COCKPIT,
	ARMOR,
	STRUCTURE,
	ACTUATOR,
	UNKNOWN
};

// Equipment class representing a single piece of equipment
class Equipment {
public:
	// Constructors
	Equipment();
	Equipment(const std::string& id, const std::string& name, int slots, float tons);

	// Getters
	const std::string& GetComponentDefID() const {
		return componentDefID_;
	}
	const std::string& GetUIName() const {
		return uiName_;
	}
	const std::string& GetDetails() const {
		return details_;
	}
	int GetInventorySize() const {
		return inventorySize_;
	}
	float GetTonnage() const {
		return tonnage_;
	}
	bool IsLocked() const {
		return isLocked_;
	}
	const std::string& GetAllowedLocations() const {
		return allowedLocations_;
	}
	const std::string& GetDisallowedLocations() const {
		return disallowedLocations_;
	}
	int GetDamage() const {
		return damage_;
	}
	int GetHeat() const {
		return heat_;
	}
	EquipmentCategory GetCategory() const {
		return category_;
	}

	// Setters
	void SetComponentDefID(const std::string& id) {
		componentDefID_ = id;
	}
	void SetUIName(const std::string& name) {
		uiName_ = name;
	}
	void SetDetails(const std::string& details) {
		details_ = details;
	}
	void SetInventorySize(int size) {
		inventorySize_ = size;
	}
	void SetTonnage(float tons) {
		tonnage_ = tons;
	}
	void SetLocked(bool locked) {
		isLocked_ = locked;
	}
	void SetAllowedLocations(const std::string& locations) {
		allowedLocations_ = locations;
	}
	void SetDisallowedLocations(const std::string& locations) {
		disallowedLocations_ = locations;
	}
	void SetDamage(int dmg) {
		damage_ = dmg;
	}
	void SetHeat(int h) {
		heat_ = h;
	}
	void SetCategory(EquipmentCategory cat) {
		category_ = cat;
	}

	// Location validation
	bool CanPlaceInLocation(const std::string& location) const;

	// Check if this equipment is a structural/locked item by ID
	bool IsStructuralItem() const;

	// Static method to create equipment from mock data
	static Equipment* CreateMockEquipment(const std::string& id, const std::string& name, int slots, float tons, EquipmentCategory category);

private:
	std::string componentDefID_;      // Unique ID from JSON
	std::string uiName_;              // Display name
	std::string details_;             // Description text
	int inventorySize_;               // Number of slots (1-10+)
	float tonnage_;                   // Weight in tons
	bool isLocked_;                   // Cannot be moved/removed
	std::string allowedLocations_;    // From JSON (e.g., "All", "CenterTorso")
	std::string disallowedLocations_; // From JSON
	int damage_;                      // For weapons (0 if not applicable)
	int heat_;                        // Heat generation (0 if not applicable)
	EquipmentCategory category_;      // Equipment category
};

// Helper functions for locked items
bool IsLockedStructuralPart(const std::string& componentDefID);
EquipmentCategory GetCategoryFromString(const std::string& categoryStr);
std::string GetCategoryString(EquipmentCategory category);

} // namespace equipment

#endif // OPENWANZER_EQUIPMENT_HPP
