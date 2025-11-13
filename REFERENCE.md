# OpenPanzer Gameplay Systems - Implementation Progress
 
**Based on:** ref/REFERENCE.md analysis
**Current Status:** Core gameplay functional, many systems simplified or missing
**Goal:** Implement essential gameplay systems, defer UI/campaign/polish
 
---
 
## ✅ Already Implemented (Core Functional)
 
### Map & Rendering
- ✅ Hex-based map with offset coordinates
- ✅ Hex rendering with terrain colors
- ✅ Camera panning (arrow keys + WASD)
- ✅ Camera zoom (mouse wheel, R/F keys)
- ✅ Map centering and viewport management
- ✅ Victory hex markers (gold circles)
 
### Units
- ✅ Unit structure with all properties
- ✅ 6 unit classes (Infantry, Tank, Artillery, Recon, Anti-Tank, Air Defense)
- ✅ Unit rendering (rectangles with symbols)
- ✅ Unit selection (mouse click)
- ✅ Unit info panel (shows stats)
- ✅ Unit strength tracking (1-10)
- ✅ Experience tracking (0-5 bars)
- ✅ Entrenchment tracking (0-5 levels)
- ✅ Fuel and ammo tracking
- ✅ hasMoved/hasFired/hasResupplied flags
 
### Movement
- ✅ Basic movement range calculation (simple distance)
- ✅ Movement highlighting (green hexes)
- ✅ Click to move functionality
- ✅ Movement point deduction
- ✅ Fuel consumption on move
- ✅ movesLeft tracking
 
### Combat
- ✅ Attack range calculation (range 1 or gunrange)
- ✅ Attack highlighting (red hexes)
- ✅ Click to attack functionality
- ✅ Basic combat calculation (simplified formula)
- ✅ Damage application
- ✅ Experience gain (partial)
- ✅ Ammo consumption on attack
- ✅ Unit destruction at strength 0
- ✅ hasFired flag
 
### Turn System
- ✅ Turn counter (current/max)
- ✅ Player switching (SPACE key)
- ✅ Turn end processing
- ✅ Reset unit states on turn end
- ✅ Hot-seat multiplayer support
 
### UI
- ✅ Status bar (turn, player, zoom)
- ✅ Unit info panel
- ✅ Options menu with live settings
- ✅ FPS display
- ✅ Help text area
 
---
 
## 🔧 To Implement (Essential Gameplay)
 
These systems are **critical** for authentic Panzer General gameplay and should be implemented:
 
### 1. **Terrain Movement Costs** ⚠️ HIGH PRIORITY
**Status:** Tracked but not applied
**Current:** All terrain costs 1 movement point
**Need:** Full movement cost tables
 
**Implementation:**
```cpp
// Add to game (openwanzer.cpp around line 30-60)
const int MOVEMENT_METHODS = 12;
const int TERRAIN_TYPES = 18;
 
// Movement cost table [movMethod][terrain]
const int MOV_TABLE_DRY[MOVEMENT_METHODS][TERRAIN_TYPES] = {
    // Tracked: Clear, City, Airfield, Forest, Bocage, Hill, Mountain, Sand, Swamp, Ocean, River, Fort, Port, Stream, Escarp, ImpassRiver, Rough, Road
    {1, 1, 1, 2, 4, 2, 254, 1, 4, 255, 254, 1, 1, 2, 255, 255, 2, 1},
    // ... (copy from REFERENCE.md prototypes.js:134-148)
};
 
// In getMoveRange/moveUnit:
int terrainCost = MOV_TABLE_DRY[unit->movMethod][hex.terrain];
if (hex.road > roadType.none) terrainCost = MOV_TABLE_DRY[unit->movMethod][17]; // Road
```
 
**Files to modify:** `openwanzer.cpp` lines 590-652
**Estimated effort:** 2-3 hours
**Priority:** HIGH - Affects all movement
 
---
 
### 2. **Zone of Control (ZOC)** ⚠️ HIGH PRIORITY
**Status:** Not implemented
**Current:** Units can move freely adjacent to enemies
**Need:** Enemy units restrict movement through adjacent hexes
 
