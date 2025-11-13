# Panzer General 2 Prototype - Technical Design Document

## Overview

This document describes the technical implementation of the Panzer General 2 raylib prototype, mapping features from the original JavaScript implementation to the C++ version.

## JavaScript to C++ Feature Mapping

### Map System (map.js → pg2_prototype.cpp)

**JavaScript Original:**
```javascript
function Hex(row, col) {
    this.terrain = terrainType.clear;
    this.unit = null;
    this.airunit = null;
    this.isZOC = function(side) { ... }
    this.isSpotted = function(side) { ... }
}
```

**C++ Implementation:**
```cpp
struct Hex {
    HexCoord coord;
    TerrainType terrain;
    int owner;
    bool isSpotted[2];  // Per-side spotting
    bool isMoveSel;     // Movement highlight
    bool isAttackSel;   // Attack highlight
};
```

### Unit System (unit.js → pg2_prototype.cpp)

**Key Stats Implemented:**
- `strength`: Current unit strength (1-10)
- `experience`: Combat experience bars (0-5)
- `entrenchment`: Defense bonus from staying put (0-5)
- `movesLeft`: Remaining movement points this turn
- `fuel` / `ammo`: Resource management
- `hasMoved` / `hasFired`: Action tracking

**Combat Stats:**
- `hardAttack`: vs armored targets
- `softAttack`: vs infantry/soft targets  
- `groundDefense`: vs ground attacks
- `closeDefense`: in melee range
- `initiative`: who fires first

### Game Rules (gamerules.js → pg2_prototype.cpp)

#### Movement Range Calculation

**Original (simplified):**
```javascript
GameRules.getMoveRange = function(map, unit, rows, cols) {
    var range = GameRules.getUnitMoveRange(unit);
    var moveCost = movTable[movmethod];
    // Calculate costs for each hex considering terrain
    // Apply ZOC restrictions
    // Return list of reachable cells
}
```

**Prototype:**
```cpp
void highlightMovementRange(GameState& game, Unit* unit) {
    int range = unit->movesLeft;
    // Simple distance check
    for each hex:
        if hexDistance(unit->position, hex) <= range:
            if valid for movement:
                hex.isMoveSel = true
}
```

#### Combat Resolution

**Original Formula:**
```javascript
// Attack/Defense values modified by:
// - Terrain (city +4 defense, river -4/+4)
// - Entrenchment (added to defense)
// - Experience (added to both)
// - Initiative (determines first strike)
// - Range defense modifier
// - Surprise status
```

**Prototype (Simplified):**
```cpp
int attackValue = attacker->hardAttack + attacker->experience * 2;
int defenseValue = defender->groundDefense + 
                   defender->entrenchment * 2 + 
                   defender->experience;
int damage = (attackValue + roll) - (defenseValue + roll);
```

### Rendering System (render.js → pg2_prototype.cpp)

#### Hex Coordinate Conversion

**JavaScript:**
```javascript
function hexToPixel(row, col) {
    x = col * colSlice;  // colSlice = HEX_WIDTH * 0.75
    y = row * r + (col % 2) * (r * 0.5);  // r = hex height
}
```

**C++:**
```cpp
Vector2 hexToPixel(int row, int col, float offsetX, float offsetY) {
    float x = offsetX + col * HEX_WIDTH * 0.75f;
    float y = offsetY + row * HEX_HEIGHT + 
              (col % 2) * (HEX_HEIGHT * 0.5f);
    return {x, y};
}
```

#### Drawing Hexagons

The prototype implements proper hexagon rendering:
- 6 vertices at 60° intervals
- Filled for terrain, outlined for grid
- Layered rendering (terrain → grid → units → UI)

### User Interface (ui.js/uibuilder.js → pg2_prototype.cpp)

**Turn Management Panel:**
- Current turn number / max turns
- Active player (Axis/Allied)
- Controls help text

**Unit Info Panel:**
- Unit name and class
- Strength display
- Experience stars
- Movement/fuel/ammo tracking
- Combat statistics

## Game Flow

