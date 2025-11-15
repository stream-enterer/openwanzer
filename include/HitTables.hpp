#ifndef OPENWANZER_HIT_TABLES_HPP
#define OPENWANZER_HIT_TABLES_HPP

#include "ArmorLocation.hpp"
#include "CombatArcs.hpp"

namespace hittables {

// Roll hit location based on attack arc
ArmorLocation rollHitLocation(combatarcs::AttackArc arc);

} // namespace hittables

#endif