**Implementation:**
```cpp
// Add to GameHex structure (line ~225)
struct GameHex {
    // ... existing fields ...
    int zoc[2];  // ZOC counter per side (like isSpotted)
 
    bool isZOC(int side) const { return zoc[side] > 0; }
    void setZOC(int side, bool on) {
        if (on) zoc[side]++;
        else if (zoc[side] > 0) zoc[side]--;
    }
};
 
// Add ZOC management functions (around line 575)
void setUnitZOC(GameState &game, Unit *unit, bool on) {
    if (!unit || isAir(unit)) return;
 
    HexCoord pos = unit->position;
    std::vector<HexCoord> adjacent = getAdjacent(pos.row, pos.col);
 
    for (const auto& adj : adjacent) {
        if (adj.row >= 0 && adj.row < MAP_ROWS && adj.col >= 0 && adj.col < MAP_COLS) {
            game.map[adj.row][adj.col].setZOC(unit->side, on);
        }
    }
}
 
// In highlightMovementRange (line ~594):
// When calculating movement cost:
int enemySide = 1 - unit->side;
if (hex.isSpotted[unit->side] && hex.isZOC(enemySide) && cost < 254) {
    cost = 254;  // Stop movement
}
```
 
**Files to modify:** `openwanzer.cpp` lines 225-240, 575-652
**Estimated effort:** 3-4 hours
**Priority:** HIGH - Major tactical element
 
---
 
### 3. **Spotting & Fog of War** ⚠️ HIGH PRIORITY
**Status:** Hex spotting tracked but not enforced
**Current:** All hexes/units visible
**Need:** Only show spotted hexes and units
 
**Implementation:**
```cpp
// Already have isSpotted[2] in GameHex - just need to use it
 
// Add spotting range management (around line 575)
void setUnitSpotRange(GameState &game, Unit *unit, bool on) {
    if (!unit) return;
 
    HexCoord pos = unit->position;
    int range = unit->spotRange;
    std::vector<HexCoord> cells = getCellsInRange(pos.row, pos.col, range);
    cells.push_back(pos);  // Include unit's own hex
 
    for (const auto& cell : cells) {
        if (cell.row >= 0 && cell.row < MAP_ROWS && cell.col >= 0 && cell.col < MAP_COLS) {
            game.map[cell.row][cell.col].setSpotted(unit->side, on);
        }
    }
}
 
// In drawMap (line ~474):
// Only draw hexes/units that are spotted by current player
for (int row = 0; row < MAP_ROWS; row++) {
    for (int col = 0; col < MAP_COLS; col++) {
        GameHex &hex = game.map[row][col];
 
        // Skip if not spotted by current player
        if (!hex.isSpotted[game.currentPlayer]) continue;
 
        // Draw hex and unit...
    }
}
```
 
**Files to modify:** `openwanzer.cpp` lines 474-574, 639-652
**Estimated effort:** 2-3 hours
**Priority:** HIGH - Core tactical visibility
 
---
 
### 4. **Full Combat Formula** ⚠️ MEDIUM PRIORITY
**Status:** Simplified formula only
**Current:** Basic attack - defense + randomness
**Need:** Complete PG2 combat calculation with all modifiers
 
