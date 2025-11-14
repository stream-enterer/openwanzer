# OpenWanzer - Panzer General 2 C++ Prototype

**Language**: C++17 | **Graphics**: Raylib 4.0+ | **GUI**: RayGUI 3.0+ | **Architecture**: Single-file with namespaces (~3095 lines)

⚠️ **MANDATORY**: Read `CONVENTIONS.md` before coding. All code must follow namespace organization and naming conventions.

---

## Quick Reference

### File Structure
```
openwanzer.cpp          # Single source file, organized by sections
├─ SECTION 1  : Includes & Preprocessor
├─ SECTION 2  : Constants & Globals (HEX_SIZE, MAP_ROWS, SCREEN_WIDTH)
├─ SECTION 3  : Enums & Data Tables (TerrainType, UnitClass, MovMethod)
├─ SECTION 4  : GameLogic::Utilities (terrain costs, facing, distance)
├─ SECTION 5  : Data Structures (Unit, GameHex, GameState, etc.)
├─ SECTION 5A : Config::StyleDiscovery
├─ SECTION 6  : Rendering::Core (hex drawing, map, units, indicators)
├─ SECTION 6B : Rendering::UI (combat log, unit info, options menu)
├─ SECTION 7  : GameLogic::Mechanics (pathfinding, combat, ZOC, FOW)
├─ SECTION 8  : Input::Camera (centering, panning)
├─ SECTION 8B : Input::Zoom (mouse wheel, keyboard)
├─ SECTION 9  : Config::Persistence (save/load settings)
└─ SECTION 10 : main() (game loop, event handling, render pipeline)
```

### Key Namespaces
- **`Rendering::`** - All visual output (hex drawing, UI, menus)
- **`GameLogic::`** - Game rules (pathfinding, combat, ZOC, FOW, turns)
- **`Input::`** - User input handling (camera, zoom, pan)
- **`Config::`** - Settings management (styles, persistence)

### Core Data Structures (Global Scope)
```cpp
struct GameState {          // Main game container
  vector<vector<GameHex>> map;     // 2D hex grid
  vector<unique_ptr<Unit>> units;  // All units (ownership)
  Unit* selectedUnit;              // Raw pointer (non-owning)
  int currentPlayer;               // 0=Axis, 1=Allied
  int currentTurn;
  VideoSettings settings;
  CameraState camera;
  CombatLog combatLog;
  UnitInfoBox unitInfoBox;
  MovementSelection movementSel;   // Two-phase move system
  bool showOptionsMenu;
};

struct Unit {               // Military unit
  HexCoord position;        // Grid location
  UnitClass unitClass;      // INFANTRY, TANK, ARTILLERY, etc.
  Side side;                // AXIS or ALLIED
  int strength, maxStrength;
  int experience, entrenchment;
  int hardAttack, softAttack, groundDefense, closeDefense;
  float facing;             // 0-360° (E=0°, S=90°, W=180°, N=270°)
  bool hasMoved, hasFired;
  // ... fuel, ammo, movement, spotting
};

struct GameHex {            // Single hex tile
  TerrainType terrain;      // PLAINS, FOREST, MOUNTAIN, etc.
  int owner;                // -1=neutral, 0=Axis, 1=Allied
  bool isVictoryHex;
  bool isMoveSel, isAttackSel;  // Highlight states
  bool isSpotted[2];            // FOW per side
  bool isZOC[2];                // Zone of Control per side
};

struct MovementSelection {  // Two-phase move system
  bool isFacingSelection;   // Phase 2: selecting facing after move
  HexCoord oldPosition;     // For undo
  float selectedFacing;     // Preview facing (0-360°)
  // ... oldMovesLeft, oldHasMoved, oldFuel for undo
};
```

---

## Architecture Overview

### Module Boundaries (Future Split-File Design)

#### **Core Data Layer** (Would be: `types.h`, `enums.h`)
- **Enums**: `TerrainType`, `UnitClass`, `Side`, `MovMethod`
- **Structs**: `HexCoord`, `GameHex`, `Unit`, `GameState`, `VideoSettings`
- **Constants**: `MAP_ROWS`, `MAP_COLS`, `HEX_SIZE`, movement cost tables

