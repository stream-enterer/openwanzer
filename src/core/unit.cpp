#include "unit.h"

void Unit::initializeLocations(WeightClass wClass) {
    weightClass = wClass;

    switch (wClass) {
        case WeightClass::LIGHT: // 35 tons
            locations[ArmorLocation::HEAD] = LocationStatus(18, 6);
            locations[ArmorLocation::CENTER_TORSO] = LocationStatus(50, 18);
            locations[ArmorLocation::CENTER_TORSO_REAR] = LocationStatus(10, 0);
            locations[ArmorLocation::LEFT_TORSO] = LocationStatus(32, 14);
            locations[ArmorLocation::LEFT_TORSO_REAR] = LocationStatus(8, 0);
            locations[ArmorLocation::RIGHT_TORSO] = LocationStatus(32, 14);
            locations[ArmorLocation::RIGHT_TORSO_REAR] = LocationStatus(8, 0);
            locations[ArmorLocation::LEFT_ARM] = LocationStatus(28, 12);
            locations[ArmorLocation::RIGHT_ARM] = LocationStatus(28, 12);
            locations[ArmorLocation::LEFT_LEG] = LocationStatus(36, 14);
            locations[ArmorLocation::RIGHT_LEG] = LocationStatus(36, 14);
            break;

        case WeightClass::MEDIUM: // 50 tons
            locations[ArmorLocation::HEAD] = LocationStatus(18, 8);
            locations[ArmorLocation::CENTER_TORSO] = LocationStatus(80, 26);
            locations[ArmorLocation::CENTER_TORSO_REAR] = LocationStatus(16, 0);
            locations[ArmorLocation::LEFT_TORSO] = LocationStatus(52, 20);
            locations[ArmorLocation::LEFT_TORSO_REAR] = LocationStatus(12, 0);
            locations[ArmorLocation::RIGHT_TORSO] = LocationStatus(52, 20);
            locations[ArmorLocation::RIGHT_TORSO_REAR] = LocationStatus(12, 0);
            locations[ArmorLocation::LEFT_ARM] = LocationStatus(40, 16);
            locations[ArmorLocation::RIGHT_ARM] = LocationStatus(40, 16);
            locations[ArmorLocation::LEFT_LEG] = LocationStatus(52, 20);
            locations[ArmorLocation::RIGHT_LEG] = LocationStatus(52, 20);
            break;

        case WeightClass::HEAVY: // 70 tons
            locations[ArmorLocation::HEAD] = LocationStatus(18, 10);
            locations[ArmorLocation::CENTER_TORSO] = LocationStatus(100, 36);
            locations[ArmorLocation::CENTER_TORSO_REAR] = LocationStatus(24, 0);
            locations[ArmorLocation::LEFT_TORSO] = LocationStatus(64, 28);
            locations[ArmorLocation::LEFT_TORSO_REAR] = LocationStatus(16, 0);
            locations[ArmorLocation::RIGHT_TORSO] = LocationStatus(64, 28);
            locations[ArmorLocation::RIGHT_TORSO_REAR] = LocationStatus(16, 0);
            locations[ArmorLocation::LEFT_ARM] = LocationStatus(52, 24);
            locations[ArmorLocation::RIGHT_ARM] = LocationStatus(52, 24);
            locations[ArmorLocation::LEFT_LEG] = LocationStatus(64, 28);
            locations[ArmorLocation::RIGHT_LEG] = LocationStatus(64, 28);
            break;

        case WeightClass::ASSAULT: // 100 tons
            locations[ArmorLocation::HEAD] = LocationStatus(18, 12);
            locations[ArmorLocation::CENTER_TORSO] = LocationStatus(120, 50);
            locations[ArmorLocation::CENTER_TORSO_REAR] = LocationStatus(36, 0);
            locations[ArmorLocation::LEFT_TORSO] = LocationStatus(80, 40);
            locations[ArmorLocation::LEFT_TORSO_REAR] = LocationStatus(24, 0);
            locations[ArmorLocation::RIGHT_TORSO] = LocationStatus(80, 40);
            locations[ArmorLocation::RIGHT_TORSO_REAR] = LocationStatus(24, 0);
            locations[ArmorLocation::LEFT_ARM] = LocationStatus(64, 34);
            locations[ArmorLocation::RIGHT_ARM] = LocationStatus(64, 34);
            locations[ArmorLocation::LEFT_LEG] = LocationStatus(80, 40);
            locations[ArmorLocation::RIGHT_LEG] = LocationStatus(80, 40);
            break;
    }
}

bool Unit::isAlive() const {
    if (locations.at(ArmorLocation::HEAD).isDestroyed) return false;
    if (locations.at(ArmorLocation::CENTER_TORSO).isDestroyed) return false;

    if (locations.at(ArmorLocation::LEFT_LEG).isDestroyed &&
        locations.at(ArmorLocation::RIGHT_LEG).isDestroyed) return false;

    return true;
}

bool Unit::canMove() const {
    if (!isAlive()) return false;

    bool leftLegGone = locations.at(ArmorLocation::LEFT_LEG).isDestroyed;
    bool rightLegGone = locations.at(ArmorLocation::RIGHT_LEG).isDestroyed;

    return !(leftLegGone && rightLegGone);
}

int Unit::getOverallHealthPercent() const {
    int totalCurrent = 0;
    int totalMax = 0;

    for (const auto& pair : locations) {
        const LocationStatus& status = pair.second;
        totalCurrent += status.currentArmor + status.currentStructure;
        totalMax += status.maxArmor + status.maxStructure;
    }

    if (totalMax == 0) return 0;
    return (totalCurrent * 100) / totalMax;
}
