# OpenWanzer Coding Conventions

This document defines the coding standards and architectural conventions for the OpenWanzer project. **ALL CODE MUST FOLLOW THESE CONVENTIONS.**

## Architecture Principles

### Single-File Architecture with Namespaces
- The entire game remains in `openwanzer.cpp` (~2100 lines)
- Code is organized using C++ namespaces for separation of concerns
- **DO NOT** split into multiple files
- **DO NOT** over-engineer with unnecessary design patterns

### Namespace Organization

The codebase is divided into four primary namespaces:

#### **Rendering** - Visual Presentation
- Hex coordinate conversion (`gameCoordToOffset`, `offsetToGameCoord`)
- Hex layout creation (`createHexLayout`)
- Color mapping (`getTerrainColor`, `getUnitColor`, `getUnitSymbol`)
- Drawing functions (`drawHexagon`, `drawMap`, `drawUI`, `drawOptionsMenu`)
- Selection highlighting (`clearSelectionHighlights`)

#### **GameLogic** - Game Rules and Mechanics
- Utility functions (`getTerrainIndex`, `getTerrainEntrenchment`)
- Hex math (`hexDistance`, `getAdjacent`, `getCellsInRange`)
- Unit type checks (`isAir`, `isHardTarget`, `isSea`)
- Zone of Control (`setUnitZOC`, `initializeAllZOC`)
- Fog of War (`setUnitSpotRange`, `initializeAllSpotting`)
- Movement (`highlightMovementRange`, `moveUnit`)
- Combat (`highlightAttackRange`, `calculateKills`, `performAttack`)
- Entrenchment and turns (`entrenchUnit`, `endTurn`)

#### **Input** - User Interaction
- Camera controls (`calculateCenteredCameraOffset`)
- Zoom handling (`handleZoom`)
- Pan handling (`handlePan`)
- Mouse and keyboard input processing

#### **Config** - Settings Management
- Style discovery (`discoverStyles`, `getStyleIndex`)
- Configuration persistence (`saveConfig`, `loadConfig`)
- Style theme loading (`loadStyleTheme`)
- GUI scaling (`applyGuiScale`)
- Global style variables (`AVAILABLE_STYLES`, `STYLE_LABELS_STRING`)

### File Structure

```cpp
//==============================================================================
// SECTION 1: INCLUDES AND PREPROCESSOR
//==============================================================================

//==============================================================================
// SECTION 2: CONSTANTS AND GLOBALS
//==============================================================================

//==============================================================================
// SECTION 3: ENUMS AND DATA TABLES
//==============================================================================

//==============================================================================
// SECTION 4: UTILITY FUNCTIONS (GAME LOGIC)
//==============================================================================
namespace GameLogic {
  // Utility functions
}

//==============================================================================
// SECTION 5: DATA STRUCTURES
//==============================================================================
// Global scope - no namespace

//==============================================================================
// SECTION 5A: CONFIGURATION - STYLE DISCOVERY
//==============================================================================
namespace Config {
  // Style discovery functions and globals
}

//==============================================================================
// SECTION 6: RENDERING FUNCTIONS
//==============================================================================
namespace Rendering {
  // Core rendering functions
}

//==============================================================================
// SECTION 6B: RENDERING FUNCTIONS - UI AND MENUS
//==============================================================================
namespace Rendering {
  // UI and menu rendering
}

//==============================================================================
// SECTION 7: GAME LOGIC FUNCTIONS
//==============================================================================
namespace GameLogic {
  // Game mechanics and rules
}

//==============================================================================
// SECTION 8: INPUT AND CAMERA FUNCTIONS
//==============================================================================
namespace Input {
  // Camera control
}

//==============================================================================
// SECTION 8B: INPUT FUNCTIONS - ZOOM AND PAN HANDLERS
//==============================================================================
namespace Input {
  // Zoom and pan handlers
}

//==============================================================================
// SECTION 9: CONFIGURATION - SAVE/LOAD AND SETTINGS
//==============================================================================
namespace Config {
  // Config save/load functions
}

//==============================================================================
// SECTION 10: MAIN LOOP
//==============================================================================
// Global scope - no namespace
```

## Coding Standards

### Naming Conventions

**MANDATORY - DO NOT DEVIATE:**

| Element | Convention | Example |
|---------|------------|---------|
| Constants | `UPPER_SNAKE_CASE` | `SCREEN_WIDTH`, `HEX_SIZE`, `MAX_UNITS` |
| Types/Classes | `PascalCase` | `GameState`, `Unit`, `VideoSettings`, `HexCoord` |
| Functions | `camelCase` | `hexToPixel`, `drawMap`, `performAttack`, `endTurn` |
| Variables | `camelCase` | `selectedUnit`, `cameraOffsetX`, `currentPlayer` |
| Enums | `enum class` | `enum class TerrainType`, `enum class UnitClass` |
| Namespaces | `PascalCase` | `Rendering`, `GameLogic`, `Input`, `Config` |

### Const Correctness

**ENFORCE STRICTLY:**

```cpp
// ✅ CORRECT - Read-only pointer parameters
bool isAir(const Unit *unit);
bool isHardTarget(const Unit *unit);
int calculateKills(int atkVal, int defVal, const Unit *attacker, const Unit *defender);

// ✅ CORRECT - Read-only reference parameters
int hexDistance(const HexCoord &a, const HexCoord &b);
OffsetCoord gameCoordToOffset(const HexCoord &coord);

// ❌ WRONG - Should be const if read-only
bool isAir(Unit *unit);  // DO NOT DO THIS
```

### Smart Pointers

**USE CONSISTENTLY:**

