#include "armor_location.h"

std::string locationToString(ArmorLocation loc) {
    switch (loc) {
        case ArmorLocation::HEAD: return "HEAD";
        case ArmorLocation::CENTER_TORSO: return "CENTER TORSO";
        case ArmorLocation::LEFT_TORSO: return "LEFT TORSO";
        case ArmorLocation::RIGHT_TORSO: return "RIGHT TORSO";
        case ArmorLocation::LEFT_ARM: return "LEFT ARM";
        case ArmorLocation::RIGHT_ARM: return "RIGHT ARM";
        case ArmorLocation::LEFT_LEG: return "LEFT LEG";
        case ArmorLocation::RIGHT_LEG: return "RIGHT LEG";
        case ArmorLocation::CENTER_TORSO_REAR: return "CT (REAR)";
        case ArmorLocation::LEFT_TORSO_REAR: return "LT (REAR)";
        case ArmorLocation::RIGHT_TORSO_REAR: return "RT (REAR)";
        default: return "NONE";
    }
}
