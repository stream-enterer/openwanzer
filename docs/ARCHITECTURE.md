# Open Wanzer - Architecture Documentation

Last Updated: 2025-11-15

---

## Table of Contents

1. [Overview](#overview)
2. [Directory Structure](#directory-structure)
3. [Module Architecture](#module-architecture)
4. [Data Flow](#data-flow)
5. [Key Systems](#key-systems)
6. [Rendering Pipeline](#rendering-pipeline)
7. [Design Patterns](#design-patterns)
8. [Performance Considerations](#performance-considerations)

---

## Overview

Open Wanzer follows a modular architecture with logical separation of concerns. The codebase uses a **flat directory structure** with PascalCase file naming, but maintains logical module boundaries through namespaces and naming conventions.

### Core Principles

- **Separation of Concerns**: Clear boundaries between game logic, rendering, and input
- **Data-Oriented Design**: Game state is centralized in `GameState` structure
- **Immediate Mode UI**: Using raygui for simple, stateless UI rendering
- **No Deep Hierarchies**: Flat structure for easy navigation and compilation

### Technology Stack

- **C++17**: Modern C++ with standard library
- **raylib**: Cross-platform graphics and windowing
- **raygui**: Immediate mode GUI library
- **Custom Hex Library**: Axial coordinate system based on Red Blob Games

---

## Directory Structure

```
openwanzer/
├── src/                   # All .cpp source files
│   ├── Main.cpp          # Entry point and game loop
│   ├── [Module].cpp      # Module implementation files
│   └── ...
├── include/               # All .h header files
│   ├── [Module].h        # Module declarations
│   └── ...
├── lib/                   # Third-party libraries (raylib)
├── resources/             # Game assets
│   └── styles/           # UI themes
├── tests/                 # Unit tests (future)
├── docs/                  # Documentation
├── scripts/               # Build and utility scripts
└── examples/              # Example scenarios (future)
```

---

## Module Architecture

### Module Map

```
┌─────────────────────────────────────────────────────┐
│                     Main.cpp                        │
│                  (Game Loop)                        │
└────────┬────────────────────────────────────────────┘
         │
         ├─────────────┬─────────────┬─────────────┬──────────────┐
         │             │             │             │              │
    ┌────▼────┐   ┌───▼────┐   ┌───▼────┐   ┌────▼─────┐  ┌────▼────┐
    │  Core   │   │  Game  │   │Render- │   │  Input   │  │ Config  │
    │ Module  │   │ Logic  │   │  ing   │   │  Module  │  │ Module  │
    └─────────┘   └────────┘   └────────┘   └──────────┘  └─────────┘
         │             │             │             │              │
         └─────────────┴─────────────┴─────────────┴──────────────┘
                              │
                         GameState
```

### Core Module

**Purpose**: Fundamental data structures and types

**Files**:
- `GameState.h/cpp`: Main game state container
- `Unit.h/cpp`: Unit class and management
- `Enums.h`: Core enumerations
- `Types.h`: Type definitions
- `Constants.h/cpp`: Game constants
- `HexCoord.h`: Hex coordinate structure
- `GameHex.h`: Hex tile data
- `ArmorLocation.h/cpp`: Armor location types

**Responsibilities**:
- Define core data structures
- Manage game state
- Unit data and management
- Coordinate system definitions

**Dependencies**: None (foundation module)

### Game Logic Module

**Purpose**: Game mechanics and rules implementation

**Files**:
- `GameLogic.h`: Namespace and function declarations
- `Combat.cpp`: Combat resolution
- `Pathfinding.cpp`: Movement and pathfinding
- `Systems.cpp`: Turn management, fog of war
- `Utilities.cpp`: Helper functions
- `AttackLines.cpp`: Attack visualization
- `CombatArcs.h/cpp`: Attack arc calculation
- `DamageSystem.h/cpp`: Armor damage system
- `HitTables.h/cpp`: Hit location determination

**Responsibilities**:
- Implement game rules
- Handle combat calculations
- Manage turn progression
- Pathfinding and movement validation
- Fog of war / line of sight

**Dependencies**: Core Module

**Key Algorithms**:
- A* pathfinding
- Dijkstra's algorithm for movement range
- BattleTech hit location tables
- Line-of-sight raycasting

### Rendering Module

**Purpose**: All visual output and UI rendering

**Files**:
- `Rendering.h`: Namespace and function declarations
- `HexDrawing.cpp`: Hex grid rendering
- `UIDrawing.cpp`: UI panels and menus
- `CombatVisuals.cpp`: Combat effects
- `PaperdollUI.h/cpp`: Mech status display

**Responsibilities**:
- Render hex grid and units
- Draw UI elements
- Visualize combat and effects
- Camera transformations
- Theme management

**Dependencies**: Core Module, Game Logic Module (for state access)

**Rendering Order**:
1. Clear screen
2. Apply camera transform
3. Draw hex grid
4. Draw units
5. Draw highlights and overlays
6. Draw attack lines
7. Reset camera transform
8. Draw UI panels
9. Draw menus

### Input Module

**Purpose**: Handle user input and camera control

**Files**:
- `Input.h`: Input handling declarations
- `Camera.cpp`: Camera control implementation

**Responsibilities**:
- Process mouse and keyboard input
- Camera panning and zooming
- Convert screen to world coordinates
- Input state management

**Dependencies**: Core Module

### Config Module

**Purpose**: Configuration and persistence

**Files**:
- `Config.h`: Configuration namespace
- `Persistence.cpp`: Save/load config
- `StyleManager.cpp`: Theme management

**Responsibilities**:
- Load and save settings
- Manage UI themes
- Handle user preferences

**Dependencies**: Core Module

### UI Module

**Purpose**: Complex UI components

**Files**:
- `UIPanels.h/cpp`: Draggable panels (combat log, unit info, paperdoll)

**Responsibilities**:
- Draggable panel management
- Panel state persistence
- Panel interaction

**Dependencies**: Core Module, Rendering Module

---

## Data Flow

### Startup Sequence

```
1. main()
2. Config::discoverStyles()
3. Config::loadConfig()
4. InitWindow()
5. Config::loadStyle()
6. GameState initialization
7. Game loop start
```

### Game Loop

```
┌────────────────────────────────────────────────┐
│  while (!WindowShouldClose())                  │
│  ├─ Process Input (mouse, keyboard)           │
│  ├─ Update Game State                         │
│  │  ├─ Handle unit selection                  │
│  │  ├─ Process movement                       │
│  │  ├─ Resolve combat                         │
│  │  └─ Update systems (fog of war, etc.)      │
│  ├─ Render                                     │
│  │  ├─ Clear screen                            │
│  │  ├─ Draw hex grid                           │
│  │  ├─ Draw units                              │
│  │  ├─ Draw UI                                 │
│  │  └─ Draw menus                              │
│  └─ End frame                                  │
└────────────────────────────────────────────────┘
```

### Combat Flow

```
1. User selects attacker
2. User selects target
3. Calculate attack arc (CombatArcs::getAttackArc)
4. Get hit table (HitTables::getHitTable)
5. Roll for hit location
6. Apply damage (DamageSystem::applyDamage)
7. Update unit state
8. Update UI (paperdoll, combat log)
9. Check for unit destruction
```

### Movement Flow

```
1. User selects unit
2. Calculate movement range (Pathfinding::highlightMovementRange)
3. User selects destination
4. Find path (Pathfinding::findPath)
5. Phase 1: Move unit to destination
6. Phase 2: Select facing direction
7. Update fog of war
8. Update unit state
```

---

## Key Systems

### Hex Coordinate System

**Implementation**: Based on Red Blob Games axial coordinate system

**Coordinate Types**:
- **Axial** (HexCoord): Internal representation (q, r)
- **Offset** (OffsetCoord): Display representation (col, row)

**Key Functions**:
- `createHexLayout()`: Creates coordinate converter
- `hex_to_pixel()`: Axial → screen coordinates
- `pixel_to_hex()`: Screen coordinates → axial
- `gameCoordToOffset()`: Game coords → offset coords
- `offsetToGameCoord()`: Offset coords → game coords

**Distance**: Manhattan distance in axial space = hex distance

### Armor System

**Locations**: 8 locations per unit
- HEAD, CT, LT, RT, LA, RA, LL, RL

**Damage Application**:
1. Determine attack arc (front/side/rear)
2. Roll on appropriate hit table
3. Get hit location
4. Apply damage to that location
5. Check for destruction
6. Update paperdoll UI

**Critical Rules**:
- CT destruction = unit destroyed
- Leg destruction = reduced movement
- Arm destruction = reduced firepower
- Head destruction = unit destroyed

### Fog of War

**Implementation**: Per-hex visibility flags for each side

**Spotting Ranges**:
- Infantry: 2 hexes
- Recon: 4 hexes
- Regular units: 3 hexes

**Update Triggers**:
- Unit movement
- Unit creation/destruction
- Turn change

**Algorithm**:
- Flood fill from unit position up to spot range
- Mark hexes as visible for that side
- Update on unit state changes

---

## Rendering Pipeline

### Camera System

**Camera State**:
- `offsetX`, `offsetY`: World position
- `zoom`: Zoom level (0.5 - 2.0)
- Panning state

**Transformations**:
1. Screen coordinates → World coordinates
2. World coordinates → Hex coordinates
3. Hex coordinates → World coordinates
4. World coordinates → Screen coordinates

### UI Rendering (Immediate Mode)

**Pattern**: Render and handle input in same frame

**Example**:
```cpp
if (GuiButton(bounds, "Text")) {
    // Handle click
}
```

**Benefits**:
- No state management
- Simple to reason about
- Easy to modify

**Drawbacks**:
- Re-render every frame
- Can't easily cache rendering

### Layered Rendering

**Layers** (bottom to top):
1. Terrain hexes
2. Terrain highlights
3. Units
4. Unit highlights
5. Attack lines
6. Movement previews
7. UI panels
8. Menus
9. Tooltips

---

## Design Patterns

### Patterns Used

1. **Namespace Organization**: Logical grouping without class overhead
   - `GameLogic::`, `Rendering::`, `Config::`

2. **Data-Oriented Design**: GameState as central data structure
   - All systems operate on GameState
   - Clear data dependencies

3. **Immediate Mode UI**: Stateless UI rendering
   - Simple to implement
   - No complex state management

4. **RAII**: Resource management through constructors/destructors
   - `std::unique_ptr` for owned entities
   - Automatic cleanup

5. **Composition over Inheritance**: Minimal class hierarchies
   - Prefer structs with functions
   - Avoid deep inheritance trees

### Anti-Patterns Avoided

- **God Objects**: GameState is large but well-structured
- **Circular Dependencies**: Clear module hierarchy
- **Deep Nesting**: Flat directory structure
- **Global Mutable State**: State passed explicitly

---

## Performance Considerations

### Current Performance Characteristics

- **Rendering**: ~60 FPS on modest hardware
- **Pathfinding**: Fast enough for small maps (< 50x50)
- **Fog of War**: Recalculated each frame (could be optimized)

### Optimization Opportunities

1. **Spatial Partitioning**: For large maps, use grid partitioning
2. **Dirty Flags**: Only recalculate fog of war when units move
3. **Draw Call Batching**: Batch hex rendering
4. **Texture Atlasing**: Combine unit sprites

### Memory Usage

- **GameState**: ~few KB
- **Units**: ~few bytes each
- **Map**: ~100 bytes per hex
- **Total**: < 10 MB for typical game

### Scaling Limits

- **Map Size**: Tested up to 100x100 hexes
- **Unit Count**: Tested up to 100 units
- **Performance**: Primarily rendering-bound

---

## Future Architecture Changes

### Planned Improvements

1. **Entity Component System**: For more flexible unit composition
2. **Event System**: Decouple systems via events
3. **Serialization**: Save/load game state
4. **Networking**: Multiplayer support
5. **Scripting**: Lua integration for campaigns

### Migration Strategy

- Incremental refactoring
- Maintain backwards compatibility where possible
- Use feature flags for new systems
- Comprehensive testing before breaking changes

---

## See Also

- `CLAUDE.md`: AI assistant context and session guidelines
- `CONTRIBUTING.md`: Contribution guidelines
- `README.md`: User-facing documentation
