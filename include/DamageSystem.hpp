#ifndef OPENWANZER_DAMAGE_SYSTEM_HPP
#define OPENWANZER_DAMAGE_SYSTEM_HPP

#include "ArmorLocation.hpp"
#include "GameState.hpp"
#include "Unit.hpp"

namespace damagesystem {

void applyDamageToLocation(GameState& game, Unit* target, ArmorLocation location, int damage);
ArmorLocation getTransferLocation(ArmorLocation destroyed);

} // namespace damagesystem

#endif
