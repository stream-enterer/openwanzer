#include "ArmorLocation.hpp"

std::string locationToString(ArmorLocation loc) {
	switch (loc) {
		case ArmorLocation::FRONT:
			return "FRONT";
		case ArmorLocation::REAR:
			return "REAR";
		case ArmorLocation::LEFT:
			return "LEFT";
		case ArmorLocation::RIGHT:
			return "RIGHT";
		case ArmorLocation::CENTER:
			return "CENTER";
		default:
			return "NONE";
	}
}
