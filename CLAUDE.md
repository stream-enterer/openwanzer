# CLAUDE.md - AI Assistant Context for Open Wanzer

Last Updated: 2025-11-15

---

## 🔴 CRITICAL: SESSION-END PROTOCOL

**IMPORTANT**: At the end of EVERY session, you MUST:

1. **Scan the entire codebase** for significant changes made during the session
2. **Update this CLAUDE.md file** with:
   - New files created
   - Major refactorings performed
   - Architecture changes
   - New dependencies added
   - Breaking changes introduced
   - Updated file organization
3. **Update the "Last Updated" date** at the top of this file
4. **Update the "Recent Changes" section** with session summary

This ensures continuity across sessions and prevents knowledge loss.

---

## Project Overview

**Open Wanzer** is a turn-based tactical mech combat game written in C++17, inspired by Panzer General and BattleTech. It features hex-based tactical gameplay with an advanced armor damage system.

### Key Technologies
- **Language**: C++17
- **Graphics**: raylib 5.5.0
- **UI**: raygui (immediate mode GUI)
- **Build System**: CMake 3.15+ (with Makefile wrapper for convenience)

### Project Goals
- Classic turn-based tactical gameplay
- BattleTech-style armor system with 8 damage locations
- Hex-based movement and combat
- Multiple unit types (mechs, infantry, vehicles, aircraft)
- Fog of war and line-of-sight mechanics

---

## Codebase Structure

### Directory Layout

```
openwanzer/
├── src/                   # All .cpp source files (PascalCase)
├── include/               # All .h header files (PascalCase)
├── lib/                   # raylib binaries
├── resources/             # Fonts, themes, styles
├── tests/                 # Unit tests (future)
├── docs/                  # Documentation
├── scripts/               # Build and utility scripts
└── examples/              # Example scenarios (future)
```

### Important: Flat Directory Structure

**ALL headers go in `include/`** - no subdirectories
**ALL source files go in `src/`** - no subdirectories

### Naming Conventions

**CRITICAL: These conventions are MANDATORY for all new and refactored code.**

#### Code Elements
- **Class names**: PascalCase (e.g., `GameState`, `Unit`, `HexCoord`)
- **Method names**: PascalCase (e.g., `CalculateDamage`, `GetUnitAt`, `IsValidMove`)
- **Private member variables**: camelCase_ with trailing underscore (e.g., `unitCount_`, `maxHealth_`, `currentPlayer_`)
- **Parameters**: camelCase (e.g., `unitId`, `targetPos`, `damageAmount`)
- **Local variables**: camelCase (e.g., `result`, `hexDistance`, `isValid`)
- **Constants**: kCamelCase with k prefix (e.g., `kMaxUnits`, `kDefaultHealth`, `kHexSize`)
- **Constexpr values**: SCREAMING_SNAKE_CASE (e.g., `MAX_MAP_SIZE`, `DEFAULT_ARMOR_VALUE`)
- **Type aliases**: snake_case_t with _t suffix (e.g., `unit_ptr_t`, `hex_map_t`)
- **Namespaces**: lowercase (e.g., `gamelogic`, `rendering`, `config`)
- **Enum values**: SCREAMING_SNAKE_CASE (e.g., `TERRAIN_GRASS`, `UNIT_MECH`, `WEAPON_MISSILE`)

#### Files
- **Header files**: PascalCase.hpp (e.g., `GameState.hpp`, `Unit.hpp`)
- **Source files**: PascalCase.cpp (e.g., `GameState.cpp`, `Unit.cpp`)
- **Header guards**: SCREAMING_SNAKE_CASE (e.g., `OPENWANZER_GAME_STATE_HPP`, `OPENWANZER_UNIT_HPP`)

#### Data Files
- **JSON keys**: snake_case (e.g., `"unit_type"`, `"max_health"`, `"armor_locations"`)
- **JSON files**: lowercase_underscore (e.g., `game_config.json`, `unit_data.json`)

### Examples

