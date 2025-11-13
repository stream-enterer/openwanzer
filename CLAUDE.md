# Panzer General 2 Raylib Prototype

## Project Overview

This is a C++ prototype implementation of Panzer General 2, a classic hex-based turn-based strategy game. It uses **raylib** for graphics/input and **raygui** for the GUI. The prototype is based on an open-source JavaScript port and implements core gameplay mechanics with a modern options menu.

**Language**: C++17  
**Graphics**: Raylib 4.0+  
**GUI**: RayGUI (header-only, included with raylib)  
**Platform**: Cross-platform (Linux, macOS, Windows)

## Key Features

- Hex-based map with offset coordinate system
- Turn-based gameplay (Axis vs Allied)
- 6 unit classes: Infantry, Tank, Artillery, Recon, Anti-Tank, Air Defense
- Combat system with experience and entrenchment
- In-game options menu with live settings adjustment
- Adjustable resolution, FPS, hex size, camera pan speed

## Project Structure

```
.
├── openwanzer.cpp       # Main game file (single-file architecture)
├── build.sh                # Bash build script
├── Makefile                # Make build system
├── README.md               # User documentation
├── DESIGN.md               # Technical architecture details
├── COMPARISON.md           # JS vs C++ feature comparison
├── OPTIONS_GUIDE.md        # Options menu visual guide
├── BUGFIXES.md             # Recent bug fixes
├── QUICKSTART.md           # Quick setup guide
└── /include/*		    # Raylib and raygui libraries
```

## Architecture

### Single-File Design
The entire game is in `openwanzer.cpp` (~820 lines) for simplicity. Structure:

```cpp
// Includes & Pragma (lines 1-18)
// Constants & Enums (lines 20-95)
// Data Structures (lines 97-235)
//   - HexCoord, Hex, Unit, VideoSettings, GameState
// Hex Math & Rendering (lines 237-380)
// Game Logic (lines 382-520)
// UI & Options Menu (lines 522-695)
// Main Loop (lines 697-824)
```

### Key Data Structures

**GameState**: Main game container
- `map`: 2D vector of Hex objects
- `units`: Vector of Unit pointers
- `selectedUnit`: Currently selected unit
- `settings`: VideoSettings struct
- `showOptionsMenu`: Menu visibility flag

**Hex**: Single hexagonal tile
- Terrain type, owner, victory hex flag
- Selection states (isMoveSel, isAttackSel)
- Spotting per side

**Unit**: Military unit
- Position, class, side, strength
- Combat stats (attack, defense, initiative)
- Status (hasMoved, hasFired, fuel, ammo)
- Experience and entrenchment levels

**VideoSettings**: Configurable options
- Resolution, fullscreen, vsync, FPS
- Hex size, camera pan speed, MSAA
- Dropdown edit states (for menu functionality)

## Building

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt install libraylib-dev

# Arch Linux
sudo pacman -S raylib

# macOS
brew install raylib
```

### Build Commands
```fish
# Fish shell
./build.fish

# Bash
./build.sh

# Make
make

# Manual
g++ pg2_prototype.cpp -o pg2_prototype \
    $(pkg-config --cflags --libs raylib) \
    -std=c++17 -O2 -Wall
```

## Common Tasks

### Adding a New Unit Type
1. Add to `UnitClass` enum (line ~47)
2. Update `getUnitSymbol()` (line ~301)
3. Add initialization in `GameState::addUnit()` (line ~214)

### Adding a New Terrain Type
1. Add to `TerrainType` enum (line ~34)
2. Update `getTerrainColor()` (line ~288)
3. Update random generation in `initializeMap()` (line ~176)

### Adding a New Video Setting
1. Add field to `VideoSettings` struct (line ~61)
2. Add GUI control in `drawOptionsMenu()` (line ~565)
3. Add application logic in Apply button (line ~649)
4. Initialize in `VideoSettings()` constructor (line ~73)

### Modifying Combat
- Combat calculation: `performAttack()` (line ~465)
- Attack range: `highlightAttackRange()` (line ~424)
- Movement range: `highlightMovementRange()` (line ~409)

## Critical Implementation Details

### RayGUI Parameter Pattern
RayGUI uses **immediate-mode GUI** with pointers:
```cpp
// CORRECT - Pass by pointer
GuiCheckBox(bounds, label, &game.settings.vsync);
GuiSlider(bounds, min, max, &game.settings.hexSize, 20, 80);

// WRONG - Don't do this
value = GuiCheckBox(bounds, label, value);  // Compile error
```

### Dropdown State Management
Dropdowns need explicit edit state:
```cpp
// VideoSettings must have:
bool resolutionDropdownEdit;
bool fpsDropdownEdit;