```cpp
// ✅ CORRECT - Ownership with unique_ptr
std::vector<std::unique_ptr<Unit>> units;
auto unit = std::make_unique<Unit>();
units.push_back(std::move(unit));

// ✅ CORRECT - Observation with raw pointer
Unit *selectedUnit;  // Observer, does not own
Unit *clickedUnit = game.getUnitAt(coord);  // Temporary observer

// ❌ WRONG - Don't use shared_ptr unless shared ownership is needed
std::shared_ptr<Unit> unit;  // Overkill for this use case
```

### Namespace Usage

**RULES:**

1. **Always use namespace prefix when calling across namespaces:**
   ```cpp
   // ✅ CORRECT
   GameLogic::endTurn(game);
   Rendering::drawMap(game);
   Input::handleZoom(game);
   Config::saveConfig(settings);
   ```

2. **Functions within the same namespace can call each other directly:**
   ```cpp
   // Inside GameLogic namespace
   void highlightMovementRange(GameState &game, Unit *unit) {
     // Can call clearSelectionHighlights from Rendering namespace
     Rendering::clearSelectionHighlights(game);

     // Can call isAir directly (same namespace)
     bool ignoreZOC = isAir(unit);
   }
   ```

3. **Data structures (structs) remain in global scope:**
   ```cpp
   // ✅ CORRECT - Global scope
   struct Unit { ... };
   struct GameState { ... };
   struct HexCoord { ... };

   // ❌ WRONG - Don't put data structures in namespaces
   namespace GameLogic {
     struct Unit { ... };  // DO NOT DO THIS
   }
   ```

### Forward Declarations

**USE WHEN NECESSARY:**

```cpp
// ✅ CORRECT - Forward declare Config functions before using in Rendering
namespace Config {
  void saveConfig(const VideoSettings& settings);
  void loadStyleTheme(const std::string& themeName);
  extern std::vector<std::string> AVAILABLE_STYLES;
}

namespace Rendering {
  void drawOptionsMenu(GameState &game, bool &needsRestart) {
    Config::saveConfig(game.settings);  // Can now use Config functions
  }
}
```

### RayGUI Patterns

**CRITICAL - FOLLOW EXACTLY:**

```cpp
// ✅ CORRECT - Pass by pointer
GuiCheckBox(bounds, label, &game.settings.vsync);
GuiSlider(bounds, min, max, &game.settings.hexSize, 20, 80);

// ❌ WRONG - Don't do this
value = GuiCheckBox(bounds, label, value);  // Compile error

// ✅ CORRECT - Dropdown state management
struct VideoSettings {
  bool resolutionDropdownEdit;  // Must have edit state
  bool fpsDropdownEdit;
};

if (GuiDropdownBox(bounds, labels, &index, game.settings.resolutionDropdownEdit)) {
  game.settings.resolutionDropdownEdit = !game.settings.resolutionDropdownEdit;
}
```

## What to Avoid

### ❌ DO NOT Over-Engineer

**NEVER ADD:**
- Component-based entity systems (overkill for prototype)
- Event queue systems (not needed yet)
- Multiple files (keep single-file architecture)
- Abstract factory patterns (unnecessary complexity)
- Template metaprogramming (keep it simple)

### ❌ DO NOT Break Const Correctness

```cpp
// ❌ WRONG - Modifying through const pointer
void badFunction(const Unit *unit) {
  unit->strength = 10;  // Compile error - good!
}

// ✅ CORRECT - Use non-const if you need to modify
void goodFunction(Unit *unit) {
  unit->strength = 10;  // OK
}
```

### ❌ DO NOT Use Raw `new` or `delete`

```cpp
// ❌ WRONG - Manual memory management
Unit *unit = new Unit();
delete unit;

// ✅ CORRECT - Use smart pointers
auto unit = std::make_unique<Unit>();
// Automatically deleted
```

## Cross-Namespace Dependencies

### Allowed Dependencies

```
Rendering → Config (for config functions in options menu)
Rendering → Input (for camera centering in UI)
GameLogic → Rendering (for clearing selection highlights)
Input → Rendering (for hex layout calculations)
Main → All (main function can call any namespace)
```

### Namespace Interaction Example

```cpp
// In main()
int main() {
  Config::discoverStyles();
  Config::loadConfig(settings);
  Input::calculateCenteredCameraOffset(camera, width, height);
  GameLogic::initializeAllZOC(game);

  while (!WindowShouldClose()) {
    Input::handleZoom(game);
    GameLogic::endTurn(game);
    Rendering::drawMap(game);
  }
}

// In GameLogic namespace
void endTurn(GameState &game) {
  // Can call across namespaces with prefix
  Rendering::clearSelectionHighlights(game);
}
```

## Testing and Validation

### Before Committing

**MANDATORY CHECKS:**

1. **Compile successfully:**
   ```bash
   make clean && make
   ```

2. **Test basic functionality:**
   - Select and move units
   - Attack enemy units
   - End turn
   - Open options menu
   - Change settings

3. **Verify namespace organization:**
   - All rendering in `Rendering::`
   - All game logic in `GameLogic::`
   - All input in `Input::`
   - All config in `Config::`

## Summary

**THE GOLDEN RULES:**

1. ✅ Use namespaces: `Rendering`, `GameLogic`, `Input`, `Config`
2. ✅ Follow naming: `UPPER_SNAKE_CASE` constants, `PascalCase` types, `camelCase` functions
3. ✅ Use const correctness: `const Unit*` for read-only, `const&` for references
4. ✅ Use smart pointers: `std::unique_ptr` for ownership
5. ✅ Keep single-file architecture
6. ❌ Don't over-engineer
7. ❌ Don't split files
8. ❌ Don't use raw `new`/`delete`

**FOLLOW THESE CONVENTIONS IN ALL CODE. NO EXCEPTIONS.**
