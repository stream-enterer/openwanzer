#ifndef OPENWANZER_ARMOR_LOCATION_HPP
#define OPENWANZER_ARMOR_LOCATION_HPP

#include <string>

enum class ArmorLocation {
    HEAD,
    CENTER_TORSO,
    LEFT_TORSO,
    RIGHT_TORSO,
    LEFT_ARM,
    RIGHT_ARM,
    LEFT_LEG,
    RIGHT_LEG,
    CENTER_TORSO_REAR,
    LEFT_TORSO_REAR,
    RIGHT_TORSO_REAR,
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
        : currentArmor(0), maxArmor(0),
          currentStructure(0), maxStructure(0),
          isDestroyed(false) {}

    LocationStatus(int armor, int structure)
        : currentArmor(armor), maxArmor(armor),
          currentStructure(structure), maxStructure(structure),
          isDestroyed(false) {}
};

#endif
