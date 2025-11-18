#ifndef OPENWANZER_MECH_LOADOUT_HPP
#define OPENWANZER_MECH_LOADOUT_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "Equipment.hpp"

namespace mechloadout {

// Body part slot structure
struct BodyPartSlot {
	std::string location;                         // "Head", "CenterTorso", "LeftTorso", etc.
	int maxSlots;                                 // Maximum slots available
	std::vector<equipment::Equipment*> equipment; // Ordered list of equipment

	BodyPartSlot()
	    : location(""), maxSlots(0) {
	}

	BodyPartSlot(const std::string& loc, int slots)
	    : location(loc), maxSlots(slots) {
	}

	// Get number of free slots
	int GetFreeSlots() const;

	// Get total occupied slots (accounting for multi-slot items)
	int GetOccupiedSlots() const;

	// Auto-compact: remove gaps between equipment
	void Compact();
};

// Main mech loadout class
class MechLoadout {
public:
	MechLoadout();
	~MechLoadout();

	// Initialization
	void LoadMockChassis(const std::string& chassisName, float maxTons);
	void PopulateLockedItems();

	// Inventory management
	void AddToInventory(const std::string& componentDefID, int quantity = 1);
	void RemoveFromInventory(const std::string& componentDefID);
	int GetInventoryQuantity(const std::string& componentDefID) const;
	const std::map<std::string, int>& GetInventory() const {
		return inventory_;
	}

	// Equipment database (stores actual equipment objects)
	void RegisterEquipment(equipment::Equipment* eq);
	equipment::Equipment* GetEquipmentByID(const std::string& componentDefID) const;
	const std::vector<equipment::Equipment*>& GetAllEquipment() const {
		return equipmentDatabase_;
	}

	// Equipment placement
	bool CanPlaceEquipment(equipment::Equipment* eq, const std::string& location, int slotIndex);
	bool PlaceEquipment(equipment::Equipment* eq, const std::string& location, int slotIndex);
	bool RemoveEquipment(const std::string& location, int equipmentIndex);
	bool MoveEquipment(const std::string& fromLocation, int fromIndex, const std::string& toLocation, int toIndex);

	// Body part access
	BodyPartSlot* GetBodyPart(const std::string& location);
	const std::map<std::string, BodyPartSlot>& GetAllBodyParts() const {
		return bodyParts_;
	}

	// Tonnage management
	float GetCurrentTonnage() const {
		return currentTonnage_;
	}
	float GetMaxTonnage() const {
		return maxTonnage_;
	}
	void RecalculateTonnage();

	// Chassis info
	const std::string& GetChassisName() const {
		return chassisName_;
	}
	void SetChassisName(const std::string& name) {
		chassisName_ = name;
	}

	// State management (for Apply/Cancel)
	void SaveState();
	void RestoreState();
	bool HasUnsavedChanges() const {
		return hasUnsavedChanges_;
	}
	void MarkAsChanged() {
		hasUnsavedChanges_ = true;
	}
	void ClearChangeFlag() {
		hasUnsavedChanges_ = false;
	}

private:
	std::string chassisName_;
	float currentTonnage_;
	float maxTonnage_;

	std::map<std::string, BodyPartSlot> bodyParts_;        // 8 locations
	std::map<std::string, int> inventory_;                 // componentDefID -> quantity (-1 = unlimited)
	std::vector<equipment::Equipment*> equipmentDatabase_; // All equipment objects (owned)

	// Saved state for Apply/Cancel
	std::map<std::string, BodyPartSlot> savedBodyParts_;
	std::map<std::string, int> savedInventory_;
	bool hasUnsavedChanges_;

	// Helper methods
	void InitializeBodyParts();
};

// Location name constants
const std::string LOC_HEAD = "Head";
const std::string LOC_CENTER_TORSO = "CenterTorso";
const std::string LOC_LEFT_TORSO = "LeftTorso";
const std::string LOC_RIGHT_TORSO = "RightTorso";
const std::string LOC_LEFT_ARM = "LeftArm";
const std::string LOC_RIGHT_ARM = "RightArm";
const std::string LOC_LEFT_LEG = "LeftLeg";
const std::string LOC_RIGHT_LEG = "RightLeg";

} // namespace mechloadout

#endif // OPENWANZER_MECH_LOADOUT_HPP