**Implementation:**
```cpp
// Replace performAttack (line ~654) with full calculation:
 
void performAttack(GameState &game, Unit *attacker, Unit *defender) {
    if (!attacker || !defender || attacker->hasFired) return;
 
    // Get unit data and positions
    int distance = hexDistance(attacker->position, defender->position);
    GameHex &atkHex = game.map[attacker->position.row][attacker->position.col];
    GameHex &defHex = game.map[defender->position.row][defender->position.col];
 
    // Determine attack/defense values based on target type
    int aav = 0, adv = 0, dav = 0, ddv = 0;
 
    // Base values (see REFERENCE.md:422-464)
    if (isHardTarget(defender)) aav = attacker->hardAttack;
    else aav = attacker->softAttack;
 
    adv = attacker->groundDefense;
    dav = defender->softAttack;  // TODO: check attacker type
    ddv = defender->groundDefense;
 
    // Apply modifiers:
    // 1. Experience
    int aExpBars = attacker->experience / 100;
    int dExpBars = defender->experience / 100;
    aav += aExpBars; adv += aExpBars;
    dav += dExpBars; ddv += dExpBars;
 
    // 2. Entrenchment
    adv += attacker->entrenchment;
    ddv += defender->entrenchment;
 
    // 3. Terrain (see REFERENCE.md:492-512)
    if (defHex.terrain == TerrainType::CITY) ddv += 4;
    if (defHex.terrain == TerrainType::WATER && defHex.road == 0) {
        ddv -= 4;
        aav += 4;
    }
 
    // 4. Initiative
    int initDiff = attacker->initiative - defender->initiative;
    if (initDiff >= 0) {
        adv += 4;
        aav += std::min(4, initDiff);
    } else {
        ddv += 4;
        dav += std::min(4, -initDiff);
    }
 
    // 5. Range defense modifier
    if (distance > 1) {
        adv += attacker->rangeDefMod;
        ddv += defender->rangeDefMod;
    }
 
    // 6. Previous hits
    adv -= attacker->hits;
    ddv -= defender->hits;
 
    // Calculate kills (see REFERENCE.md:586-602)
    int kills = calculateKills(aav, ddv, attacker, defender);
 
    // Defender can fire back?
    bool defCanFire = (distance <= 1 || (isSea(attacker) && isSea(defender)));
    int losses = 0;
    if (defCanFire && defender->ammo > 0) {
        losses = calculateKills(dav, adv, defender, attacker);
    }
 
    // Apply damage
    defender->strength = std::max(0, defender->strength - kills);
    attacker->strength = std::max(0, attacker->strength - losses);
 
    // Experience gain (see REFERENCE.md:572-581)
    int bonusAD = std::max(1, dav + 6 - adv);
    int atkExpGain = (bonusAD * (defender->strength / 10) + bonusAD) * kills;
    int defExpGain = 2 * losses;
 
    attacker->experience = std::min(500, attacker->experience + atkExpGain);
    defender->experience = std::min(500, defender->experience + defExpGain);
 
    // Mark as fired
    attacker->hasFired = true;
    attacker->ammo = std::max(0, attacker->ammo - 1);
 
    // Increment hits
    attacker->hits++;
    defender->hits++;
 
    // Reduce entrenchment on hit
    if (kills > 0 && defender->entrenchment > 0) defender->entrenchment--;
    if (losses > 0 && attacker->entrenchment > 0) attacker->entrenchment--;
 
    // Remove destroyed units
    game.units.erase(std::remove_if(game.units.begin(), game.units.end(),
        [](const std::unique_ptr<Unit> &u) { return u->strength <= 0; }),
        game.units.end());
}
 
int calculateKills(int atkVal, defVal, Unit *attacker, Unit *defender) {
    int kF = atkVal - defVal;
 
    // PG2 formula
    if (kF > 4) kF = 4 + (2 * kF - 8) / 5;
    kF += 6;
 
    // Artillery/Bomber penalty
    if (attacker->unitClass == UnitClass::ARTILLERY) kF -= 3;
 
    // Clamp
    kF = std::max(1, std::min(19, kF));
 
    return std::round((5.0f * kF * attacker->strength + 50) / 100);
}
```
 
**Files to modify:** `openwanzer.cpp` lines 654-690
**Estimated effort:** 4-5 hours
**Priority:** MEDIUM - Combat works but could be more accurate
 
---
 
### 5. **Entrenchment Gain** ⚠️ MEDIUM PRIORITY
**Status:** Tracked but not gained
**Current:** Entrenchment never increases
**Need:** Units gain entrenchment when stationary
 
