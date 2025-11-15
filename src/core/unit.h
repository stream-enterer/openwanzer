#ifndef OPENWANZER_CORE_UNIT_H
#define OPENWANZER_CORE_UNIT_H

#include <string>
#include "hex_coord.h"
#include "enums.h"

struct Unit {
  std::string name;
  UnitClass unitClass;
  int side;     // 0 = axis, 1 = allied
  int strength; // 1-10
  int maxStrength;
  HexCoord position;

  // Combat stats
  int attack;
  int defense;

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
      : strength(10), maxStrength(10),
        attack(8), defense(6),
        movMethod(MovMethod::TRACKED), movementPoints(6),
        movesLeft(6), spotRange(2),
        hasMoved(false), hasFired(false), isCore(false), facing(0.0f) {}
};

#endif // OPENWANZER_CORE_UNIT_H
