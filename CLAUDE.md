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
- **Build System**: GNU Make

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

- **Files**: PascalCase (e.g., `GameState.cpp`, `Unit.h`)
- **Classes/Structs**: PascalCase (e.g., `GameState`, `Unit`)
- **Functions**: snake_case (e.g., `calculate_damage`, `get_unit_at`)
- **Variables**: snake_case (e.g., `unit_count`, `max_health`)
- **Header Guards**: `OPENWANZER_FILENAME_H` (e.g., `OPENWANZER_GAMESTATE_H`)

### Module Organization

The codebase is logically organized into modules (though physically flat):

#### Core Module
- **GameState.h/cpp**: Main game state and structures
- **Unit.h/cpp**: Unit class and unit data
- **Enums.h**: Game enumerations (TerrainType, UnitClass, MovMethod, etc.)
- **Types.h**: Type aliases and basic types
- **Constants.h/cpp**: Game constants and configuration
- **HexCoord.h**: Hex coordinate structure
- **GameHex.h**: Hex tile data structure
- **ArmorLocation.h/cpp**: Armor location enumeration and utilities

#### Game Logic Module
- **GameLogic.h**: Main game logic namespace and function declarations
- **Combat.cpp**: Combat resolution and damage calculation
- **Pathfinding.cpp**: A* pathfinding and movement range calculation
- **Systems.cpp**: Fog of war, spotting, turn management
- **Utilities.cpp**: Helper functions (terrain, facing, hex math)
- **AttackLines.cpp**: Attack line visualization
- **CombatArcs.h/cpp**: Attack arc determination (front/side/rear)
- **DamageSystem.h/cpp**: Armor damage system
- **HitTables.h/cpp**: Hit location tables

#### Rendering Module
- **Rendering.h**: Main rendering namespace and function declarations
- **HexDrawing.cpp**: Hex rendering and coordinate conversion
- **UIDrawing.cpp**: UI rendering (status bar, panels, menus)
- **CombatVisuals.cpp**: Combat visualization (attack lines, highlights)
- **PaperdollUI.h/cpp**: BattleTech-style paperdoll mech display

#### Input Module
- **Input.h**: Input handling declarations
- **Camera.cpp**: Camera controls and viewport management

#### Config Module
- **Config.h**: Configuration management namespace
- **Persistence.cpp**: Config file loading/saving
- **StyleManager.cpp**: UI theme management

#### UI Module
- **UIPanels.h/cpp**: Draggable UI panels (combat log, unit info, paperdoll)

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

### Makefile Targets

```bash
make              # Build release version
make debug        # Build with debug symbols
make clean        # Remove build artifacts
make run          # Build and run
make format       # Format code with clang-format
make install      # Install to /usr/local/bin
make uninstall    # Uninstall
make help         # Show all targets
```

### Dependencies
- raylib 5.5.0 (included in `lib/` and `include/`)
- X11 libraries (Linux only)
- C++17 compiler

---

## Common Tasks

### Adding a New Source File

1. Create `NewFile.cpp` in `src/` and `NewFile.h` in `include/`
2. Use header guard: `#ifndef OPENWANZER_NEWFILE_H`
3. Include header in relevant files: `#include "NewFile.h"`
4. No need to update Makefile (uses automatic detection)

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
- `GameLogic::` - All game logic functions
- `Rendering::` - All rendering functions
- `Config::` - Configuration functions
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
All files moved from `src/module/snake_case.ext` to `src/PascalCase.ext` or `include/PascalCase.h`

Example mappings:
- `src/core/game_state.h` → `include/GameState.h`
- `src/core/game_state.cpp` → `src/GameState.cpp`
- `src/game_logic/combat.cpp` → `src/Combat.cpp`
- `src/rendering/rendering.h` → `include/Rendering.h`

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