**Implementation:**
```cpp
// Add terrain entrenchment table (around line 60)
const int TERRAIN_ENTRENCHMENT[10] = {
    0,  // PLAINS
    3,  // CITY
    2,  // FOREST
    2,  // MOUNTAIN
    1,  // HILL
    0,  // DESERT
    0,  // SWAMP
    3,  // FORTIFICATION
    0,  // WATER
    2   // ROUGH
};
 
const int UNIT_ENTRENCH_RATE[6] = {
    3,  // INFANTRY
    1,  // TANK
    2,  // RECON
    2,  // ANTI_TANK
    1,  // AIR_DEFENSE
    2   // ARTILLERY
};
 
// In endTurn (line ~692), after resetting flags:
for (auto &unit : game.units) {
    if (unit->side == game.currentPlayer) {
        // Gain entrenchment if didn't move
        if (!unit->hasMoved) {
            entrenchUnit(unit, game.map[unit->position.row][unit->position.col]);
        } else {
            unit->entrenchment = 0;  // Lost on move
        }
 
        // Reset for next turn
        unit->hasMoved = false;
        unit->hasFired = false;
        unit->movesLeft = unit->movementPoints;
        unit->hits = 0;
    }
}
 
void entrenchUnit(Unit *unit, const GameHex &hex) {
    int terrainEntrench = TERRAIN_ENTRENCHMENT[hex.terrain];
    int uc = (int)unit->unitClass;
 
    if (unit->entrenchment >= terrainEntrench) {
        // Slow gain above terrain level
        int level = unit->entrenchment - terrainEntrench;
        int nextThreshold = 9 * level + 4;
        int expBars = unit->experience / 100;
 
        unit->entrenchTicks += expBars + (terrainEntrench + 1) * UNIT_ENTRENCH_RATE[uc];
 
        while (unit->entrenchTicks >= nextThreshold &&
               unit->entrenchment < terrainEntrench + 5) {
            unit->entrenchTicks -= nextThreshold;
            unit->entrenchment++;
            level++;
            nextThreshold = 9 * level + 4;
        }
    } else {
        // Instant gain to terrain level
        unit->entrenchment = terrainEntrench;
        unit->entrenchTicks = 0;
    }
}
```
 
**Files to modify:** `openwanzer.cpp` lines 60, 692-720
**Estimated effort:** 2 hours
**Priority:** MEDIUM - Adds defensive depth
 
---
 
### 6. **Pathfinding** ⚠️ MEDIUM PRIORITY
**Status:** Uses simple distance
**Current:** Straight-line movement ignoring terrain
**Need:** Dijkstra pathfinding around obstacles
 
**Implementation:**
```cpp
// Add after hexDistance (around line ~592)
std::vector<HexCoord> findShortestPath(GameState &game, Unit *unit,
                                       HexCoord start, HexCoord end,
                                       const std::vector<Cell> &moveRange) {
    // Dijkstra's algorithm (see REFERENCE.md:253-335)
    std::vector<PathCell> cells;
    std::vector<PathCell> visited;
 
    // Initialize
    PathCell startCell = {start, 0, nullptr, 0};
    cells.push_back(startCell);
 
    for (const auto &cell : moveRange) {
        PathCell pc = {cell.pos, cell.cost, nullptr, INFINITY};
        cells.push_back(pc);
    }
 
    while (!cells.empty()) {
        // Find minimum distance cell
        auto minIt = std::min_element(cells.begin(), cells.end(),
            [](const PathCell &a, const PathCell &b) { return a.dist < b.dist; });
 
        if (minIt->dist == INFINITY) break;
 
        PathCell current = *minIt;
        cells.erase(minIt);
 
        // Found destination?
        if (current.pos == end) {
            // Backtrack to build path
            std::vector<HexCoord> path;
            PathCell *node = &current;
            while (node != nullptr) {
                path.insert(path.begin(), node->pos);
                node = node->prev;
            }
            return path;
        }
 
        // Update adjacent cells
        std::vector<HexCoord> adjacent = getAdjacent(current.pos.row, current.pos.col);
        for (const auto &adj : adjacent) {
            auto cellIt = std::find_if(cells.begin(), cells.end(),
                [&adj](const PathCell &c) { return c.pos == adj; });
 
            if (cellIt != cells.end()) {
                int newDist = current.dist + cellIt->cost;
                if (newDist < cellIt->dist) {
                    cellIt->dist = newDist;
                    cellIt->prev = &visited.back();  // Point to current in visited list
                }
            }
        }
 
        visited.push_back(current);
    }
 
    return {};  // No path found
}
```
 
**Files to modify:** `openwanzer.cpp` lines 592, 639-652
**Estimated effort:** 3-4 hours
**Priority:** MEDIUM - Improves movement realism
 
---
 
### 7. **Supply & Reinforcement** ⚠️ LOW PRIORITY
**Status:** Not implemented
**Current:** No way to resupply or heal units
**Need:** Resupply and reinforcement actions
 
