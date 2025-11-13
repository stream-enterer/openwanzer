"""
Panzer General 2 Prototype - Constants
Based on the JavaScript implementation
"""

from enum import IntEnum
import math

# Screen settings
SCREEN_WIDTH = 1200
SCREEN_HEIGHT = 800
SCREEN_TITLE = "Panzer General 2 - Python Prototype"

# Hex settings
HEX_SIZE = 40
HEX_WIDTH = HEX_SIZE * 2
HEX_HEIGHT = HEX_SIZE * math.sqrt(3)

# Map size
MAP_ROWS = 12
MAP_COLS = 16

# Colors
COLOR_CLEAR = (144, 238, 144)
COLOR_FOREST = (34, 139, 34)
COLOR_MOUNTAIN = (139, 137, 137)
COLOR_CITY = (169, 169, 169)
COLOR_WATER = (70, 130, 180)
COLOR_ROAD = (101, 67, 33)
COLOR_SELECTED = (255, 255, 0)
COLOR_MOVEABLE = (0, 255, 0, 128)
COLOR_ATTACKABLE = (255, 0, 0, 128)
COLOR_HEX_BORDER = (100, 100, 100)
COLOR_AXIS = (255, 0, 0)
COLOR_ALLIES = (0, 0, 255)

# Unit classes
class UnitClass(IntEnum):
    NONE = 0
    INFANTRY = 1
    TANK = 2
    RECON = 3
    ANTI_TANK = 4
    ARTILLERY = 5
    FIGHTER = 10
    BOMBER = 11

# Unit types
class UnitType(IntEnum):
    SOFT = 0
    HARD = 1
    AIR = 2
    NAVAL = 3

# Terrain types
class TerrainType(IntEnum):
    CLEAR = 0
    CITY = 1
    FOREST = 2
    MOUNTAIN = 3
    WATER = 4
    ROAD = 5

TERRAIN_NAMES = {
    TerrainType.CLEAR: "Clear",
    TerrainType.CITY: "City",
    TerrainType.FOREST: "Forest",
    TerrainType.MOUNTAIN: "Mountain",
    TerrainType.WATER: "Water",
    TerrainType.ROAD: "Road"
}

TERRAIN_COLORS = {
    TerrainType.CLEAR: COLOR_CLEAR,
    TerrainType.CITY: COLOR_CITY,
    TerrainType.FOREST: COLOR_FOREST,
    TerrainType.MOUNTAIN: COLOR_MOUNTAIN,
    TerrainType.WATER: COLOR_WATER,
    TerrainType.ROAD: COLOR_ROAD
}

# Movement costs for different terrain (fuel cost)
MOVEMENT_COSTS = {
    TerrainType.CLEAR: 1,
    TerrainType.CITY: 1,
    TerrainType.FOREST: 2,
    TerrainType.MOUNTAIN: 3,
    TerrainType.WATER: 999,  # Impassable for ground units
    TerrainType.ROAD: 1
}

# Terrain defense bonuses (percentage)
TERRAIN_DEFENSE = {
    TerrainType.CLEAR: 0,
    TerrainType.CITY: 30,
    TerrainType.FOREST: 20,
    TerrainType.MOUNTAIN: 40,
    TerrainType.WATER: 0,
    TerrainType.ROAD: 0
}

# Player sides
class Side(IntEnum):
    AXIS = 0
    ALLIES = 1

SIDE_NAMES = {
    Side.AXIS: "Axis",
    Side.ALLIES: "Allies"
}

SIDE_COLORS = {
    Side.AXIS: COLOR_AXIS,
    Side.ALLIES: COLOR_ALLIES
}

# Player types
class PlayerType(IntEnum):
    HUMAN = 0
    AI = 1

# Action types
class ActionType(IntEnum):
    MOVE = 0
    ATTACK = 1
    SKIP = 2
    END_TURN = 3

# Unit equipment data
UNIT_DATA = {
    "German_Infantry": {
        "name": "Infantry",
        "unit_class": UnitClass.INFANTRY,
        "unit_type": UnitType.SOFT,
        "attack": 6,
        "defense": 8,
        "move_points": 4,
        "fuel": 40,
        "ammo": 10,
        "cost": 50
    },
    "German_Tank": {
        "name": "Panzer IV",
        "unit_class": UnitClass.TANK,
        "unit_type": UnitType.HARD,
        "attack": 12,
        "defense": 10,
        "move_points": 6,
        "fuel": 60,
        "ammo": 8,
        "cost": 150
    },
    "German_Recon": {
        "name": "Recon",
        "unit_class": UnitClass.RECON,
        "unit_type": UnitType.SOFT,
        "attack": 5,
        "defense": 6,
        "move_points": 8,
        "fuel": 70,
        "ammo": 8,
        "cost": 80
    },
    "German_Artillery": {
        "name": "Artillery",
        "unit_class": UnitClass.ARTILLERY,
        "unit_type": UnitType.SOFT,
        "attack": 14,
        "defense": 4,
        "move_points": 3,
        "fuel": 40,
        "ammo": 6,
        "cost": 120
    },
    "Allied_Infantry": {
        "name": "Infantry",
        "unit_class": UnitClass.INFANTRY,
        "unit_type": UnitType.SOFT,
        "attack": 6,
        "defense": 8,
        "move_points": 4,
        "fuel": 40,
        "ammo": 10,
        "cost": 50
    },
    "Allied_Tank": {
        "name": "Sherman",
        "unit_class": UnitClass.TANK,
        "unit_type": UnitType.HARD,
        "attack": 10,
        "defense": 9,
        "move_points": 6,
        "fuel": 60,
        "ammo": 8,
        "cost": 140
    },
    "Allied_Recon": {
        "name": "Recon",
        "unit_class": UnitClass.RECON,
        "unit_type": UnitType.SOFT,
        "attack": 5,
        "defense": 6,
        "move_points": 8,
        "fuel": 70,
        "ammo": 8,
        "cost": 80
    },
    "Allied_Artillery": {
        "name": "Artillery",
        "unit_class": UnitClass.ARTILLERY,
        "unit_type": UnitType.SOFT,
        "attack": 13,
        "defense": 4,
        "move_points": 3,
        "fuel": 40,
        "ammo": 6,
        "cost": 120
    }
}
