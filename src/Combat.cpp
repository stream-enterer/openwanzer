#include <cstdlib>
#include <string>
#include "ArmorLocation.hpp"
#include "CombatArcs.hpp"
#include "Constants.hpp"
#include "DamageSystem.hpp"
#include "GameLogic.hpp"
#include "Hex.hpp"
#include "HitTables.hpp"
#include "Rendering.hpp"

namespace gamelogic {

void performAttack(GameState &game, Unit *attacker, Unit *defender) {
	if (!attacker || !defender)
		return;

	if (attacker->hasFired) {
		addLogMessage(game, "Unit has already fired this turn");
		return;
	}

	std::string attackerName = attacker->name + " (" + (attacker->side == 0 ? "Axis" : "Allied") + ")";
	std::string defenderName = defender->name + " (" + (defender->side == 0 ? "Axis" : "Allied") + ")";
	addLogMessage(game, "[COMBAT] " + attackerName + " fires at " + defenderName);

	// Check range
	int distance = hexDistance(attacker->position, defender->position);
	if (distance > attacker->weaponRange) {
		addLogMessage(game, "[COMBAT] Target out of range");
		return;
	}

	// Get attack arc using existing system
	Layout layout = rendering::createHexLayout(HEX_SIZE, 0, 0, 1.0f);

	OffsetCoord attackerOffset = rendering::gameCoordToOffset(attacker->position);
	OffsetCoord defenderOffset = rendering::gameCoordToOffset(defender->position);
	::Hex attackerCube = OffsetToCube(attackerOffset);
	::Hex defenderCube = OffsetToCube(defenderOffset);
	Point attackerPixel = HexToPixel(layout, attackerCube);
	Point defenderPixel = HexToPixel(layout, defenderCube);

	Vector2 attackerPos = {(float)attackerPixel.x, (float)attackerPixel.y};
	Vector2 defenderPos = {(float)defenderPixel.x, (float)defenderPixel.y};

	// Use existing getAttackArc function
	combatarcs::AttackArc arc = combatarcs::getAttackArc(attackerPos, defenderPos, defender->facing);

	std::string arcName;
	switch (arc) {
		case combatarcs::AttackArc::FRONT:
			arcName = "FRONT";
			break;
		case combatarcs::AttackArc::LEFT_SIDE:
			arcName = "LEFT SIDE";
			break;
		case combatarcs::AttackArc::RIGHT_SIDE:
			arcName = "RIGHT SIDE";
			break;
		case combatarcs::AttackArc::REAR:
			arcName = "REAR";
			break;
	}
	addLogMessage(game, "[HIT LOCATION] Attack from " + arcName + " arc");

	// 30% miss chance
	int missRoll = std::rand() % 100;
	if (missRoll < 30) {
		addLogMessage(game, "[COMBAT] MISS!");
		spawnCombatText(game, defender->position, "MISS!", false);
		attacker->hasFired = true;
		addLogMessage(game, "---");
		return;
	}

	// Roll hit location
	ArmorLocation hitLoc = hittables::rollHitLocation(arc);
	addLogMessage(game, "[HIT LOCATION] Hit: " + locationToString(hitLoc));

	// Get the structure location (rear armor maps to front structure)
	ArmorLocation structLoc = damagesystem::mapRearToFront(hitLoc);

	// Record armor/structure before damage
	int armorBefore = defender->locations[hitLoc].currentArmor;
	int structureBefore = defender->locations[structLoc].currentStructure;

	// Apply damage
	int damage = attacker->attack;
	damagesystem::applyDamageToLocation(game, defender, hitLoc, damage);

	// Calculate actual damage dealt
	int armorAfter = defender->locations[hitLoc].currentArmor;
	int structureAfter = defender->locations[structLoc].currentStructure;

	int armorDamage = armorBefore - armorAfter;
	int structureDamage = structureBefore - structureAfter;

	// Spawn combat text for damage
	if (armorDamage > 0 && structureDamage > 0) {
		// Both armor and structure damaged - spawn two numbers
		spawnCombatText(game, defender->position, std::to_string(armorDamage), false);
		spawnCombatText(game, defender->position, std::to_string(structureDamage), true);
	} else if (structureDamage > 0) {
		// Only structure damage (no armor)
		spawnCombatText(game, defender->position, std::to_string(structureDamage), true);
	} else if (armorDamage > 0) {
		// Only armor damage
		spawnCombatText(game, defender->position, std::to_string(armorDamage), false);
	}

	// Check death
	if (!defender->isAlive()) {
		addLogMessage(game, "[COMBAT RESULT] " + defenderName + " DESTROYED!");
		setUnitSpotRange(game, defender, false);
	} else {
		addLogMessage(game, "[COMBAT RESULT] " + defenderName + " damaged");
	}

	attacker->hasFired = true;
	addLogMessage(game, "---");
}

} // namespace gamelogic