**Implementation:**
```cpp
// Add functions (around line 690)
Supply calculateResupply(GameState &game, Unit *unit) {
    if (unit->hasMoved || unit->hasFired || unit->hasResupplied)
        return {0, 0};
    if (unit->fuel == unit->maxFuel && unit->ammo == unit->maxAmmo)
        return {0, 0};
 
    int ammo = unit->maxAmmo - unit->ammo;
    int fuel = unit->maxFuel - unit->fuel;
 
    // Air units need airfield
    if (isAir(unit)) {
        if (!isNearAirfield(game, unit)) return {0, 0};
        return {ammo, fuel};
    }
 
    // Terrain modifier
    GameHex &hex = game.map[unit->position.row][unit->position.col];
    if (hex.terrain != TerrainType::CITY) {
        ammo /= 2;
        fuel /= 2;
    }
 
    // Enemy modifier
    int enemies = countAdjacentEnemies(game, unit);
    if (enemies > 0 && enemies <= 2) {
        ammo /= 2;
        fuel /= 2;
    } else if (enemies > 2) {
        ammo /= 4;
        fuel /= 4;
    }
 
    return {ammo, fuel};
}
 
void resupplyUnit(GameState &game, Unit *unit) {
    Supply s = calculateResupply(game, unit);
    unit->ammo += s.ammo;
    unit->fuel += s.fuel;
    unit->hasResupplied = true;
    unit->hasMoved = true;  // Can't move after resupply
    unit->hasFired = true;  // Can't attack after resupply
}
 
// Similar for reinforcement (see REFERENCE.md:652-684)
```
 
**Files to modify:** `openwanzer.cpp` lines 690
**Estimated effort:** 3 hours
**Priority:** LOW - Nice to have but not critical
 
---
 
## 🚫 Intentionally NOT Implementing
 
These systems are **out of scope** for the current prototype. Focus is on core gameplay only.
 
### UI/Polish Systems
- ❌ **Advanced UI Windows** (equipment, buy, deploy, upgrade)
- ❌ **Mini-map** (small overview map)
- ❌ **Unit List Panel** (sidebar with all units)
- ❌ **Scenario Selection Menu**
- ❌ **Victory Dialog** (end-of-scenario summary)
- ❌ **Animations** (movement, combat, explosions)
- ❌ **Particle Effects** (smoke, fire, tracers)
- ❌ **Sound Effects** (combat sounds, movement sounds)
- ❌ **Music** (background music)
- ❌ **Unit Sprites** (using rectangles is fine for prototype)
- ❌ **Terrain Graphics** (color-coded hexes sufficient)
 
### Campaign Systems
- ❌ **Campaign Mode** (multi-scenario progression)
- ❌ **Core Units** (persistent units across scenarios)
- ❌ **Prestige Economy** (buying/upgrading units)
- ❌ **HQ Pool** (undeployed units)
- ❌ **Unit Deployment** (placing units at scenario start)
- ❌ **Unit Upgrades** (improving equipment mid-campaign)
- ❌ **Branching Scenarios** (different paths based on performance)
 
### Advanced Features
- ❌ **AI Opponent** (hot-seat is sufficient)
- ❌ **Network Multiplayer** (beyond scope)
- ❌ **Save/Load System** (restart scenario instead)
- ❌ **Scenario Editor** (use hardcoded map)
- ❌ **Scenario Loading** (from XML files)
- ❌ **Equipment Database** (500+ units - use simplified set)
- ❌ **Weather System** (fair weather only)
- ❌ **Ground Conditions** (dry only)
- ❌ **Leader Abilities** (33 special abilities - skip)
- ❌ **Transport System** (mounting/dismounting/carriers)
- ❌ **Air Layer** (separate air/ground - single layer OK)
- ❌ **Naval Units** (focus on ground combat)
- ❌ **Bridges** (treat rivers as impassable or use roads)
- ❌ **Airfields/Ports** (special terrain for air/naval - skip)
- ❌ **Support Fire** (artillery supporting adjacent attacks)
- ❌ **Recon Movement** (phased movement - move as single action)
- ❌ **Surprise Mechanics** (out of the sun, ZOC surprise)
- ❌ **Retreat System** (unit automatically retreats when damaged)
 
---
 
## 📊 Implementation Priority Summary
 
