#ifndef OPENWANZER_HIT_TABLES_HPP
#define OPENWANZER_HIT_TABLES_HPP

#include "ArmorLocation.hpp"
#include "CombatArcs.hpp"

namespace HitTables {

// Roll hit location based on attack arc
ArmorLocation rollHitLocation(CombatArcs::AttackArc arc);

} // namespace HitTables

#endif