#### **Rendering Module** (Would be: `rendering.h/.cpp`)
- **Dependencies**: Core Data, Raylib, hex.h library
- **Exports**: `drawMap()`, `drawUI()`, `drawOptionsMenu()`, `drawCombatLog()`
- **Internal**: Color mapping, hex coordinate conversion, shape drawing
- **No game logic**: Only reads GameState, doesn't modify it

#### **Game Logic Module** (Would be: `game_logic.h/.cpp`)
- **Dependencies**: Core Data
- **Exports**:
  - Pathfinding: `findPath()`
  - Combat: `performAttack()`, `calculateKills()`
  - Systems: `setUnitZOC()`, `setUnitSpotRange()`, `endTurn()`
  - Queries: `hexDistance()`, `isAir()`, `isHardTarget()`
- **Mutates**: `GameState` (units, map, selections)
- **Deterministic**: No rendering, no input, pure game rules

#### **Input Module** (Would be: `input.h/.cpp`)
- **Dependencies**: Core Data, Raylib
- **Exports**: `handleCombatLogScroll()`, `handleZoom()`, `handlePan()`
- **Mutates**: Camera state, UI drag states
- **Delegates**: Game actions to GameLogic module

#### **Config Module** (Would be: `config.h/.cpp`)
- **Dependencies**: Core Data, Raylib (filesystem)
- **Exports**: `saveConfig()`, `loadConfig()`, `applyGuiScale()`, `discoverStyles()`
- **IO**: Reads/writes config files, discovers GUI themes

#### **Main Loop** (Would be: `main.cpp`)
- **Orchestrates**: Initialization → Event Loop → Shutdown
- **Event Flow**: Input → Game Logic → Rendering
- **Owns**: `GameState` instance

---

## Control Flow & Interaction Patterns

### Game Loop (60 FPS)
```
main() {
  Initialize Raylib, GameState, load config

  while (!WindowShouldClose()) {
    // 1. INPUT PHASE
    if (!showOptionsMenu) {
      Handle right-click (undo/deselect)
      Update facing preview (Phase 2)
      Handle left-click (select/move/attack/confirm facing)
      Handle keyboard (zoom: R/F, pan: arrows/WASD, turn: SPACE)
      Handle mouse wheel zoom
      Handle middle mouse drag (pan)
      Handle combat log scroll/drag
    } else {
      Handle options menu input (ESC, dropdowns, sliders, Apply)
    }

    // 2. RENDERING PHASE
    BeginDrawing()
      ClearBackground()
      drawMap(game)        // Terrain, borders, highlights
                           // Units (rotated), selection boxes, facing indicators
      drawUI(game)         // Turn counter, combat log, unit info, buttons
      if (showOptionsMenu) drawOptionsMenu(game)
    EndDrawing()
  }

  Save config, cleanup
}
```

### Two-Phase Movement System
```
Phase 1: Movement Selection
- User clicks owned unit → showOptionsMenu movement range (green hexes)
- User clicks destination → unit moves immediately
- Enter Phase 2

Phase 2: Facing Selection
- Display angle indicator (">") pointing at mouse cursor
- Preview exact angle (0-360°, no hex snapping)
- User clicks anywhere → confirm facing, set unit->facing
- Show attack range if unit hasn't fired
- Reset to Phase 1
```

### Combat System
```
performAttack(attacker, defender) {
  1. Calculate base attack/defense values
  2. Apply modifiers (terrain, experience, entrenchment, range)
  3. Roll random kills for both sides
  4. Apply damage (reduce strength)
  5. Update state (hasFired, experience gains)
  6. Log message to combat log
  7. Check for unit destruction
}
```

### Pathfinding (BFS with Movement Costs)
```
findPath(game, unit, start, goal) {
  - Use BFS with PathNode (coord, movementUsed, parent)
  - Check terrain cost via getMovementCost(unit->movMethod, terrain)
  - Respect ZOC (units stop in enemy ZOC unless aircraft)
  - Respect max movement (unit->movesLeft)
  - Return path from start to goal (or empty if unreachable)
}
```

