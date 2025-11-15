#ifndef OPENWANZER_HIT_TABLES_H
#define OPENWANZER_HIT_TABLES_H

#include "ArmorLocation.h"
#include "CombatArcs.h"

namespace HitTables {

// Roll hit location based on attack arc
ArmorLocation rollHitLocation(CombatArcs::AttackArc arc);

} // namespace HitTables

#endif
