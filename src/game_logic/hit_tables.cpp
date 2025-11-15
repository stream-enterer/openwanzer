#include "hit_tables.h"
#include <cstdlib>

namespace HitTables {

ArmorLocation rollHitLocation(CombatArcs::AttackArc arc) {
    int roll = rand() % 81; // 0-80

    switch (arc) {
        case CombatArcs::AttackArc::FRONT:
            if (roll < 1) return ArmorLocation::HEAD;
            if (roll < 17) return ArmorLocation::CENTER_TORSO;
            if (roll < 31) return ArmorLocation::LEFT_TORSO;
            if (roll < 45) return ArmorLocation::RIGHT_TORSO;
            if (roll < 55) return ArmorLocation::LEFT_ARM;
            if (roll < 65) return ArmorLocation::RIGHT_ARM;
            if (roll < 73) return ArmorLocation::LEFT_LEG;
            return ArmorLocation::RIGHT_LEG;

        case CombatArcs::AttackArc::LEFT_SIDE:
            if (roll < 1) return ArmorLocation::HEAD;
            if (roll < 5) return ArmorLocation::CENTER_TORSO;
            if (roll < 33) return ArmorLocation::LEFT_TORSO;
            if (roll < 61) return ArmorLocation::LEFT_ARM;
            return ArmorLocation::LEFT_LEG;

        case CombatArcs::AttackArc::RIGHT_SIDE:
            if (roll < 1) return ArmorLocation::HEAD;
            if (roll < 5) return ArmorLocation::CENTER_TORSO;
            if (roll < 33) return ArmorLocation::RIGHT_TORSO;
            if (roll < 61) return ArmorLocation::RIGHT_ARM;
            return ArmorLocation::RIGHT_LEG;

        case CombatArcs::AttackArc::REAR:
            if (roll < 16) return ArmorLocation::CENTER_TORSO_REAR;
            if (roll < 30) return ArmorLocation::LEFT_TORSO_REAR;
            if (roll < 44) return ArmorLocation::RIGHT_TORSO_REAR;
            if (roll < 48) return ArmorLocation::LEFT_ARM;
            if (roll < 52) return ArmorLocation::RIGHT_ARM;
            if (roll < 57) return ArmorLocation::LEFT_LEG;
            return ArmorLocation::RIGHT_LEG;

        default:
            return ArmorLocation::CENTER_TORSO;
    }
}

} // namespace HitTables