```cpp
// Header file: GameState.hpp
#ifndef OPENWANZER_GAME_STATE_HPP
#define OPENWANZER_GAME_STATE_HPP

namespace gamelogic {

// Constants
constexpr int MAX_PLAYERS = 8;
constexpr int DEFAULT_MAP_SIZE = 20;
const int kMaxUnitsPerPlayer = 50;

// Type aliases
using unit_id_t = int;
using position_t = HexCoord;

// Enum
enum TerrainType {
    TERRAIN_GRASS,
    TERRAIN_FOREST,
    TERRAIN_MOUNTAIN
};

// Class
class GameState {
public:
    // Public methods: PascalCase
    void InitializeGame(int mapSize);
    bool AddUnit(unit_id_t unitId, position_t pos);
    Unit* GetUnitAt(position_t pos) const;

private:
    // Private members: camelCase_
    int currentTurn_;
    int activePlayer_;
    std::vector<std::unique_ptr<Unit>> units_;

    // Private methods: PascalCase
    void UpdateFogOfWar();
    bool IsPositionValid(position_t pos) const;
};

} // namespace gamelogic

#endif // OPENWANZER_GAME_STATE_HPP
```

---

## 🔴 CRITICAL: COMPILER WARNINGS POLICY

**ZERO TOLERANCE FOR COMPILER WARNINGS IN OUR CODE**

### Mandatory Rules

1. **EVERY BUILD MUST BE WARNING-FREE** for code in `src/` and `include/`
2. **NEVER commit code that produces compiler warnings**
3. **FIX ALL WARNINGS IMMEDIATELY** when they appear
4. **DO NOT ignore or suppress warnings** - fix the underlying issue
5. **WARNINGS FROM THIRD-PARTY LIBRARIES** (raylib, raymath.h, etc.) are acceptable but should be documented

### Common Warnings and Fixes

#### Unused Parameters
```cpp
// BAD - produces warning
void MyFunction(int unusedParam) {
    // ... code that doesn't use unusedParam
}

// GOOD - mark as intentionally unused
void MyFunction([[maybe_unused]] int unusedParam) {
    // ... code that doesn't use unusedParam
}

// BETTER - remove if truly unnecessary
void MyFunction() {
    // ... code
}
```

#### Unused Variables
```cpp
// BAD - produces warning
int result = Calculate();
// ... result never used

// GOOD - use it or remove it
int result = Calculate();
DoSomethingWith(result);

// OR cast to void if intentionally unused for side effects
(void)Calculate();
```

#### Missing Field Initializers
```cpp
// BAD - produces warning
struct Data {
    int x;
    int y;
    int z;
};
Data d = { 1 }; // missing y and z

// GOOD - initialize all fields
Data d = { 1, 0, 0 };

// BETTER - use designated initializers or constructors
Data d = { .x = 1, .y = 0, .z = 0 };
```

### Build Verification

Before committing:
```bash
make clean && make 2>&1 | grep "^src/" | grep "warning:"
```

This command MUST produce NO OUTPUT. If it shows any warnings from our source files, FIX THEM before committing.

### AI Assistant Instructions

**When you (AI assistant) write or modify code:**
1. Build immediately after changes
2. Check for ANY warnings in src/ or include/
3. Fix ALL warnings before proceeding
4. NEVER leave warnings "for later"
5. If you can't fix a warning, ASK the user rather than ignoring it

**This is non-negotiable. Warnings indicate potential bugs, portability issues, or code quality problems.**

---

### Module Organization

The codebase is logically organized into modules (though physically flat):

#### Core Module
- **GameState.hpp/cpp**: Main game state and structures
- **Unit.hpp/cpp**: Unit class and unit data
- **Enums.hpp**: Game enumerations (TerrainType, UnitClass, MovMethod, etc.)
- **Types.hpp**: Type aliases and basic types
- **Constants.hpp/cpp**: Game constants and configuration
- **HexCoord.hpp**: Hex coordinate structure
- **GameHex.hpp**: Hex tile data structure
- **ArmorLocation.hpp/cpp**: Armor location enumeration and utilities

