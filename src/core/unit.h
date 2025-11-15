#ifndef OPENWANZER_CORE_UNIT_H
#define OPENWANZER_CORE_UNIT_H

#include <string>
#include <map>
#include <vector>
#include "hex_coord.h"
#include "enums.h"
#include "armor_location.h"

enum class WeaponType {
    MISSILE,    // Magenta
    ENERGY,     // Green
    BALLISTIC,  // Cyan
    ARTILLERY,  // Red
    MELEE       // White
};

struct Weapon {
    std::string name;
    WeaponType type;
    int damage;
    bool isDestroyed;

    Weapon(const std::string& n, WeaponType t, int dmg)
        : name(n), type(t), damage(dmg), isDestroyed(false) {}
};

struct Unit {
  std::string name;
  UnitClass unitClass;
  int side;     // 0 = axis, 1 = allied

  // **NEW: Location-based damage**
  enum class WeightClass { LIGHT, MEDIUM, HEAVY, ASSAULT };
  WeightClass weightClass;
  std::map<ArmorLocation, LocationStatus> locations;
  std::vector<Weapon> weapons;

  HexCoord position;

  // Combat stats
  int attack;

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
        attack(8),
        weaponRange(3),
        movMethod(MovMethod::TRACKED), movementPoints(6),
        movesLeft(6), spotRange(2),
        hasMoved(false), hasFired(false), isCore(false), facing(0.0f) {
    initializeLocations(WeightClass::MEDIUM);
  }

  void initializeLocations(WeightClass wClass);
  void initializeWeapons();
  bool isAlive() const;
  bool canMove() const;
  int getOverallHealthPercent() const;
};

#endif // OPENWANZER_CORE_UNIT_H
