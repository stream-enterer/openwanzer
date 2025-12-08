#include "DamageSystem.hpp"
#include "PaperdollUI.hpp"

#include <algorithm>
#include "GameLogic.hpp"

namespace damagesystem {

ArmorLocation getTransferLocation(ArmorLocation destroyed) {
	// In the simplified 5-part system, all non-CENTER locations overflow to CENTER
	switch (destroyed) {
		case ArmorLocation::FRONT:
		case ArmorLocation::REAR:
		case ArmorLocation::LEFT:
		case ArmorLocation::RIGHT:
			return ArmorLocation::CENTER;
		default:
			// CENTER has no transfer - destroying it destroys the mech
			return ArmorLocation::NONE;
	}
}

void applyDamageToLocation(GameState& game, Unit* target, ArmorLocation location, int damage) {
	LocationStatus& loc = target->locations[location];

	gamelogic::addLogMessage(game, "[DAMAGE] " + std::to_string(damage) + " damage to " + locationToString(location));

	// Trigger hit flash on paperdoll panel
	paperdollui::triggerHitFlash(game, target, location);

	int remainingDamage = damage;

	// Apply to armor
	if (loc.currentArmor > 0) {
		int armorAbsorbed = std::min(loc.currentArmor, remainingDamage);
		loc.currentArmor -= armorAbsorbed;
		remainingDamage -= armorAbsorbed;

		gamelogic::addLogMessage(game, "[DAMAGE] " + locationToString(location) + " armor: " + std::to_string(loc.currentArmor + armorAbsorbed) + " -> " + std::to_string(loc.currentArmor) + " (absorbed " + std::to_string(armorAbsorbed) + ")");

		if (remainingDamage > 0) {
			gamelogic::addLogMessage(game, "[DAMAGE] " + std::to_string(remainingDamage) + " overflow to structure");
		}
	}

	// Apply to structure
	if (remainingDamage > 0 && loc.currentStructure > 0) {
		int structureAbsorbed = std::min(loc.currentStructure, remainingDamage);
		loc.currentStructure -= structureAbsorbed;
		remainingDamage -= structureAbsorbed;

		gamelogic::addLogMessage(game, "[DAMAGE] " + locationToString(location) + " structure: " + std::to_string(loc.currentStructure + structureAbsorbed) + " -> " + std::to_string(loc.currentStructure) + " (absorbed " + std::to_string(structureAbsorbed) + ")");
	}

	// Check for location destruction
	if (loc.currentStructure <= 0 && !loc.isDestroyed) {
		loc.isDestroyed = true;
		gamelogic::addLogMessage(game, "[LOCATION DESTROYED] " + locationToString(location) + " destroyed!");

		// Death condition: CENTER destroyed
		if (location == ArmorLocation::CENTER) {
			gamelogic::addLogMessage(game, "[MECH DEATH] Center destroyed!");
			return;
		}

		// Transfer overflow damage to CENTER
		if (remainingDamage > 0) {
			ArmorLocation transferLocation = getTransferLocation(location);
			if (transferLocation != ArmorLocation::NONE) {
				gamelogic::addLogMessage(game, "[DAMAGE TRANSFER] " + std::to_string(remainingDamage) + " overflow -> " + locationToString(transferLocation));
				applyDamageToLocation(game, target, transferLocation, remainingDamage);
			}
		}
	}

	// Log final status
	gamelogic::addLogMessage(game, "[LOCATION STATUS] " + locationToString(location) + ": " + std::to_string(loc.currentArmor) + "/" + std::to_string(loc.maxArmor) + " armor, " + std::to_string(loc.currentStructure) + "/" + std::to_string(loc.maxStructure) + " structure");
}

} // namespace damagesystem