#### Game Logic Module
- **GameLogic.hpp**: Main game logic namespace and function declarations
- **Combat.cpp**: Combat resolution and damage calculation
- **Pathfinding.cpp**: A* pathfinding and movement range calculation
- **Systems.cpp**: Fog of war, spotting, turn management
- **Utilities.cpp**: Helper functions (terrain, facing, hex math)
- **AttackLines.cpp**: Attack line visualization
- **CombatArcs.hpp/cpp**: Attack arc determination (front/side/rear)
- **DamageSystem.hpp/cpp**: Armor damage system
- **HitTables.hpp/cpp**: Hit location tables

#### Rendering Module
- **Rendering.hpp**: Main rendering namespace and function declarations
- **HexDrawing.cpp**: Hex rendering and coordinate conversion
- **UIDrawing.cpp**: UI rendering (status bar, panels, menus)
- **CombatVisuals.cpp**: Combat visualization (attack lines, highlights)
- **PaperdollUI.hpp/cpp**: BattleTech-style paperdoll mech display

#### Input Module
- **Input.hpp**: Input handling declarations
- **Camera.cpp**: Camera controls and viewport management

#### Config Module
- **Config.hpp**: Configuration management namespace
- **Persistence.cpp**: Config file loading/saving
- **StyleManager.cpp**: UI theme management

#### UI Module
- **UIPanels.hpp/cpp**: Draggable UI panels (combat log, unit info, paperdoll)

#### Entry Point
- **Main.cpp**: Program entry point and main game loop

---

## Key Systems

### Hex Grid System
- Uses axial coordinate system internally
- Converts to/from offset coordinates for display
- Flat-top hexagons
- Based on Red Blob Games hex math

### Armor System (BattleTech-inspired)
- 8 armor locations: Head, CT, LT, RT, LA, RA, LL, RL
- Each location tracks current/max armor separately
- Damage applies to specific locations via hit tables
- Destroyed locations affect unit performance
- Paperdoll UI shows visual damage representation

### Combat System
- Attack arcs (front/side/rear) affect hit tables
- Range affects hit chance
- Different hit tables for different unit types
- Hardness system (hard vs soft targets)
- Multiple weapon types (Missile, Energy, Ballistic, Artillery, Melee)

### Fog of War
- Dynamic line-of-sight system
- Spotting ranges vary by unit type
- Hidden/visible hex tracking per side

### Movement System
- Movement points based on unit type
- Terrain affects movement cost
- Two-phase selection (move, then select facing)
- Pathfinding with A* algorithm

---

## Build System

### Overview

The project uses **CMake** as the build system, with all build artifacts placed in the `build/` directory. A **Makefile wrapper** is provided for convenience, allowing you to use familiar `make` commands.

### Quick Start

```bash
make              # Build release version
make run          # Build and run the game
make clean        # Remove all build artifacts
make help         # Show all available targets
```

### Makefile Targets (CMake Wrapper)

```bash
# Build Commands
make              # Build release version (default)
make release      # Build release version
make debug        # Build debug version with symbols
make build        # Build with current configuration

# Configuration
make configure       # Configure CMake (Release)
make configure-debug # Configure CMake (Debug)

# Utilities
make clean        # Remove all build artifacts
make run          # Build and run the game
make check-deps   # Verify all dependencies are present
make format       # Format code with clang-format

# Installation
make install      # Install to /usr/local/bin (requires sudo)
make uninstall    # Remove from /usr/local/bin (requires sudo)

# Help
make help         # Show all available targets
```

### Using CMake Directly

For advanced usage, you can use CMake directly:

```bash
# Configure and build
mkdir build && cd build
cmake ..
cmake --build .

# Configure with specific build type
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Run the executable
./openwanzer
```

### Build Output

- **Binary**: `build/openwanzer`
- **Build artifacts**: `build/` directory (git-ignored)
- **Libraries**: `build/lib/` (if any)

### Dependencies
- CMake 3.15 or higher
- raylib 5.5.0 (included in `lib/` and `include/`)
- X11 libraries (Linux only)
- C++17 compiler (GCC, Clang, etc.)

---

## Common Tasks

### Adding a New Source File