### Zone of Control (ZOC)
```
setUnitZOC(game, unit, on) {
  - For each adjacent hex to unit position
  - Set/clear isZOC[enemySide] flag
  - Land units project ZOC to 6 neighbors
  - Used by pathfinding to restrict movement
}
```

### Fog of War (FOW)
```
setUnitSpotRange(game, unit, on) {
  - For each hex within unit->spotRange (default 2)
  - Set/clear isSpotted[unit->side] flag
  - Used by rendering to hide enemy units
  - Updated on unit move, turn end
}
```

---

## Critical Implementation Details

### Hex Coordinate Systems
**Offset Coordinates** (storage): `{row, col}` where odd columns shift down by 0.5 hex height
**Cube Coordinates** (math): `{q, r, s}` where `q + r + s = 0` (used for distance calculations)
**Pixel Coordinates**: `{x, y}` screen position

```cpp
// Conversions (via hex.h library)
Hex offset_to_cube(OffsetCoord offset);
Point hex_to_pixel(Layout layout, Hex hex);
Hex pixel_to_hex(Layout layout, Point point);

// Distance (Manhattan distance in cube space)
int hexDistance(HexCoord a, HexCoord b) {
  Hex cubeA = offset_to_cube(offsetA);
  Hex cubeB = offset_to_cube(offsetB);
  return (abs(cubeA.q - cubeB.q) + abs(cubeA.r - cubeB.r) + abs(cubeA.s - cubeB.s)) / 2;
}
```

### Unit Facing System (Degree-Based)
- **Storage**: `float facing` (0-360°), exact angle, no hex snapping
- **Display**: Hybrid notation "NNE (018°)" (16-point compass rose + degrees)
- **Initialization**: Axis=0° (East), Allied=180° (West)
- **Visual Indicator**: ">" arrow shape, drawn with `DrawLineEx()`, appears when unit selected
- **Selection Box**: Rotates with unit using rotation matrix for all 4 corners
- **Calculation**: `atan2(dy, dx) * 180 / PI` for exact angle from mouse position

### RayGUI Immediate Mode Pattern
```cpp
// CORRECT: Pass variables by pointer
GuiCheckBox(bounds, "Label", &game.settings.vsync);
GuiSlider(bounds, nullptr, nullptr, &game.settings.hexSize, 20, 80);

// Dropdowns need explicit edit state tracking
if (GuiDropdownBox(bounds, "1920x1080;1280x720", &index, game.settings.resolutionDropdownEdit)) {
  game.settings.resolutionDropdownEdit = !game.settings.resolutionDropdownEdit;
}
```

### Mutable Globals (Window Configuration)
```cpp
// Updated at runtime when resolution changes
int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;
float HEX_SIZE = 30.0f;
float HEX_WIDTH = HEX_SIZE * 2.0f;
float HEX_HEIGHT = sqrtf(3.0f) * HEX_SIZE;
```
**Why**: Raylib rendering functions need global context. Config changes update these.

### Memory Ownership
- **`GameState::units`**: `vector<unique_ptr<Unit>>` - owns all units
- **`GameState::selectedUnit`**: `Unit*` - non-owning raw pointer to element in units vector
- **`GameState::map`**: `vector<vector<GameHex>>` - owns all hex tiles
- **Units never deleted during gameplay** (only destroyed by setting strength=0)

### Combat Log Circular Buffer
```cpp
struct CombatLog {
  vector<LogMessage> messages;  // Max 100 messages
  int scrollOffset;             // Current scroll position
  Rectangle bounds;             // Screen position (draggable)
  bool isDragging;              // Drag state
};
// Auto-scrolls to bottom on new messages
// User can scroll/drag manually
```

---

## Common Modification Patterns

### Adding New Unit Type
1. **Enum**: Add to `UnitClass` enum (line ~78)
2. **Symbol**: Update `getUnitSymbol()` (line ~820)
3. **Stats**: Add initialization case in `GameState::addUnit()` (line ~690)
4. **Classification**: Update `isAir()`, `isHardTarget()`, `isSea()` if needed

