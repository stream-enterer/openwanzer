#ifndef OPENWANZER_CORE_UNIT_H
#define OPENWANZER_CORE_UNIT_H

#include <string>
#include <map>
#include "hex_coord.h"
#include "enums.h"
#include "armor_location.h"

struct Unit {
  std::string name;
  UnitClass unitClass;
  int side;     // 0 = axis, 1 = allied

  // **NEW: Location-based damage**
  enum class WeightClass { LIGHT, MEDIUM, HEAVY, ASSAULT };
  WeightClass weightClass;
  std::map<ArmorLocation, LocationStatus> locations;

  HexCoord position;

  // Combat stats
  int attack;
  int defense;

  // **NEW: Weapon range (3 hexes for testing)**
  int weaponRange;

  // Movement & logistics
  MovMethod movMethod;
  int movementPoints;
  int movesLeft;
  int spotRange;

  bool hasMoved;
  bool hasFired;
  bool isCore; // campaign unit

  // Facing system (0-360 degrees, exact angle)
  float facing;

  Unit()
      : weightClass(WeightClass::MEDIUM),
        attack(8), defense(6),
        weaponRange(3),
        movMethod(MovMethod::TRACKED), movementPoints(6),
        movesLeft(6), spotRange(2),
        hasMoved(false), hasFired(false), isCore(false), facing(0.0f) {
    initializeLocations(WeightClass::MEDIUM);
  }

  void initializeLocations(WeightClass wClass);
  bool isAlive() const;
  bool canMove() const;
  int getOverallHealthPercent() const;
};

#endif // OPENWANZER_CORE_UNIT_H
