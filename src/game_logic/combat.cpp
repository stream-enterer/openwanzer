#include "game_logic.h"
#include <algorithm>
#include <string>

namespace GameLogic {

// Calculate kills using simplified formula
int calculateKills(int atkVal, int defVal, const Unit *attacker, const Unit *defender) {
  int killFactor = atkVal - defVal + (rand() % 11 - 5); // +/- 5 random
  if (killFactor < 1) killFactor = 1;
  return (killFactor * attacker->strength) / 10;
}

void performAttack(GameState &game, Unit *attacker, Unit *defender) {
  if (!attacker || !defender)
    return;

  if (attacker->hasFired) {
    addLogMessage(game, "Unit has already fired this turn");
    return;
  }

  // Log combat initiation
  std::string attackerName = attacker->name + " (" + (attacker->side == 0 ? "Axis" : "Allied") + ")";
  std::string defenderName = defender->name + " (" + (defender->side == 0 ? "Axis" : "Allied") + ")";
  addLogMessage(game, attackerName + " attacks " + defenderName);

  // Get distance for return fire check
  int distance = hexDistance(attacker->position, defender->position);

  // Simple attack/defense values
  int aav = attacker->attack;
  int adv = attacker->defense;
  int dav = defender->attack;
  int ddv = defender->defense;

  // Calculate kills
  int kills = calculateKills(aav, ddv, attacker, defender);

  // Defender can fire back if:
  // - At range 1 (close combat), OR
  // - Both are sea units (naval combat)
  bool defCanFire = (distance <= 1 || (isSea(attacker) && isSea(defender)));
  int losses = 0;

  if (defCanFire) {
    losses = calculateKills(dav, adv, defender, attacker);
  }

  // Apply damage
  defender->strength = std::max(0, defender->strength - kills);
  attacker->strength = std::max(0, attacker->strength - losses);

  // Log damage
  if (kills > 0) {
    addLogMessage(game, attackerName + " deals " + std::to_string(kills) + " damage to " + defenderName);
  }
  if (losses > 0 && defCanFire) {
    addLogMessage(game, defenderName + " returns fire, dealing " + std::to_string(losses) + " damage");
  }

  // Mark as fired
  attacker->hasFired = true;

  // Clear spotting for units about to be destroyed, and log destruction
  for (auto &unit : game.units) {
    if (unit->strength <= 0) {
      setUnitSpotRange(game, unit.get(), false);

      // Log unit destruction
      std::string unitName = unit->name + " (" + (unit->side == 0 ? "Axis" : "Allied") + ")";
      addLogMessage(game, unitName + " destroyed!");
    }
  }

  // Remove destroyed units
  game.units.erase(std::remove_if(game.units.begin(), game.units.end(),
                                  [](const std::unique_ptr<Unit> &u) {
                                    return u->strength <= 0;
                                  }),
                   game.units.end());
}

} // namespace GameLogic
