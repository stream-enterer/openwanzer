#ifndef OPENWANZER_CORE_ENUMS_H
#define OPENWANZER_CORE_ENUMS_H

// Terrain types
enum class TerrainType {
  PLAINS,      // Open grassland
  FOREST,      // Woods/Trees
  MOUNTAIN,    // High elevation
  HILL,        // Low elevation
  DESERT,      // Sandy/arid
  SWAMP,       // Marsh/wetland
  CITY,        // Urban
  WATER,       // River/lake
  ROAD,        // Paved road
  ROUGH        // Rocky/broken terrain
};

// Unit classes
enum class UnitClass {
  INFANTRY,
  TANK,
  ARTILLERY,
  RECON,
  ANTI_TANK,
  AIR_DEFENSE
};

// Sides
enum class Side { AXIS = 0, ALLIED = 1 };

// Movement methods (12 types)
enum class MovMethod {
  TRACKED = 0,
  HALF_TRACKED = 1,
  WHEELED = 2,
  LEG = 3,
  TOWED = 4,
  AIR = 5,
  DEEP_NAVAL = 6,
  COSTAL = 7,
  ALL_TERRAIN_TRACKED = 8,
  AMPHIBIOUS = 9,
  NAVAL = 10,
  ALL_TERRAIN_LEG = 11
};

// Terrain type indices for movement tables
enum TerrainIndex {
  TI_CLEAR = 0,
  TI_CITY = 1,
  TI_AIRFIELD = 2,
  TI_FOREST = 3,
  TI_BOCAGE = 4,
  TI_HILL = 5,
  TI_MOUNTAIN = 6,
  TI_SAND = 7,
  TI_SWAMP = 8,
  TI_OCEAN = 9,
  TI_RIVER = 10,
  TI_FORTIFICATION = 11,
  TI_PORT = 12,
  TI_STREAM = 13,
  TI_ESCARPMENT = 14,
  TI_IMPASSABLE_RIVER = 15,
  TI_ROUGH = 16,
  TI_ROAD = 17
};

#endif // OPENWANZER_CORE_ENUMS_H
