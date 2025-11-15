#ifndef OPENWANZER_DAMAGE_SYSTEM_H
#define OPENWANZER_DAMAGE_SYSTEM_H

#include "Unit.h"
#include "ArmorLocation.h"
#include "GameState.h"

namespace DamageSystem {

void applyDamageToLocation(GameState& game, Unit* target, ArmorLocation location, int damage);
ArmorLocation getTransferLocation(ArmorLocation destroyed);
ArmorLocation mapRearToFront(ArmorLocation rear);

} // namespace DamageSystem

#endif