// In drawOptionsMenu():
if (GuiDropdownBox(bounds, labels, &index, game.settings.resolutionDropdownEdit)) {
    game.settings.resolutionDropdownEdit = !game.settings.resolutionDropdownEdit;
}
```

### Hex Coordinate System
Uses **offset coordinates** (not axial or cube):
- Odd columns are shifted down by half a hex
- Conversion functions: `hexToPixel()` (line ~237), `pixelToHex()` (line ~243)
- Distance calculation: `hexDistance()` uses cube coordinate conversion (line ~397)

### Warning Suppression
RayGUI has unavoidable warnings from library code:
```cpp
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#include "raygui.h"
#pragma GCC diagnostic pop
```

## Coding Conventions

- **Constants**: `UPPER_SNAKE_CASE` (SCREEN_WIDTH, HEX_SIZE)
- **Types**: `PascalCase` (GameState, Unit, VideoSettings)
- **Functions**: `camelCase` (hexToPixel, drawMap, performAttack)
- **Variables**: `camelCase` (selectedUnit, cameraOffsetX)
- **Enums**: `enum class` for type safety

## Dependencies

- **raylib 4.0+**: Graphics, input, window management
- **raygui 3.0+**: GUI controls (included with raylib)
- **C++17**: For modern features (structured bindings, std::optional if added)
- **pkg-config**: For build system integration

## Known Issues & Gotchas

### Issue: ESC Key Behavior
**Solution**: Use `SetExitKey(KEY_NULL)` to disable ESC from closing window.
```cpp
// In main():
SetExitKey(KEY_NULL);  // Required for menu ESC handling
```

### Issue: Resolution Changes
**Note**: Window resizing updates global variables:
```cpp
SCREEN_WIDTH = res.width;   // Must update globals
SCREEN_HEIGHT = res.height; // for rendering to work
```

### Issue: Hex Size Changes
**Note**: Must recalculate dependent values:
```cpp
HEX_SIZE = game.settings.hexSize;
HEX_WIDTH = HEX_SIZE * 2.0f;        // Must update
HEX_HEIGHT = sqrtf(3.0f) * HEX_SIZE; // Must update
```

### Issue: MSAA Requires Restart
**Reason**: MSAA flag must be set before `InitWindow()`:
```cpp
SetConfigFlags(FLAG_MSAA_4X_HINT);  // Before InitWindow()
InitWindow(width, height, title);
```

## Debug Tips

### FPS Drops
- Check map rendering loop (line ~330)
- Watch for excessive unit count (>50 units)
- Test with smaller hex sizes

### Menu Not Appearing
- Verify `game.showOptionsMenu = true`
- Check that menu drawing happens after main game (line ~811)
- Ensure overlay isn't blocking (z-order issue)

### Dropdowns Not Working
- Verify edit state variables exist in VideoSettings
- Check toggle logic: `state = !state` on click
- Ensure dropdowns close when buttons clicked

### Units Not Selecting
- Check mouse position conversion: `pixelToHex()`
- Verify unit is at clicked hex: `getUnitAt()`
- Ensure unit belongs to current player

## Testing Scenarios

### Basic Gameplay Test
1. Select axis unit (red)
2. Move to green hex
3. Attack allied unit (blue) in red hex
4. End turn (SPACE)
5. Allied player moves
6. End turn
7. Repeat

### Options Menu Test
1. Open menu (ESC or OPTIONS button)
2. Click resolution dropdown - should expand
3. Select different resolution - should change immediately on Apply
4. Adjust hex size slider - drag should work smoothly
5. Toggle checkboxes - should toggle on/off
6. Click Apply - menu should close, settings active
7. Reopen menu - settings should persist

### ESC Key Test
1. Press ESC from game - menu opens
2. Click resolution dropdown - expands
3. Press ESC - dropdown closes (menu stays open)
4. Press ESC again - menu closes
5. Game continues (doesn't exit)

## Performance Targets

- **60 FPS** at 1920x1080 with 20 units
- **< 10ms** map rendering time
- **< 1ms** UI rendering time
- **< 50MB** memory usage

## Future Enhancements

Priority features to add:
1. **Save/Load System** - Serialize GameState to JSON
2. **AI Opponent** - Basic minimax or rules-based AI
3. **Zone of Control** - Restrict movement near enemies
4. **Transport System** - Units carrying other units
5. **Sound Effects** - Combat and UI sounds
6. **Unit Sprites** - Replace rectangles with proper graphics
7. **Campaign Mode** - Multi-scenario progression
8. **Network Multiplayer** - Client-server architecture

## References

- **Original JavaScript Port**: /mnt/project/*.js files
- **Raylib Docs**: https://www.raylib.com/cheatsheet/cheatsheet.html
- **RayGUI Docs**: https://github.com/raysan5/raygui
- **Hex Grid Guide**: https://www.redblobgames.com/grids/hexagons/
- **PG2 Rules**: See DESIGN.md for combat formulas

## Quick Reference

### Key Functions
- `hexToPixel()` - Convert hex coords to screen position
- `pixelToHex()` - Convert screen position to hex coords
- `hexDistance()` - Calculate distance between hexes
- `moveUnit()` - Move unit to new position
- `performAttack()` - Execute combat between units
- `endTurn()` - Switch players and reset unit states
- `drawOptionsMenu()` - Render and handle settings menu

### Key Variables
- `HEX_SIZE` - Radius of hexagon (adjustable)
- `MAP_ROWS`, `MAP_COLS` - Map dimensions
- `SCREEN_WIDTH`, `SCREEN_HEIGHT` - Window size (mutable)
- `game.selectedUnit` - Currently selected unit pointer
- `game.showOptionsMenu` - Menu visibility flag

### Controls
- **Left Click**: Select/Move/Attack
- **Arrow Keys**: Pan camera
- **SPACE**: End turn
- **ESC**: Toggle options menu
- **Close Window**: Exit game

---

**Last Updated**: November 2024  
**Version**: 2.0 (With RayGUI Options Menu)  
**License**: Educational Reference (Original PG2 IP rights retained by owners)