1. Create `NewFile.cpp` in `src/` and `NewFile.hpp` in `include/`
2. Use header guard: `#ifndef OPENWANZER_NEW_FILE_HPP`
3. Include header in relevant files: `#include "NewFile.hpp"`
4. No need to update Makefile (uses automatic detection)
5. Follow naming conventions: PascalCase for files, methods; camelCase for variables

### Adding a New Feature

1. Determine which module it belongs to
2. Add declarations to appropriate header in `include/`
3. Implement in corresponding .cpp file in `src/`
4. Update `CLAUDE.md` with the change (at session end)
5. Update `docs/ARCHITECTURE.md` if architecture changes

### Modifying Game State

1. Edit `include/GameState.h` for new state members
2. Initialize in `GameState.cpp` constructor
3. Update serialization in `Persistence.cpp` if needed

### Adding UI Elements

1. Add rendering code to appropriate file in Rendering module
2. Add input handling to `Main.cpp` or `Camera.cpp`
3. Consider adding to `UIPanels.cpp` if draggable

---

## Code Patterns and Idioms

### Namespaces
- `gamelogic::` - All game logic functions
- `rendering::` - All rendering functions
- `config::` - Configuration functions
- `combatarcs::` - Combat arc calculations
- `damagesystem::` - Damage system functions
- `hittables::` - Hit location tables
- `input::` - Input handling
- `paperdollui::` - Paperdoll UI rendering
- `uipanel::` - UI panel management
- No namespace for core structures (GameState, Unit, etc.)

### Memory Management
- Use `std::unique_ptr` for owned entities (e.g., units)
- Use raw pointers for non-owning references
- RAII for all resources

### Error Handling
- Use assertions for invariants
- Return bool/int status codes for operations
- Log errors to console (currently minimal logging)

### Coordinate Systems
- Internal: Axial coordinates (HexCoord)
- Display: Offset coordinates (OffsetCoord)
- Always convert at rendering boundary

---

## Recent Changes

### 2025-11-15: Naming Convention Updates and Header Refactoring (Phase 1)
- **UPDATED NAMING CONVENTIONS**: Established comprehensive new naming standards
  - Methods: PascalCase (e.g., `CalculateDamage`)
  - Parameters/locals: camelCase (e.g., `unitId`, `targetPos`)
  - Private members: camelCase_ (e.g., `unitCount_`)
  - Constants: kCamelCase (e.g., `kMaxUnits`)
  - Constexpr: SCREAMING_SNAKE_CASE (e.g., `MAX_PLAYERS`)
  - Type aliases: snake_case_t (e.g., `unit_ptr_t`)
  - Header files: .hpp extension
  - Header guards: SCREAMING_SNAKE_CASE with _HPP suffix
- **RENAMED ALL HEADERS**: Changed all project headers from .h to .hpp
  - All 17 project headers renamed (ArmorLocation, CombatArcs, Config, Constants, DamageSystem, Enums, GameHex, GameLogic, GameState, HexCoord, HitTables, Input, PaperdollUI, Rendering, Types, UIPanels, Unit)
  - Third-party headers remain .h (raylib, raygui, raymath, rlgl, hex)
- **UPDATED HEADER GUARDS**: Changed all guards to proper SCREAMING_SNAKE_CASE format
  - Example: `OPENWANZER_GAMESTATE_H` → `OPENWANZER_GAME_STATE_HPP`
  - Added underscores between words (e.g., `ARMOR_LOCATION`, `GAME_STATE`, `UI_PANELS`)
- **UPDATED ALL #INCLUDE STATEMENTS**: Changed all includes from .h to .hpp for project headers (34 files updated)
- **FIXED ALL COMPILER WARNINGS** in our source code:
  - Fixed 6 unused parameter/variable warnings across 5 files
  - Used `[[maybe_unused]]` attribute for parameters reserved for future use
  - Build now produces ZERO warnings in src/ and include/ directories
- **ADDED COMPILER WARNING POLICY**: Established zero-tolerance policy
  - Mandatory rules for fixing warnings immediately
  - Examples and guidelines for common warning types
  - Build verification commands
  - Forceful instructions for AI assistants

