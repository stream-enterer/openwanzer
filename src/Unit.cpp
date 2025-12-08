#include "Unit.hpp"

void Unit::initializeLocations(WeightClass wClass) {
	weightClass = wClass;

	// Simplified 5-part armor system
	// Structure is 5 for all locations and all weight classes
	// Armor varies by weight class:
	// - Front: highest armor
	// - Rear: lowest armor
	// - Left/Right/Center: equal middle armor
	switch (wClass) {
		case WeightClass::LIGHT:
			// Front 20, Rear 10, L/R/C 15, Structure 5
			locations[ArmorLocation::FRONT] = LocationStatus(20, 5);
			locations[ArmorLocation::REAR] = LocationStatus(10, 5);
			locations[ArmorLocation::LEFT] = LocationStatus(15, 5);
			locations[ArmorLocation::RIGHT] = LocationStatus(15, 5);
			locations[ArmorLocation::CENTER] = LocationStatus(15, 5);
			break;

		case WeightClass::MEDIUM:
			// Front 30, Rear 20, L/R/C 25, Structure 5
			locations[ArmorLocation::FRONT] = LocationStatus(30, 5);
			locations[ArmorLocation::REAR] = LocationStatus(20, 5);
			locations[ArmorLocation::LEFT] = LocationStatus(25, 5);
			locations[ArmorLocation::RIGHT] = LocationStatus(25, 5);
			locations[ArmorLocation::CENTER] = LocationStatus(25, 5);
			break;

		case WeightClass::HEAVY:
			// Front 40, Rear 30, L/R/C 35, Structure 5
			locations[ArmorLocation::FRONT] = LocationStatus(40, 5);
			locations[ArmorLocation::REAR] = LocationStatus(30, 5);
			locations[ArmorLocation::LEFT] = LocationStatus(35, 5);
			locations[ArmorLocation::RIGHT] = LocationStatus(35, 5);
			locations[ArmorLocation::CENTER] = LocationStatus(35, 5);
			break;

		case WeightClass::ASSAULT:
			// Front 50, Rear 40, L/R/C 45, Structure 5
			locations[ArmorLocation::FRONT] = LocationStatus(50, 5);
			locations[ArmorLocation::REAR] = LocationStatus(40, 5);
			locations[ArmorLocation::LEFT] = LocationStatus(45, 5);
			locations[ArmorLocation::RIGHT] = LocationStatus(45, 5);
			locations[ArmorLocation::CENTER] = LocationStatus(45, 5);
			break;
	}
}

bool Unit::isAlive() const {
	// Mech is destroyed when CENTER is destroyed
	return !locations.at(ArmorLocation::CENTER).isDestroyed;
}

bool Unit::canMove() const {
	return isAlive();
}

int Unit::getOverallHealthPercent() const {
	int totalCurrent = 0;
	int totalMax = 0;

	for (const auto& pair : locations) {
		const LocationStatus& status = pair.second;
		totalCurrent += status.currentArmor + status.currentStructure;
		totalMax += status.maxArmor + status.maxStructure;
	}

	if (totalMax == 0)
		return 0;
	return (totalCurrent * 100) / totalMax;
}

void Unit::initializeWeapons() {
	weapons.clear();

	// Assign weapons based on weight class for testing variety
	switch (weightClass) {
		case WeightClass::LIGHT:
			weapons.push_back(Weapon("L LASER", WeaponType::ENERGY, 5));
			weapons.push_back(Weapon("S LASER", WeaponType::ENERGY, 3));
			weapons.push_back(Weapon("SRM2", WeaponType::MISSILE, 2));
			break;

		case WeightClass::MEDIUM:
			weapons.push_back(Weapon("M LASER", WeaponType::ENERGY, 7));
			weapons.push_back(Weapon("LRM5", WeaponType::MISSILE, 5));
			weapons.push_back(Weapon("AC/5", WeaponType::BALLISTIC, 5));
			break;

		case WeightClass::HEAVY:
			weapons.push_back(Weapon("PPC", WeaponType::ENERGY, 10));
			weapons.push_back(Weapon("SRM6", WeaponType::MISSILE, 2));
			weapons.push_back(Weapon("AC/10", WeaponType::BALLISTIC, 10));
			weapons.push_back(Weapon("M LASER", WeaponType::ENERGY, 7));
			break;

		case WeightClass::ASSAULT:
			weapons.push_back(Weapon("AC/20", WeaponType::BALLISTIC, 20));
			weapons.push_back(Weapon("LRM15", WeaponType::MISSILE, 15));
			weapons.push_back(Weapon("ER L LASER", WeaponType::ENERGY, 8));
			weapons.push_back(Weapon("SRM4", WeaponType::MISSILE, 2));
			weapons.push_back(Weapon("MELEE", WeaponType::MELEE, 100));
			break;
	}
}
