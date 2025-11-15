#ifndef OPENWANZER_DAMAGE_SYSTEM_HPP
#define OPENWANZER_DAMAGE_SYSTEM_HPP

#include "Unit.hpp"
#include "ArmorLocation.hpp"
#include "GameState.hpp"

namespace damagesystem {

void applyDamageToLocation(GameState& game, Unit* target, ArmorLocation location, int damage);
ArmorLocation getTransferLocation(ArmorLocation destroyed);
ArmorLocation mapRearToFront(ArmorLocation rear);

} // namespace damagesystem

#endif