```
Main Loop:
    ├── Input Processing
    │   ├── Mouse: Select/Move/Attack
    │   ├── Keyboard: End Turn, Pan Camera
    │   └── Selection Management
    │
    ├── Game State Update
    │   ├── Unit Movement
    │   ├── Combat Resolution
    │   ├── Turn Advancement
    │   └── Victory Check
    │
    └── Rendering
        ├── Map Layer (Terrain)
        ├── Highlight Layer (Movement/Attack)
        ├── Unit Layer (Sprites)
        └── UI Layer (Panels/Text)
```

## Data Flow Diagrams

### Unit Selection & Movement
```
Click Hex
    ↓
Get Unit at Hex ──→ Unit Found? ──→ Friendly? ──→ Select Unit
    ↓                   ↓                              ↓
    No              Enemy Unit                 Highlight Movement
                        ↓                       Highlight Attack
                   Attack Available?
                        ↓
                   Perform Combat
```

### Combat Resolution
```
Attack Command
    ↓
Calculate Attack Value ← Attacker Stats + Experience + Roll
    ↓
Calculate Defense Value ← Defender Stats + Entrenchment + Experience + Roll
    ↓
Determine Damage ← Attack Value - Defense Value
    ↓
Apply Casualties ──→ Attacker ← Counter-attack damage
    ↓                Defender
    ↓
Update Experience ← Successful Attack = +1 XP
    ↓
Mark Unit as Fired
    ↓
Check for Destroyed Units ──→ Remove from Game
```

### Turn Sequence
```
Active Player Phase
    ├── Move Units
    │   ├── Select Unit
    │   ├── Show Movement Range
    │   ├── Move to Hex
    │   └── Consume Movement Points
    │
    ├── Combat
    │   ├── Select Target
    │   ├── Resolve Attack
    │   ├── Apply Casualties
    │   └── Mark as Fired
    │
    └── End Turn
        ├── Reset Action Flags
        ├── Increase Entrenchment
        ├── Switch Player
        └── Advance Turn Counter
```

## Memory Layout

### GameState Structure
```
GameState (~100 KB typical)
├── map[12][16] (Hex objects)       ~3 KB
│   └── Each Hex: ~16 bytes
├── units (vector of pointers)      ~1 KB
│   └── Each Unit: ~120 bytes
├── selectedUnit (pointer)          8 bytes
└── game metadata                   ~20 bytes
```

### Performance Characteristics
- **Hex to Pixel**: O(1) - Direct calculation
- **Pixel to Hex**: O(1) - Inverse calculation  
- **Find Unit at Hex**: O(n) - Linear search, n = unit count
- **Movement Range**: O(m) - m = map size (192 hexes)
- **Attack Range**: O(m) - Similar to movement
- **Full Map Render**: O(m + n) - All hexes + all units

## Extension Points

### Adding New Unit Types
```cpp
// 1. Add to UnitClass enum
enum class UnitClass {
    // ... existing ...
    FIGHTER,        // Air fighter
    BOMBER,         // Tactical bomber
    NAVAL,          // Naval units
};

// 2. Update getUnitSymbol()
std::string getUnitSymbol(UnitClass uClass) {
    case UnitClass::FIGHTER: return "FTR";
    case UnitClass::BOMBER: return "BMB";
    // ...
}

// 3. Adjust combat calculations for air/naval
```

### Adding Terrain Effects
```cpp
struct Hex {
    // ...
    int getMovementCost(UnitClass unitClass) {
        switch (terrain) {
            case TerrainType::FOREST:
                if (unitClass == UnitClass::INFANTRY) return 1;
                return 2;
            // ...
        }
    }
    
    int getDefenseBonus() {
        switch (terrain) {
            case TerrainType::CITY: return 4;
            case TerrainType::FOREST: return 2;
            // ...
        }
    }
};
```

### Zone of Control (ZOC)
```cpp
struct Hex {
    bool isZOC[2];  // Per-side ZOC
    
    void updateZOC(GameState& game) {
        // For each unit, mark adjacent hexes as ZOC
        // Movement through enemy ZOC costs extra
    }
};
```

