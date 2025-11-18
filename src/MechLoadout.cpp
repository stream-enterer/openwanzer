#include "MechLoadout.hpp"
#include <algorithm>

namespace mechloadout {

// BodyPartSlot implementations
int BodyPartSlot::GetFreeSlots() const {
	return maxSlots - GetOccupiedSlots();
}

int BodyPartSlot::GetOccupiedSlots() const {
	int occupied = 0;
	for (const auto* eq : equipment) {
		if (eq != nullptr) {
			occupied += eq->GetInventorySize();
		}
	}
	return occupied;
}

void BodyPartSlot::Compact() {
	// Remove any nullptr entries
	equipment.erase(std::remove(equipment.begin(), equipment.end(), nullptr), equipment.end());
}

// MechLoadout implementations
MechLoadout::MechLoadout()
    : chassisName_(""),
      currentTonnage_(0.0f),
      maxTonnage_(100.0f),
      hasUnsavedChanges_(false) {
	InitializeBodyParts();
}

MechLoadout::~MechLoadout() {
	// Clean up equipment database
	for (auto* eq : equipmentDatabase_) {
		delete eq;
	}
	equipmentDatabase_.clear();
}

void MechLoadout::InitializeBodyParts() {
	// Initialize all 8 body parts with default slot counts
	// These will be overridden when loading a specific chassis
	bodyParts_[LOC_HEAD] = BodyPartSlot(LOC_HEAD, 6);
	bodyParts_[LOC_CENTER_TORSO] = BodyPartSlot(LOC_CENTER_TORSO, 12);
	bodyParts_[LOC_LEFT_TORSO] = BodyPartSlot(LOC_LEFT_TORSO, 12);
	bodyParts_[LOC_RIGHT_TORSO] = BodyPartSlot(LOC_RIGHT_TORSO, 12);
	bodyParts_[LOC_LEFT_ARM] = BodyPartSlot(LOC_LEFT_ARM, 12);
	bodyParts_[LOC_RIGHT_ARM] = BodyPartSlot(LOC_RIGHT_ARM, 12);
	bodyParts_[LOC_LEFT_LEG] = BodyPartSlot(LOC_LEFT_LEG, 6);
	bodyParts_[LOC_RIGHT_LEG] = BodyPartSlot(LOC_RIGHT_LEG, 6);
}

void MechLoadout::LoadMockChassis(const std::string& chassisName, float maxTons) {
	chassisName_ = chassisName;
	maxTonnage_ = maxTons;

	// For Blackjack BJ-1, set specific slot counts
	if (chassisName == "Blackjack BJ-1" || chassisName == "BJ-1") {
		bodyParts_[LOC_HEAD].maxSlots = 6;
		bodyParts_[LOC_CENTER_TORSO].maxSlots = 12;
		bodyParts_[LOC_LEFT_TORSO].maxSlots = 12;
		bodyParts_[LOC_RIGHT_TORSO].maxSlots = 12;
		bodyParts_[LOC_LEFT_ARM].maxSlots = 12;
		bodyParts_[LOC_RIGHT_ARM].maxSlots = 12;
		bodyParts_[LOC_LEFT_LEG].maxSlots = 6;
		bodyParts_[LOC_RIGHT_LEG].maxSlots = 6;
		maxTonnage_ = 45.0f;
	}

	// Populate locked structural items
	PopulateLockedItems();
	RecalculateTonnage();
}

void MechLoadout::PopulateLockedItems() {
	using namespace equipment;

	// Create and place locked structural items
	// HEAD
	Equipment* sensors = Equipment::CreateMockEquipment("Gear_Cockpit_Generic_Standard", "SENSORS", 1, 0.0f, EquipmentCategory::COCKPIT);
	sensors->SetLocked(true);
	RegisterEquipment(sensors);
	PlaceEquipment(sensors, LOC_HEAD, 0);

	Equipment* lifeSupport = Equipment::CreateMockEquipment("Gear_Cockpit_LifeSupport", "LIFE SUPPORT", 1, 0.0f, EquipmentCategory::COCKPIT);
	lifeSupport->SetLocked(true);
	RegisterEquipment(lifeSupport);
	PlaceEquipment(lifeSupport, LOC_HEAD, 1);

	Equipment* cockpit = Equipment::CreateMockEquipment("Gear_Cockpit_Generic_Standard_Cockpit", "COCKPIT", 1, 0.0f, EquipmentCategory::COCKPIT);
	cockpit->SetLocked(true);
	RegisterEquipment(cockpit);
	PlaceEquipment(cockpit, LOC_HEAD, 2);

	// CENTER TORSO
	Equipment* engine1 = Equipment::CreateMockEquipment("emod_engine_180", "ENGINE", 1, 8.5f, EquipmentCategory::ENGINE);
	engine1->SetLocked(true);
	RegisterEquipment(engine1);
	PlaceEquipment(engine1, LOC_CENTER_TORSO, 0);

	Equipment* engine2 = Equipment::CreateMockEquipment("emod_engine_180_core", "ENGINE", 1, 0.0f, EquipmentCategory::ENGINE);
	engine2->SetLocked(true);
	RegisterEquipment(engine2);
	PlaceEquipment(engine2, LOC_CENTER_TORSO, 1);

	Equipment* engine3 = Equipment::CreateMockEquipment("emod_engine_180_core2", "ENGINE", 1, 0.0f, EquipmentCategory::ENGINE);
	engine3->SetLocked(true);
	RegisterEquipment(engine3);
	PlaceEquipment(engine3, LOC_CENTER_TORSO, 2);

	Equipment* gyro = Equipment::CreateMockEquipment("Gear_Gyro_Generic_Standard", "GYRO", 1, 2.0f, EquipmentCategory::GYRO);
	gyro->SetLocked(true);
	RegisterEquipment(gyro);
	PlaceEquipment(gyro, LOC_CENTER_TORSO, 3);

	// ARMS - Actuators
	for (int i = 0; i < 2; i++) {
		std::string armLoc = (i == 0) ? LOC_LEFT_ARM : LOC_RIGHT_ARM;

		Equipment* shoulder = Equipment::CreateMockEquipment("emod_arm_part_shoulder", "SHOULDER", 1, 0.0f, EquipmentCategory::ACTUATOR);
		shoulder->SetLocked(true);
		RegisterEquipment(shoulder);
		PlaceEquipment(shoulder, armLoc, 0);

		Equipment* upper = Equipment::CreateMockEquipment("emod_arm_part_upper", "UPPER ARM", 1, 0.0f, EquipmentCategory::ACTUATOR);
		upper->SetLocked(true);
		RegisterEquipment(upper);
		PlaceEquipment(upper, armLoc, 1);

		Equipment* lower = Equipment::CreateMockEquipment("emod_arm_part_lower", "LOWER ARM", 1, 0.0f, EquipmentCategory::ACTUATOR);
		lower->SetLocked(true);
		RegisterEquipment(lower);
		PlaceEquipment(lower, armLoc, 2);

		Equipment* hand = Equipment::CreateMockEquipment("emod_arm_part_hand", "HAND", 1, 0.0f, EquipmentCategory::ACTUATOR);
		hand->SetLocked(true);
		RegisterEquipment(hand);
		PlaceEquipment(hand, armLoc, 3);
	}

	// LEGS - Actuators
	for (int i = 0; i < 2; i++) {
		std::string legLoc = (i == 0) ? LOC_LEFT_LEG : LOC_RIGHT_LEG;

		Equipment* hip = Equipment::CreateMockEquipment("emod_leg_hip", "HIP", 1, 0.0f, EquipmentCategory::ACTUATOR);
		hip->SetLocked(true);
		RegisterEquipment(hip);
		PlaceEquipment(hip, legLoc, 0);

		Equipment* upperLeg = Equipment::CreateMockEquipment("emod_leg_upper", "UPPER LEG", 1, 0.0f, EquipmentCategory::ACTUATOR);
		upperLeg->SetLocked(true);
		RegisterEquipment(upperLeg);
		PlaceEquipment(upperLeg, legLoc, 1);

		Equipment* lowerLeg = Equipment::CreateMockEquipment("emod_leg_lower", "LOWER LEG", 1, 0.0f, EquipmentCategory::ACTUATOR);
		lowerLeg->SetLocked(true);
		RegisterEquipment(lowerLeg);
		PlaceEquipment(lowerLeg, legLoc, 2);

		Equipment* foot = Equipment::CreateMockEquipment("emod_leg_foot", "FOOT", 1, 0.0f, EquipmentCategory::ACTUATOR);
		foot->SetLocked(true);
		RegisterEquipment(foot);
		PlaceEquipment(foot, legLoc, 3);
	}

	RecalculateTonnage();
}

void MechLoadout::AddToInventory(const std::string& componentDefID, int quantity) {
	if (inventory_.find(componentDefID) != inventory_.end()) {
		// Already exists, add to quantity (unless unlimited)
		if (inventory_[componentDefID] != -1) {
			inventory_[componentDefID] += quantity;
		}
	} else {
		inventory_[componentDefID] = quantity;
	}
	MarkAsChanged();
}

void MechLoadout::RemoveFromInventory(const std::string& componentDefID) {
	auto it = inventory_.find(componentDefID);
	if (it != inventory_.end()) {
		if (it->second != -1) { // Not unlimited
			it->second--;
			if (it->second <= 0) {
				inventory_.erase(it);
			}
		}
	}
	MarkAsChanged();
}

int MechLoadout::GetInventoryQuantity(const std::string& componentDefID) const {
	auto it = inventory_.find(componentDefID);
	if (it != inventory_.end()) {
		return it->second;
	}
	return 0;
}

void MechLoadout::RegisterEquipment(equipment::Equipment* eq) {
	if (eq != nullptr) {
		equipmentDatabase_.push_back(eq);
	}
}

equipment::Equipment* MechLoadout::GetEquipmentByID(const std::string& componentDefID) const {
	for (auto* eq : equipmentDatabase_) {
		if (eq->GetComponentDefID() == componentDefID) {
			return eq;
		}
	}
	return nullptr;
}

bool MechLoadout::CanPlaceEquipment(equipment::Equipment* eq, const std::string& location, int slotIndex) {
	if (eq == nullptr)
		return false;

	// Get body part
	auto it = bodyParts_.find(location);
	if (it == bodyParts_.end())
		return false;

	BodyPartSlot& slot = it->second;

	// Check location restrictions
	if (!eq->CanPlaceInLocation(location)) {
		return false;
	}

	// Check if slot index is valid
	if (slotIndex < 0 || slotIndex > (int)slot.equipment.size()) {
		return false;
	}

	// If inserting at an existing item, check if it's locked
	if (slotIndex < (int)slot.equipment.size()) {
		if (slot.equipment[slotIndex] != nullptr && slot.equipment[slotIndex]->IsLocked()) {
			return false; // Cannot insert at locked item position
		}
	}

	// Calculate available space
	// We need enough consecutive free slots from the insertion point
	int requiredSlots = eq->GetInventorySize();

	// Count free slots after insertion point (accounting for pushing items down)
	int totalFree = slot.GetFreeSlots();

	// Must have enough free slots total
	if (totalFree < requiredSlots) {
		return false;
	}

	return true;
}

bool MechLoadout::PlaceEquipment(equipment::Equipment* eq, const std::string& location, int slotIndex) {
	if (!CanPlaceEquipment(eq, location, slotIndex)) {
		return false;
	}

	auto it = bodyParts_.find(location);
	if (it == bodyParts_.end())
		return false;

	BodyPartSlot& slot = it->second;

	// Insert equipment at the specified index
	if (slotIndex >= (int)slot.equipment.size()) {
		slot.equipment.push_back(eq);
	} else {
		slot.equipment.insert(slot.equipment.begin() + slotIndex, eq);
	}

	// Compact the list (remove nullptr gaps)
	slot.Compact();

	RecalculateTonnage();
	MarkAsChanged();
	return true;
}

bool MechLoadout::RemoveEquipment(const std::string& location, int equipmentIndex) {
	auto it = bodyParts_.find(location);
	if (it == bodyParts_.end())
		return false;

	BodyPartSlot& slot = it->second;

	if (equipmentIndex < 0 || equipmentIndex >= (int)slot.equipment.size())
		return false;

	equipment::Equipment* eq = slot.equipment[equipmentIndex];
	if (eq == nullptr)
		return false;

	// Cannot remove locked items
	if (eq->IsLocked())
		return false;

	// Remove from slot
	slot.equipment.erase(slot.equipment.begin() + equipmentIndex);

	// Add back to inventory
	AddToInventory(eq->GetComponentDefID(), 1);

	// Compact the list
	slot.Compact();

	RecalculateTonnage();
	MarkAsChanged();
	return true;
}

bool MechLoadout::MoveEquipment(const std::string& fromLocation, int fromIndex, const std::string& toLocation, int toIndex) {
	// Get source equipment
	auto fromIt = bodyParts_.find(fromLocation);
	if (fromIt == bodyParts_.end())
		return false;

	BodyPartSlot& fromSlot = fromIt->second;
	if (fromIndex < 0 || fromIndex >= (int)fromSlot.equipment.size())
		return false;

	equipment::Equipment* eq = fromSlot.equipment[fromIndex];
	if (eq == nullptr || eq->IsLocked())
		return false;

	// Check if we can place at destination
	if (!CanPlaceEquipment(eq, toLocation, toIndex))
		return false;

	// Remove from source
	fromSlot.equipment.erase(fromSlot.equipment.begin() + fromIndex);
	fromSlot.Compact();

	// Place at destination
	auto toIt = bodyParts_.find(toLocation);
	if (toIt == bodyParts_.end())
		return false;

	BodyPartSlot& toSlot = toIt->second;
	if (toIndex >= (int)toSlot.equipment.size()) {
		toSlot.equipment.push_back(eq);
	} else {
		toSlot.equipment.insert(toSlot.equipment.begin() + toIndex, eq);
	}
	toSlot.Compact();

	RecalculateTonnage();
	MarkAsChanged();
	return true;
}

BodyPartSlot* MechLoadout::GetBodyPart(const std::string& location) {
	auto it = bodyParts_.find(location);
	if (it != bodyParts_.end()) {
		return &(it->second);
	}
	return nullptr;
}

void MechLoadout::RecalculateTonnage() {
	currentTonnage_ = 0.0f;

	// Sum up all equipment tonnage
	for (const auto& pair : bodyParts_) {
		const BodyPartSlot& slot = pair.second;
		for (const auto* eq : slot.equipment) {
			if (eq != nullptr) {
				currentTonnage_ += eq->GetTonnage();
			}
		}
	}
}

void MechLoadout::SaveState() {
	// Deep copy body parts
	savedBodyParts_.clear();
	for (const auto& pair : bodyParts_) {
		savedBodyParts_[pair.first] = pair.second; // Shallow copy is fine (equipment pointers are shared)
	}

	// Copy inventory
	savedInventory_ = inventory_;

	hasUnsavedChanges_ = false;
}

void MechLoadout::RestoreState() {
	// Restore body parts
	bodyParts_.clear();
	for (const auto& pair : savedBodyParts_) {
		bodyParts_[pair.first] = pair.second;
	}

	// Restore inventory
	inventory_ = savedInventory_;

	RecalculateTonnage();
	hasUnsavedChanges_ = false;
}

} // namespace mechloadout