### Adding New Terrain Type
1. **Enum**: Add to `TerrainType` enum (line ~65)
2. **Color**: Update `getTerrainColor()` (line ~789)
3. **Movement**: Update `getMovementCost()` cost tables (line ~230)
4. **Entrenchment**: Update `getTerrainEntrenchment()` (line ~204)
5. **Generation**: Update `GameState::initializeMap()` random generation (line ~776)

### Adding Video Setting
1. **Struct**: Add field to `VideoSettings` (line ~300)
2. **Constructor**: Initialize in `VideoSettings()` constructor
3. **GUI**: Add control in `drawOptionsMenu()` (line ~2226)
4. **Apply**: Handle in Apply button logic (applies changes, may require restart)
5. **Persistence**: Update `saveConfig()`/`loadConfig()` (line ~2624/2646)

### Modifying Combat Formula
- **Core Logic**: `calculateKills()` (line ~1591) - returns kills based on attack/defense
- **Attack Flow**: `performAttack()` (line ~1615) - orchestrates combat, applies damage
- **Modifiers**: Experience, entrenchment, terrain, range defense
- **Randomness**: `rand() % 100 < probability` for kill chances

---

## Gotchas & Known Issues

### ESC Key Handling
```cpp
SetExitKey(KEY_NULL);  // Disable ESC closing window
// Handle ESC manually in main loop (close menu, then deselect)
```

### MSAA Requires Restart
MSAA flag must be set **before** `InitWindow()`. Changes require full restart.

### Resolution Changes Update Globals
```cpp
// When applying new resolution:
SCREEN_WIDTH = newWidth;
SCREEN_HEIGHT = newHeight;
SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
// HEX_SIZE/HEX_WIDTH/HEX_HEIGHT must also be recalculated if hex size changes
```

### RayGUI Warning Suppression
```cpp
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#include "raygui.h"
#pragma GCC diagnostic pop
```
Unavoidable warnings from library code.

### Unit Selection Edge Cases
- **Right-click in Phase 2**: Undo move, restore old position/fuel/movesLeft
- **Right-click in Phase 1**: Deselect unit, clear highlights
- **Clicking enemy unit**: Can select for info display, but cannot control
- **Movement exhausted**: Unit selected shows no green hexes, only attack range (red)

### Combat Log Z-Order
Combat log drawn **after** map but **before** options menu. Can be dragged anywhere on screen.

---

## Refactoring Guidance (Future Multi-File Split)

### Suggested File Structure
```
src/
├── core/
│   ├── types.h           # Forward declarations, basic types
│   ├── enums.h           # TerrainType, UnitClass, Side, MovMethod
│   ├── constants.h       # MAP_ROWS, movement tables, globals
│   ├── hex_coord.h       # HexCoord, coordinate conversions
│   ├── game_hex.h        # GameHex struct
│   ├── unit.h            # Unit struct
│   └── game_state.h      # GameState, VideoSettings, Camera
├── rendering/
│   ├── rendering.h       # Public API
│   ├── rendering.cpp     # drawMap(), drawUI()
│   ├── hex_drawing.cpp   # Hex geometry, colors
│   └── ui_drawing.cpp    # Combat log, unit info, menus
├── game_logic/
│   ├── game_logic.h      # Public API
│   ├── pathfinding.cpp   # findPath(), BFS
│   ├── combat.cpp        # performAttack(), calculateKills()
│   ├── systems.cpp       # ZOC, FOW, entrenchment
│   └── utilities.cpp     # Distance, terrain costs, queries
├── input/
│   ├── input.h           # Public API
│   ├── camera.cpp        # Pan, zoom, centering
│   └── event_handlers.cpp # Mouse, keyboard, drag
├── config/
│   ├── config.h          # Public API
│   ├── persistence.cpp   # Save/load settings
│   └── style_manager.cpp # GUI themes, discovery
└── main.cpp              # Game loop, initialization

include/
├── raylib.h              # External dependency
├── raygui.h              # External dependency
└── hex.h                 # External hex math library
```

### Dependency Graph (Acyclic)
```
main.cpp
  ↓
┌─────────┬─────────────┬─────────────┐
│ Input   │ Rendering   │ Config      │
└─────────┴─────────────┴─────────────┘
  ↓             ↓             ↓
┌───────────────────────────────┐
│       Game Logic              │
└───────────────────────────────┘
  ↓
┌───────────────────────────────┐
│       Core Data Types         │
└───────────────────────────────┘
  ↓
External libs (raylib, hex.h)
```

