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
  int experience;   // 0-5 bars
  int entrenchment; // 0-5
  HexCoord position;

  // Combat stats
  int hardAttack;
  int softAttack;
  int groundDefense;
  int closeDefense;
  int initiative;

  // Movement & logistics
  MovMethod movMethod;
  int movementPoints;
  int movesLeft;
  int fuel;
  int maxFuel;
  int ammo;
  int maxAmmo;
  int spotRange;
  int rangeDefMod;  // Range defense modifier
  int hits;         // Accumulated hits (reduces defense)
  int entrenchTicks; // Ticks toward next entrenchment level

  bool hasMoved;
  bool hasFired;
  bool isCore; // campaign unit

  // Facing system (0-360 degrees, exact angle)
  float facing;

  Unit()
      : strength(10), maxStrength(10), experience(0), entrenchment(0),
        hardAttack(8), softAttack(10), groundDefense(6), closeDefense(5),
        initiative(5), movMethod(MovMethod::TRACKED), movementPoints(6),
        movesLeft(6), fuel(50), maxFuel(50), ammo(20), maxAmmo(20),
        spotRange(2), rangeDefMod(0), hits(0), entrenchTicks(0),
        hasMoved(false), hasFired(false), isCore(false), facing(0.0f) {}
};

#endif // OPENWANZER_CORE_UNIT_H