### AI Opponent
```cpp
class AIPlayer {
    void planTurn(GameState& game) {
        // 1. Evaluate board state
        // 2. Find objectives
        // 3. Plan unit movements
        // 4. Execute attacks
    }
    
    Unit* selectBestAttacker(Unit* target) {
        // Calculate best unit to attack target
    }
    
    HexCoord findBestPosition(Unit* unit) {
        // Calculate optimal hex for unit
    }
};
```

## Testing Scenarios

### Scenario 1: Basic Movement
```
Initial State:
- Tank at (2,2) with 6 movement points
- Clear terrain all around

Expected:
- Can move to any hex within 6 distance
- Movement costs reduce movesLeft
- Fuel depletes by distance moved
```

### Scenario 2: Combat
```
Initial State:
- Axis Tank at (5,5): Strength 10, Experience 0
- Allied Infantry at (5,6): Strength 10, Entrenchment 2

Action: Tank attacks Infantry

Expected:
- Infantry takes damage (2-4 strength loss)
- Tank takes minor counter-damage (0-1 strength loss)
- Tank gains experience (0 → 1)
- Tank marked as hasFired = true
```

### Scenario 3: Turn Management
```
Initial State:
- Turn 1, Axis player active
- 3 Axis units, 3 Allied units

Action: End Turn (SPACE)

Expected:
- Axis units: reset hasMoved/hasFired flags
- Stationary Axis units: +1 entrenchment
- Switch to Allied player
- Turn remains 1
```

### Scenario 4: Victory Hex
```
Initial State:
- Victory hex at (5,4) owned by Allies
- Axis Tank moves to (5,4)

Expected:
- Hex owner changes to Axis
- Victory check triggered
- Objective counter updated
```

## Build Configurations

### Debug Build
```bash
g++ pg2_prototype.cpp -o pg2_prototype_debug \
    $(pkg-config --cflags --libs raylib) \
    -std=c++17 -g -Wall -Wextra -DDEBUG
```

### Release Build
```bash
g++ pg2_prototype.cpp -o pg2_prototype \
    $(pkg-config --cflags --libs raylib) \
    -std=c++17 -O3 -Wall -DNDEBUG
```

### Profile Build
```bash
g++ pg2_prototype.cpp -o pg2_prototype_profile \
    $(pkg-config --cflags --libs raylib) \
    -std=c++17 -O2 -pg -Wall
```

## Performance Targets

- **Frame Rate**: 60 FPS constant
- **Input Latency**: < 16ms (1 frame)
- **Map Render**: < 10ms per frame
- **Unit Count**: Support 50+ units without slowdown
- **Memory**: < 50 MB total usage

## Known Limitations

1. **No pathfinding**: Movement uses simple distance, not optimal paths
2. **No terrain cost**: All terrain costs 1 movement point
3. **Simple combat**: Doesn't account for all original modifiers
4. **No save/load**: Game state not persistent
5. **Hot-seat only**: No network or AI opponent

## Future Roadmap

### Phase 1: Core Improvements
- [ ] Implement proper pathfinding (A* algorithm)
- [ ] Add terrain movement costs
- [ ] Implement Zone of Control
- [ ] Add fog of war rendering

### Phase 2: Advanced Features
- [ ] Leader traits and abilities
- [ ] Transport/carrier system
- [ ] Weather effects
- [ ] Supply lines

### Phase 3: Polish
- [ ] Sound effects
- [ ] Unit sprites/graphics
- [ ] Animations (movement, combat)
- [ ] Particle effects

### Phase 4: Multiplayer
- [ ] Basic AI opponent
- [ ] Network multiplayer
- [ ] Replay system
- [ ] Tournament mode

## References

- Original JavaScript source: /mnt/project/*.js
- Raylib documentation: https://www.raylib.com/cheatsheet/cheatsheet.html
- Panzer General 2 game manual: For rule details
- Hex grid guide: https://www.redblobgames.com/grids/hexagons/
