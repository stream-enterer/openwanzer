#ifndef OPENWANZER_ARMOR_LOCATION_HPP
#define OPENWANZER_ARMOR_LOCATION_HPP

#include <string>

// Simplified 5-part armor system (top-down view)
// Layout:     [FRONT]
//         [LEFT][CENTER][RIGHT]
//             [REAR]
enum class ArmorLocation {
	FRONT,
	REAR,
	LEFT,
	RIGHT,
	CENTER,
	NONE
};

std::string locationToString(ArmorLocation loc);

struct LocationStatus {
	int currentArmor;
	int maxArmor;
	int currentStructure;
	int maxStructure;
	bool isDestroyed;

	LocationStatus()
	    : currentArmor(0), maxArmor(0), currentStructure(0), maxStructure(0), isDestroyed(false) {
	}

	LocationStatus(int armor, int structure)
	    : currentArmor(armor), maxArmor(armor), currentStructure(structure), maxStructure(structure), isDestroyed(false) {
	}
};

#endif
