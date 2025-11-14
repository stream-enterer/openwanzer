#include "game_logic.h"
#include <algorithm>
#include <string>

namespace GameLogic {

// Calculate kills using PG2 formula
int calculateKills(int atkVal, int defVal, const Unit *attacker, const Unit *defender) {
  int kF = atkVal - defVal;

  // PG2 formula: compress high values
  if (kF > 4) {
    kF = 4 + (2 * kF - 8) / 5;
  }
  kF += 6;

  // Artillery/Bomber penalty (less effective at killing)
  if (attacker->unitClass == UnitClass::ARTILLERY) {
    kF -= 3;
  }

  // Clamp kill factor between 1 and 19
  kF = std::max(1, std::min(19, kF));

  // Calculate kills based on attacker strength
  // Formula: (5 * kF * strength + 50) / 100 = (kF * strength) / 20 + 0.5
  int kills = (5 * kF * attacker->strength + 50) / 100;

  return kills;
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

  // Get distance and hex information
  int distance = hexDistance(attacker->position, defender->position);
  GameHex &atkHex = game.map[attacker->position.row][attacker->position.col];
  GameHex &defHex = game.map[defender->position.row][defender->position.col];

  // Determine attack/defense values based on target type
  int aav, adv, dav, ddv;

  // Attacker attack value (use hard attack for armored targets)
  if (isHardTarget(defender)) {
    aav = attacker->hardAttack;
  } else {
    aav = attacker->softAttack;
  }

  // Attacker defense value
  adv = attacker->groundDefense;

  // Defender attack value (for return fire)
  if (isHardTarget(attacker)) {
    dav = defender->hardAttack;
  } else {
    dav = defender->softAttack;
  }

  // Defender defense value
  ddv = defender->groundDefense;

  // 1. Apply experience modifiers (+1 per experience bar)
  int aExpBars = attacker->experience / 100;
  int dExpBars = defender->experience / 100;
  aav += aExpBars;
  adv += aExpBars;
  dav += dExpBars;
  ddv += dExpBars;

  // 2. Apply entrenchment (adds to defense only)
  adv += attacker->entrenchment;
  ddv += defender->entrenchment;

  // 3. Apply terrain modifiers
  // Cities give +4 defense
  if (defHex.terrain == TerrainType::CITY) {
    ddv += 4;
  }
  if (atkHex.terrain == TerrainType::CITY) {
    adv += 4;
  }

  // Water without road: -4 defense, attacker gets +4 attack
  if (defHex.terrain == TerrainType::WATER) {
    ddv -= 4;
    aav += 4;
  }
  if (atkHex.terrain == TerrainType::WATER) {
    adv -= 4;
    dav += 4;
  }

  // 4. Apply initiative bonus (who shoots first gets advantage)
  int initDiff = attacker->initiative - defender->initiative;
  if (initDiff >= 0) {
    // Attacker has initiative
    adv += 4;  // Attacker defense bonus
    aav += std::min(4, initDiff);  // Attack bonus (max +4)
  } else {
    // Defender has initiative
    ddv += 4;  // Defender defense bonus
    dav += std::min(4, -initDiff);  // Defender attack bonus (max +4)
  }

  // 5. Apply range defense modifier (for ranged combat)
  if (distance > 1) {
    adv += attacker->rangeDefMod;
    ddv += defender->rangeDefMod;
  }

  // 6. Apply accumulated hits (reduces defense)
  adv -= attacker->hits;
  ddv -= defender->hits;

  // Calculate kills
  int kills = calculateKills(aav, ddv, attacker, defender);

  // Defender can fire back if:
  // - At range 1 (close combat), OR
  // - Both are sea units (naval combat)
  bool defCanFire = (distance <= 1 || (isSea(attacker) && isSea(defender)));
  int losses = 0;

  if (defCanFire && defender->ammo > 0) {
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

  // Experience gain
  // Attacker gains based on defender's attack value and kills
  int bonusAD = std::max(1, dav + 6 - adv);
  int atkExpGain = (bonusAD * (defender->maxStrength / 10) + bonusAD) * kills;
  attacker->experience = std::min(500, attacker->experience + atkExpGain);

  // Defender gains 2 * losses
  if (defCanFire) {
    int defExpGain = 2 * losses;
    defender->experience = std::min(500, defender->experience + defExpGain);
  }

  // Mark as fired and consume ammo
  attacker->hasFired = true;
  attacker->ammo = std::max(0, attacker->ammo - 1);

  // Increment hits (reduces future defense)
  attacker->hits++;
  defender->hits++;

  // Reduce entrenchment on hit
  if (kills > 0 && defender->entrenchment > 0) {
    defender->entrenchment--;
  }
  if (losses > 0 && attacker->entrenchment > 0) {
    attacker->entrenchment--;
  }

  // Clear ZOC and spotting for units about to be destroyed, and log destruction
  for (auto &unit : game.units) {
    if (unit->strength <= 0) {
      setUnitZOC(game, unit.get(), false);
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
