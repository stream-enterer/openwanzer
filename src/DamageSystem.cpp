#include "DamageSystem.hpp"
#include "GameLogic.hpp"
#include <algorithm>

namespace damagesystem {

ArmorLocation mapRearToFront(ArmorLocation rear) {
    switch (rear) {
        case ArmorLocation::CENTER_TORSO_REAR: return ArmorLocation::CENTER_TORSO;
        case ArmorLocation::LEFT_TORSO_REAR: return ArmorLocation::LEFT_TORSO;
        case ArmorLocation::RIGHT_TORSO_REAR: return ArmorLocation::RIGHT_TORSO;
        default: return rear;
    }
}

ArmorLocation getTransferLocation(ArmorLocation destroyed) {
    switch (destroyed) {
        case ArmorLocation::LEFT_ARM:
        case ArmorLocation::LEFT_LEG:
            return ArmorLocation::LEFT_TORSO;
        case ArmorLocation::RIGHT_ARM:
        case ArmorLocation::RIGHT_LEG:
            return ArmorLocation::RIGHT_TORSO;
        case ArmorLocation::LEFT_TORSO:
        case ArmorLocation::RIGHT_TORSO:
        case ArmorLocation::LEFT_TORSO_REAR:
        case ArmorLocation::RIGHT_TORSO_REAR:
            return ArmorLocation::CENTER_TORSO;
        default:
            return ArmorLocation::NONE;
    }
}

void applyDamageToLocation(GameState& game, Unit* target, ArmorLocation location, int damage) {
    ArmorLocation structLoc = mapRearToFront(location);
    LocationStatus& loc = target->locations[location];
    LocationStatus& structLocation = target->locations[structLoc];

    gamelogic::addLogMessage(game, "[DAMAGE] " + std::to_string(damage) +
                              " damage to " + locationToString(location));

    int remainingDamage = damage;

    // Apply to armor
    if (loc.currentArmor > 0) {
        int armorAbsorbed = std::min(loc.currentArmor, remainingDamage);
        loc.currentArmor -= armorAbsorbed;
        remainingDamage -= armorAbsorbed;

        gamelogic::addLogMessage(game, "[DAMAGE] " + locationToString(location) +
                                  " armor: " + std::to_string(loc.currentArmor + armorAbsorbed) +
                                  " → " + std::to_string(loc.currentArmor) +
                                  " (absorbed " + std::to_string(armorAbsorbed) + ")");

        if (remainingDamage > 0) {
            gamelogic::addLogMessage(game, "[DAMAGE] " + std::to_string(remainingDamage) +
                                      " overflow to structure");
        }
    }

    // Apply to structure
    if (remainingDamage > 0 && structLocation.currentStructure > 0) {
        int structureAbsorbed = std::min(structLocation.currentStructure, remainingDamage);
        structLocation.currentStructure -= structureAbsorbed;
        remainingDamage -= structureAbsorbed;

        gamelogic::addLogMessage(game, "[DAMAGE] " + locationToString(structLoc) +
                                  " structure: " + std::to_string(structLocation.currentStructure + structureAbsorbed) +
                                  " → " + std::to_string(structLocation.currentStructure) +
                                  " (absorbed " + std::to_string(structureAbsorbed) + ")");
    }

    // Check for location destruction
    if (structLocation.currentStructure <= 0 && !structLocation.isDestroyed) {
        structLocation.isDestroyed = true;
        gamelogic::addLogMessage(game, "[LOCATION DESTROYED] " +
                                  locationToString(structLoc) + " destroyed!");

        // Death conditions
        if (structLoc == ArmorLocation::HEAD ||
            structLoc == ArmorLocation::CENTER_TORSO) {
            gamelogic::addLogMessage(game, "[MECH DEATH] Critical location destroyed!");
            return;
        }

        // Both legs destroyed
        if (target->locations[ArmorLocation::LEFT_LEG].isDestroyed &&
            target->locations[ArmorLocation::RIGHT_LEG].isDestroyed) {
            gamelogic::addLogMessage(game, "[MECH DEATH] Both legs destroyed!");
            return;
        }

        // Single leg loss
        if (structLoc == ArmorLocation::LEFT_LEG ||
            structLoc == ArmorLocation::RIGHT_LEG) {
            target->movementPoints = 1;
            target->movesLeft = std::min(target->movesLeft, 1);
            gamelogic::addLogMessage(game, "[MOVEMENT] Leg destroyed - reduced to 1 MP");
        }

        // Transfer overflow
        if (remainingDamage > 0) {
            ArmorLocation transferLocation = getTransferLocation(structLoc);
            if (transferLocation != ArmorLocation::NONE) {
                gamelogic::addLogMessage(game, "[DAMAGE TRANSFER] " + std::to_string(remainingDamage) +
                                          " overflow → " + locationToString(transferLocation));
                applyDamageToLocation(game, target, transferLocation, remainingDamage);
            }
        }
    }

    // Log final status
    gamelogic::addLogMessage(game, "[LOCATION STATUS] " + locationToString(location) + ": " +
                              std::to_string(loc.currentArmor) + "/" + std::to_string(loc.maxArmor) +
                              " armor, " + std::to_string(structLocation.currentStructure) + "/" +
                              std::to_string(structLocation.maxStructure) + " structure");
}

} // namespace damagesystem
