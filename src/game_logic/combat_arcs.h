#ifndef OPENWANZER_COMBAT_ARCS_H
#define OPENWANZER_COMBAT_ARCS_H

#include "raylib.h"
#include "../core/hex_coord.h"

namespace CombatArcs {

enum class AttackArc { FRONT, LEFT_SIDE, RIGHT_SIDE, REAR };

// Calculate which arc of the target is being attacked from
AttackArc getAttackArc(Vector2 attackerPos, Vector2 targetPos, float targetFacing);

// Check if target is within attacker's frontal firing arc
bool isInFiringArc(Vector2 attackerPos, float attackerFacing, Vector2 targetPos);

// Get color for attack line based on arc
Color getLineColor(AttackArc arc);

// Get color for target arc segment
Color getArcSegmentColor(AttackArc arc, bool isFrontArc);

} // namespace CombatArcs

#endif