### 2025-11-15: Namespace and Constant Refactoring (Phase 2a)
- **REFACTORED ALL NAMESPACES** to lowercase
  - GameLogic → gamelogic
  - Rendering → rendering
  - Config → config
  - CombatArcs → combatarcs
  - DamageSystem → damagesystem
  - HitTables → hittables
  - Input → input
  - PaperdollUI → paperdollui
  - UIPanel → uipanel
  - Updated all namespace declarations and usages across all files
- **REFACTORED ALL CONST CONSTANTS** to kCamelCase format
  - DEFAULT_SCREEN_WIDTH → kDefaultScreenWidth
  - DEFAULT_SCREEN_HEIGHT → kDefaultScreenHeight
  - DEFAULT_HEX_SIZE → kDefaultHexSize
  - DEFAULT_MAP_ROWS → kDefaultMapRows
  - DEFAULT_MAP_COLS → kDefaultMapCols
  - COLOR_BACKGROUND → kColorBackground
  - COLOR_GRID → kColorGrid
  - COLOR_FPS → kColorFps
  - MOV_TABLE_DRY → kMovTableDry
  - Mutable globals kept as SCREAMING_SNAKE_CASE (SCREEN_WIDTH, HEX_SIZE, etc.)
- Build status: Clean build with zero warnings

### 2025-11-15: Method and Variable Refactoring (Phase 2b)
- **REFACTORED ALL METHOD NAMES** to PascalCase (~200+ methods)
  - gamelogic:: functions (CalculateDamage, ResolveAttack, GetValidMoves, CanMoveTo, etc.)
  - rendering:: functions (DrawHexGrid, DrawUnits, DrawUI, CreateHexLayout, etc.)
  - combatarcs:: functions (GetAttackArc, GetArcColor, GetArcSegmentColor)
  - damagesystem:: functions (ApplyDamage, GetHitLocation, CalculateArmorDamage)
  - hittables:: functions (GetMechHitTable, RollHitLocation)
  - input:: functions (HandleCameraInput, UpdateCamera)
  - paperdollui:: functions (RenderPaperdoll, GetArmorColor)
  - uipanel:: functions (ShowPlayerPanel, HidePlayerPanel, ShowTargetPanel, etc.)
  - config:: functions (LoadConfig, SaveConfig, ApplyTheme)
  - Utility functions (GetTerrainName, GetFacingAngle, HexDistance, IsAir, etc.)
  - Unit class methods (GetArmorAt, SetArmorAt, TakeDamageAt, IsLocationDestroyed)
  - GameState methods (GetUnitAt, AddUnit, RemoveUnit)
- **REFACTORED ALL VARIABLES** to camelCase (~800+ variables)
  - Position variables: attackerPos, targetPos, hexPos, currentPos, etc.
  - Damage variables: baseDamage, totalDamage, armorDamage, structureDamage, etc.
  - Combat variables: hitRoll, hitChance, hitLocation, attackArc, etc.
  - Movement variables: moveCost, movementRange, maxRange, minRange, etc.
  - Type/ID variables: unitId, terrainType, terrainIndex, movMethod, etc.
  - Screen variables: screenWidth, screenHeight, hexSize, mapRows, mapCols, etc.
  - Unit references: currentUnit, selectedUnit, clickedUnit, targetUnit, etc.
  - Boolean flags: isVisible, isValid, isPlayer, isEnemy, isSelected, etc.
  - Offset variables: offsetX, offsetY, deltaX, deltaY, etc.
  - UI variables: fontSize, lineHeight, textWidth, colorFg, colorBg, etc.
- **NOTE**: External library functions (hex_to_pixel, offset_to_cube, etc.) kept as snake_case per their API
- Build status: Clean build with zero warnings

**COMPLETE**: All naming convention refactoring finished. Project now fully complies with established standards.

### 2025-11-15: Major Codebase Refactoring (Earlier Session)
- **Migrated build system** from GNU Make to CMake
- **Created CMakeLists.txt** with comprehensive configuration
  - C++17 standard enforcement
  - Automatic source file detection via globbing
  - Separate Release and Debug build configurations
  - Custom targets for format, run, and check-deps
  - Proper rpath configuration for raylib
