#include "combat_arcs.h"
#include <cmath>

namespace CombatArcs {

AttackArc getAttackArc(Vector2 attackerPos, Vector2 targetPos, float targetFacing) {
    // Calculate angle from target to attacker
    float dx = attackerPos.x - targetPos.x;
    float dy = attackerPos.y - targetPos.y;
    float attackerAngle = atan2(dy, dx) * (180.0f / PI);

    // Normalize to 0-360
    if (attackerAngle < 0) attackerAngle += 360.0f;

    // Get relative angle to target's facing
    float relativeAngle = attackerAngle - targetFacing;

    // Normalize to -180 to +180
    while (relativeAngle > 180.0f) relativeAngle -= 360.0f;
    while (relativeAngle < -180.0f) relativeAngle += 360.0f;

    // BattleTech arcs: Front ±30°, Sides 30-150°, Rear ±150-180°
    if (fabs(relativeAngle) <= 30.0f) return AttackArc::FRONT;
    if (relativeAngle > 30.0f && relativeAngle <= 150.0f) return AttackArc::RIGHT_SIDE;
    if (relativeAngle < -30.0f && relativeAngle >= -150.0f) return AttackArc::LEFT_SIDE;
    return AttackArc::REAR;
}

bool isInFiringArc(Vector2 attackerPos, float attackerFacing, Vector2 targetPos) {
    // Calculate angle from attacker to target
    float dx = targetPos.x - attackerPos.x;
    float dy = targetPos.y - attackerPos.y;
    float targetAngle = atan2(dy, dx) * (180.0f / PI);

    // Normalize to 0-360
    if (targetAngle < 0) targetAngle += 360.0f;

    // Get relative angle
    float relativeAngle = targetAngle - attackerFacing;

    // Normalize to -180 to +180
    while (relativeAngle > 180.0f) relativeAngle -= 360.0f;
    while (relativeAngle < -180.0f) relativeAngle += 360.0f;

    // Front 120° arc (±60° from facing)
    return fabs(relativeAngle) <= 60.0f;
}

Color getLineColor(AttackArc arc) {
    switch(arc) {
        case AttackArc::FRONT: return RED;
        case AttackArc::LEFT_SIDE:
        case AttackArc::RIGHT_SIDE: return SKYBLUE; // Cyan
        case AttackArc::REAR: return GREEN;
    }
    return WHITE; // Fallback
}

Color getArcSegmentColor(AttackArc arc, bool isFrontArc) {
    if (isFrontArc) {
        return Color{255, 0, 0, 60}; // Semi-transparent red
    }
    return Color{102, 102, 102, 60}; // Semi-transparent darker grey (20% darker)
}

} // namespace CombatArcs
