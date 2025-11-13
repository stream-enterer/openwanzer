# Panzer General 2 Raylib Prototype

A C++ prototype implementation of the Panzer General 2 hex-based turn-based strategy game using raylib for graphics.

## Features Implemented

Based on the original JavaScript source code, this prototype includes:

### Core Game Mechanics
- **Hex-based Map System**: Offset coordinate system with proper hex-to-pixel conversion
- **Multiple Terrain Types**: Clear, Forest, Mountain, City, Water
- **Turn-based Gameplay**: Alternating turns between Axis and Allied forces
- **Victory Hexes**: Special objectives that can be captured

### Unit System
- **Six Unit Classes**: 
  - Infantry
  - Tank
  - Artillery (with extended range)
  - Recon
  - Anti-Tank
  - Air Defense
  
- **Unit Statistics**:
  - Strength (1-10, representing combat power)
  - Experience (0-5 bars, increases through combat)
  - Entrenchment (0-5, increases when stationary)
  - Movement points and fuel consumption
  - Ammunition tracking
  - Attack and defense values

### Combat System
- **Attack Mechanics**: Range-based combat with different ranges per unit type
- **Defense Modifiers**: Entrenchment and experience affect combat outcomes
- **Experience Gain**: Units gain experience through successful attacks
- **Fog of War**: Visual representation (units marked by side)

### User Interface
- **Hex Grid Rendering**: Clean hexagonal grid with terrain colors
- **Unit Display**: Units shown with class symbols and strength indicators
- **Selection Highlighting**: Green for movement range, red for attack range
- **Info Panel**: Displays selected unit's statistics
- **Status Bar**: Shows current turn and active player

## Building the Prototype

### Prerequisites
Install raylib development libraries:

**Ubuntu/Debian:**
```bash
sudo apt install libraylib-dev
```

**Arch Linux:**
```bash
sudo pacman -S raylib
```

**macOS:**
```bash
brew install raylib
```

### Compilation

**Using Fish shell (preferred):**
```fish
chmod +x build.fish
./build.fish
```

**Using bash:**
```bash
chmod +x build.sh
./build.sh
```

**Manual compilation:**
```bash
g++ pg2_prototype.cpp -o pg2_prototype \
    $(pkg-config --cflags --libs raylib) \
    -std=c++17 -O2 -Wall
```

## Running

```bash
./pg2_prototype
```

## Controls

- **Left Click**: Select unit / Move unit / Attack enemy
- **Arrow Keys**: Pan camera view
- **SPACE**: End current player's turn
- **ESC**: Quit application

## How to Play

1. **Unit Selection**: Left-click on a friendly unit (your color) to select it
   - Axis units are RED
   - Allied units are BLUE

2. **Movement**: 
   - After selecting a unit, hexes within movement range turn GREEN
   - Click on a green hex to move the unit there
   - Movement costs fuel and consumes movement points

3. **Combat**:
   - After moving (or from starting position), hexes with enemy units in attack range turn RED
   - Click on a red hex to attack the enemy unit
   - Combat is resolved automatically based on unit stats
   - Artillery has extended range (3 hexes vs 1 hex for other units)

4. **Turn Management**:
   - Each unit can move and attack once per turn
   - Entrenchment increases when units remain stationary
   - Press SPACE to end your turn and pass to the opponent
   - Turn counter advances when both sides have played

5. **Strategy Tips**:
   - Keep units stationary to build entrenchment (better defense)
   - Artillery can attack from range without taking return fire
   - Experienced units (shown with stars) perform better in combat
   - Capture victory hexes (marked with gold circles) to win

## Architecture Overview

### Key Data Structures

**Hex**: Represents a single hexagonal tile
- Terrain type and ownership
- Victory hex and deployment zone flags
- Spotting and selection states

**Unit**: Represents a military unit
- Position, class, and allegiance
- Combat statistics (attack/defense values)
- Status tracking (moved, fired, fuel, ammo)
- Experience and entrenchment levels

**GameState**: Manages the entire game
- Hex map grid
- All units on the map
- Current turn and active player
- Selection state

### Rendering System

The rendering uses a layered approach:
1. Terrain hexagons (filled with terrain color)
2. Hex grid outlines
3. Special markers (victory hexes, highlights)
4. Unit sprites (rectangles with class symbols)
5. UI overlay (panels, text information)

### Combat Calculation

Based on the original PG2 formula with simplifications:
- Attack value = base attack + experience bonus + random roll
- Defense value = base defense + entrenchment bonus + experience + random roll
- Damage dealt = difference between attack and defense rolls
- Both attacker and defender can take casualties
- Experience increases for successful attacks

## Differences from Original JavaScript

This prototype simplifies some aspects for clarity:

1. **No Transport System**: Units cannot carry or tow other units
2. **Simplified Movement**: No ZOC (Zone of Control) mechanics
3. **No Weather/Ground Conditions**: Combat not affected by atmospheric conditions
4. **No Fuel-Based Range**: Movement range simplified
5. **Limited Leader Abilities**: No special leader traits implemented
6. **No Campaign Mode**: Single scenario play only
7. **No AI**: Human vs Human only (hot-seat)

## Future Enhancements

Potential additions to bring closer to the full game:

- [ ] Implement Zone of Control mechanics
- [ ] Add transport/carrier system
- [ ] Weather and ground condition effects
- [ ] Leader traits and abilities
- [ ] Supply line mechanics
- [ ] AI opponent
- [ ] Campaign progression system
- [ ] Save/load game state
- [ ] Scenario editor
- [ ] Network multiplayer
- [ ] Sound effects and music
- [ ] Unit sprite graphics
- [ ] Terrain graphics and animations

## Code Structure

```
pg2_prototype.cpp
├── Constants and Enums
│   ├── TerrainType
│   ├── UnitClass
│   └── Side
├── Data Structures
│   ├── HexCoord
│   ├── Hex
│   ├── Unit
│   └── GameState
├── Rendering Functions
│   ├── hexToPixel() / pixelToHex()
│   ├── drawHexagon()
│   ├── drawMap()
│   └── drawUI()
├── Game Logic
│   ├── highlightMovementRange()
│   ├── highlightAttackRange()
│   ├── moveUnit()
│   ├── performAttack()
│   └── endTurn()
└── Main Game Loop
    ├── Input handling
    ├── Game state updates
    └── Rendering
```

## Performance

The prototype is optimized for:
- 60 FPS gameplay
- Real-time hex calculations
- Efficient unit lookup
- Minimal memory allocation during gameplay

Map size: 12 rows × 16 columns = 192 hexes
Typical unit count: 6-20 units per side

## Credits

Based on the open source JavaScript Panzer General 2 port by Nicu Pavel (http://openpanzer.net)

Original Panzer General 2 by Strategic Simulations, Inc. (SSI)

This prototype uses raylib (https://www.raylib.com/) for graphics and input handling.

## License

This prototype is provided as educational reference material. The original Panzer General 2 game and assets are property of their respective owners.

## Contact & Contributions

This is a demonstration prototype showing how the JavaScript implementation could be adapted to C++ with raylib. Feel free to extend it with additional features from the original source code!