- **Implemented Makefile wrapper** for convenience
  - Provides familiar `make` commands (build, debug, clean, run, etc.)
  - Wraps CMake commands underneath
  - Maintains backwards compatibility with previous workflow
- **Updated build output directory** to `build/`
  - All build artifacts now in `build/` directory
  - Binary located at `build/openwanzer`
  - Clean separation from source code
- **Enhanced .gitignore** with additional CMake-specific patterns
- **Archived old Makefile** as `Makefile.old`
- **Updated documentation** to reflect new build system
- **Tested successfully** - build completes without errors

### 2025-11-15: Major Codebase Refactoring
- **Restructured entire codebase** to use flat directory structure
- **Renamed all files** from snake_case to PascalCase
- **Moved all headers** from `src/*/` subdirectories to `include/`
- **Moved all source files** from `src/*/` subdirectories to `src/`
- **Updated all #include statements** to reflect new file names and locations
- **Updated header guards** to remove module prefixes (e.g., `OPENWANZER_CORE_UNIT_H` → `OPENWANZER_UNIT_H`)
- **Enhanced Makefile** with additional targets (debug, format, install, help)
- **Updated .gitignore** for C++ project standards
- **Created standard directories**: tests/, docs/, scripts/, examples/
- **Created documentation**: README.md, CLAUDE.md (this file)
- **Deleted ref/ directory** (no longer needed)
- **Standardized project naming** to "Open Wanzer" throughout

### File Mapping (Old → New)
All files moved from `src/module/snake_case.ext` to `src/PascalCase.ext` or `include/PascalCase.h` (now .hpp)

Example mappings:
- `src/core/game_state.h` → `include/GameState.h` → `include/GameState.hpp`
- `src/core/game_state.cpp` → `src/GameState.cpp`
- `src/game_logic/combat.cpp` → `src/Combat.cpp`
- `src/rendering/rendering.h` → `include/Rendering.h` → `include/Rendering.hpp`

---

## Known Issues and TODOs

### Current Limitations
- No save/load functionality
- No multiplayer support
- AI is basic
- No sound effects or music
- Limited unit variety

### Planned Features
- [ ] Campaign mode
- [ ] Unit customization
- [ ] Map editor
- [ ] Sound system
- [ ] Improved AI
- [ ] Save/load game state
- [ ] Unit experience/veterancy
- [ ] More unit types and weapons

---

## Testing

### Manual Testing Checklist
- [ ] Build completes without errors
- [ ] Game launches and shows main menu
- [ ] Units can be selected and moved
- [ ] Combat works correctly
- [ ] Fog of war functions
- [ ] Paperdoll UI displays correctly
- [ ] Options menu saves settings
- [ ] Camera controls work
- [ ] Turn progression works

### Future: Automated Testing
- Unit tests planned for `tests/` directory
- Test framework: TBD (considering Catch2 or Google Test)

---

## External Resources

### Documentation
- [raylib cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [Red Blob Games - Hexagonal Grids](https://www.redblobgames.com/grids/hexagons/)
- [BattleTech Rules](https://bg.battletech.com/) (for armor system reference)

### Dependencies
- raylib: Included in `lib/` and `include/`
- raygui: Included in `include/`
- hex.h: Custom hex math library (in `include/`)

---

## Development Workflow

### Starting a Session
1. Read this CLAUDE.md file
2. Check "Recent Changes" for context
3. Review relevant module documentation
4. Build the project to ensure it compiles

### During Development
1. Follow naming conventions strictly
2. Keep files in correct directories (flat structure)
3. Update header guards correctly
4. Test changes incrementally
5. Use `make format` to maintain code style

### Ending a Session
1. **SCAN THE CODEBASE** for all changes made
2. **UPDATE THIS CLAUDE.MD** with:
   - New files created
   - Refactorings performed
   - Architecture changes
   - Breaking changes
3. **UPDATE "Recent Changes" section**
4. **UPDATE "Last Updated" date**
5. Build and test one final time
6. Commit changes with descriptive message

---

## Contact and Contribution

See `docs/CONTRIBUTING.md` for contribution guidelines.

For questions or issues, open an issue on GitHub.

---

**Remember: Always update this file at the end of your session!**