### Critical (Do First)
1. **Terrain Movement Costs** - 2-3 hours
2. **Zone of Control** - 3-4 hours
3. **Spotting & Fog of War** - 2-3 hours
 
**Total:** ~10 hours for core tactical gameplay
 
### Important (Do Second)
4. **Full Combat Formula** - 4-5 hours
5. **Entrenchment Gain** - 2 hours
6. **Pathfinding** - 3-4 hours
 
**Total:** ~10 hours for combat depth
 
### Nice to Have (Optional)
7. **Supply & Reinforcement** - 3 hours
8. **Road System** - 2 hours
9. **Victory Conditions Check** - 1 hour
 
**Total:** ~6 hours for completeness
 
---
 
## 🎯 Recommended Implementation Order
 
**Phase 1: Tactical Basics** (Critical - Day 1)
1. Terrain Movement Costs
2. Zone of Control
3. Spotting & Fog of War
 
→ Result: Authentic tactical movement
 
**Phase 2: Combat Depth** (Important - Day 2)
4. Full Combat Formula
5. Entrenchment Gain
6. Pathfinding
 
→ Result: Realistic combat and unit progression
 
**Phase 3: Polish** (Optional - Day 3)
7. Supply & Reinforcement
8. Victory Condition Enforcement
9. Bug fixes and playtesting
 
→ Result: Complete tactical wargame experience
 
---
 
## 📝 Testing Checklist
 
After implementing each system, test:
 
**Terrain Costs:**
- [ ] Infantry moves 1 in plains, 2 in forest
- [ ] Tanks can't enter mountains (cost 254)
- [ ] Wheeled units struggle in forest/rough
- [ ] Roads override terrain costs
 
**Zone of Control:**
- [ ] Can't move past enemy unit to adjacent hex
- [ ] Can enter enemy ZOC but stops there (cost 254)
- [ ] ZOC only affects spotted hexes
- [ ] Air units ignore ZOC
 
**Fog of War:**
- [ ] Only see hexes within unit spotting range
- [ ] Can't see enemy units on unspotted hexes
- [ ] Can't attack units on unspotted hexes
- [ ] Spotting reveals hidden units
 
**Combat:**
- [ ] Infantry in city gets +4 defense
- [ ] Experience adds to attack/defense
- [ ] Entrenchment adds to defense
- [ ] Initiative affects who shoots first
- [ ] Defender can't fire back from range > 1
 
**Entrenchment:**
- [ ] Units gain entrenchment when don't move
- [ ] Entrenchment lost when unit moves
- [ ] Faster gain in favorable terrain
- [ ] Max 5 levels total
 
**Pathfinding:**
- [ ] Units path around impassable terrain
- [ ] Units use roads when available
- [ ] Path avoids ZOC when possible
- [ ] Shows actual movement cost
 
---
 
## 📈 Current vs Target State
 
| System | Current | Target | Gap |
|--------|---------|--------|-----|
| Movement | Simple distance | Terrain costs + ZOC | Medium |
| Combat | Basic formula | Full PG2 formula | Medium |
| Visibility | All visible | Fog of war | Large |
| Entrenchment | Tracked only | Gains over time | Small |
| Pathfinding | Straight line | Dijkstra around obstacles | Medium |
| Supply | None | Resupply/reinforce | Large |
| Experience | Partial gain | Full formula | Small |
| Victory | Tracked but not checked | Win/loss enforcement | Small |
 
---
 
## 🔮 Future Expansion Possibilities
 
If time permits after core systems:
 
### Easy Additions (1-2 hours each)
- More unit types (just add to enum)
- More terrain types (add to terrain enum)
- Camera zoom limits
- Unit facing display
- Strength bar display
- Victory condition popup
 
### Medium Additions (Half day each)
- Basic AI opponent (random valid moves)
- Scenario loading from file
- Save/Load current game
- Unit sprite rendering
- Terrain tile graphics
- Sound effects
 
### Large Additions (Multiple days)
- Campaign mode
- Advanced AI with strategy
- Network multiplayer
- Scenario editor
- Full equipment database
- Leader abilities system
 
---
 
**Status:** Ready to implement Phase 1 (Tactical Basics)
**Next Step:** Begin with Terrain Movement Costs
**Estimated Total Time:** 20-26 hours for Phases 1-3
