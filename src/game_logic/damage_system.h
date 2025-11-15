#ifndef OPENWANZER_DAMAGE_SYSTEM_H
#define OPENWANZER_DAMAGE_SYSTEM_H

#include "../core/unit.h"
#include "../core/armor_location.h"
#include "../core/game_state.h"

namespace DamageSystem {

void applyDamageToLocation(GameState& game, Unit* target, ArmorLocation location, int damage);
ArmorLocation getTransferLocation(ArmorLocation destroyed);
ArmorLocation mapRearToFront(ArmorLocation rear);

} // namespace DamageSystem

#endif
