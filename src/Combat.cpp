#include "GameLogic.hpp"
#include "DamageSystem.hpp"
#include "HitTables.hpp"
#include "CombatArcs.hpp"
#include "ArmorLocation.hpp"
#include "Constants.hpp"
#include "Rendering.hpp"
#include "hex.h"
#include <string>

namespace gamelogic {

void performAttack(GameState &game, Unit *attacker, Unit *defender) {
  if (!attacker || !defender) return;

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
  ::Hex attackerCube = offset_to_cube(attackerOffset);
  ::Hex defenderCube = offset_to_cube(defenderOffset);
  Point attackerPixel = HexToPixel(layout, attackerCube);
  Point defenderPixel = HexToPixel(layout, defenderCube);

  Vector2 attackerPos = {(float)attackerPixel.x, (float)attackerPixel.y};
  Vector2 defenderPos = {(float)defenderPixel.x, (float)defenderPixel.y};

  // Use existing getAttackArc function
  combatarcs::AttackArc arc = combatarcs::getAttackArc(attackerPos, defenderPos, defender->facing);

  std::string arcName;
  switch (arc) {
    case combatarcs::AttackArc::FRONT: arcName = "FRONT"; break;
    case combatarcs::AttackArc::LEFT_SIDE: arcName = "LEFT SIDE"; break;
    case combatarcs::AttackArc::RIGHT_SIDE: arcName = "RIGHT SIDE"; break;
    case combatarcs::AttackArc::REAR: arcName = "REAR"; break;
  }
  addLogMessage(game, "[HIT LOCATION] Attack from " + arcName + " arc");

  // Roll hit location
  ArmorLocation hitLoc = hittables::rollHitLocation(arc);
  addLogMessage(game, "[HIT LOCATION] Hit: " + locationToString(hitLoc));

  // Apply damage
  int damage = attacker->attack;
  damagesystem::applyDamageToLocation(game, defender, hitLoc, damage);

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