### Key Refactoring Principles
1. **Pure Functions**: GameLogic should be testable without Raylib (no rendering dependencies)
2. **Clear Ownership**: `GameState` owns all game data, passed by reference to modules
3. **Const Correctness**: Rendering takes `const GameState&`, GameLogic takes `GameState&`
4. **Forward Declarations**: Minimize header dependencies with forward declarations
5. **Namespace Preservation**: Keep `Rendering::`, `GameLogic::`, `Input::`, `Config::`
6. **No Circular Dependencies**: Main → Modules → Core, never upward

### Current Cross-Namespace Calls
```cpp
// In Rendering namespace
namespace GameLogic {
  std::vector<HexCoord> findPath(...);  // Forward declaration
}
// Later: GameLogic::findPath() called from drawMap() for path preview

// In main()
GameLogic::highlightMovementRange(game, unit);
Rendering::drawMap(game);
Input::handleZoom(game);
```

### Testing Strategy (Post-Refactor)
```cpp
// Game logic testable without Raylib
TEST(Pathfinding, FindPathBasic) {
  GameState game;
  game.initializeMap();
  Unit* unit = game.addUnit(INFANTRY, AXIS, {5, 5});
  auto path = GameLogic::findPath(game, unit, {5,5}, {7,7});
  ASSERT_EQ(path.size(), 3);
}

// Rendering tested with mock GameState
TEST(Rendering, UnitColorMapping) {
  ASSERT_EQ(Rendering::getUnitColor(0), RED);   // Axis
  ASSERT_EQ(Rendering::getUnitColor(1), BLUE);  // Allied
}
```

---

## Performance Characteristics

### Targets
- **60 FPS** @ 1920x1080 with 20 units
- **Map rendering**: < 10ms (draws ~400 hexes, ~20 units)
- **UI rendering**: < 1ms (combat log, unit info, buttons)
- **Pathfinding**: < 5ms (BFS on 20×20 grid)
- **Memory**: < 50MB total

### Bottlenecks
- **Hex border drawing**: `DrawLineEx()` called 6× per hex, 2400 calls/frame
- **Path preview**: Recalculates BFS every frame when hovering (could cache)
- **Unit rotation**: Trigonometry for rotated rectangles (sin/cos per unit)

### Optimizations Applied
- **Spotlight culling**: FOW hides enemy units (no draw calls)
- **Lazy ZOC update**: Only recalculate on unit move/turn end
- **Combat log pruning**: Max 100 messages (prevents unbounded growth)
- **Immediate mode GUI**: No retained mode overhead

---

## Build & Dependencies

### Build Commands
```bash
make               # Uses Makefile
./build.sh         # Bash script
g++ -std=c++17 -O2 -Wall -I./include openwanzer.cpp -o openwanzer \
    -L./lib -Wl,-rpath=./lib -lraylib -lm -lpthread -ldl -lrt -lX11
```

### External Dependencies
- **raylib 4.0+**: Window, graphics, input (`apt install libraylib-dev`)
- **raygui 3.0+**: GUI controls (header-only, bundled in `include/`)
- **hex.h**: Hex coordinate math (custom, bundled in `include/`)

### Runtime Requirements
- **Linux**: X11, OpenGL 3.3+
- **Config file**: `~/.config/openwanzer/config.cfg` (auto-created)
- **GUI themes**: `~/.config/openwanzer/styles/*.rgs` (optional)

---

## References

- **Hex Grid Math**: https://www.redblobgames.com/grids/hexagons/
- **Raylib API**: https://www.raylib.com/cheatsheet/cheatsheet.html
- **RayGUI Docs**: https://github.com/raysan5/raygui
- **Panzer General 2**: Original game rules (DESIGN.md for combat formulas)
- **Conventions**: `CONVENTIONS.md` - coding standards (MANDATORY READ)

---

**Version**: 3.0 (Degree-Based Facing System)
**Last Updated**: 2025-11
**Lines of Code**: ~3095
**License**: Educational Reference
