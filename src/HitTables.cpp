#include "HitTables.hpp"
#include <cstdlib>

namespace hittables {

ArmorLocation rollHitLocation(combatarcs::AttackArc arc) {
	// Simplified hit location: 90% chance to hit arc-appropriate location,
	// 10% chance to hit CENTER instead
	int roll = rand() % 100;

	// 10% chance to hit CENTER regardless of arc
	if (roll < 10) {
		return ArmorLocation::CENTER;
	}

	// 90% chance to hit the arc-appropriate location
	switch (arc) {
		case combatarcs::AttackArc::FRONT:
			return ArmorLocation::FRONT;

		case combatarcs::AttackArc::LEFT_SIDE:
			return ArmorLocation::LEFT;

		case combatarcs::AttackArc::RIGHT_SIDE:
			return ArmorLocation::RIGHT;

		case combatarcs::AttackArc::REAR:
			return ArmorLocation::REAR;

		default:
			return ArmorLocation::CENTER;
	}
}

} // namespace hittables
