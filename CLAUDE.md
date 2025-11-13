# Panzer General 2 Python Prototype - Claude Context

## Project Overview

A turn-based tactical strategy game prototype based on Panzer General 2, built with Python Arcade 3.x. Features hex-based combat, unit management, and AI opponents.

**Current Version**: 0.2.0  
**Engine**: Python Arcade 3.x  
**Python**: 3.8+  
**Architecture**: Event-driven game loop with component-based design

## Quick Start

```bash
# Run the game
uv run main.py

# Or with pip
pip install arcade
python main.py
```

## File Structure & Responsibilities

```
panzer_proto/
├── main.py           - Game loop, rendering, GUI layout (500 lines)
├── constants.py      - Game data, enums, unit stats (250 lines)
├── hex_map.py        - Hex grid system, pathfinding (280 lines)
├── unit.py           - Unit class, combat mechanics (200 lines)
├── game_state.py     - Game state, turn management (250 lines)
├── ai.py             - AI decision making (150 lines)
└── pyproject.toml    - Dependencies and metadata
```

### main.py - Game Window & Rendering
**Purpose**: Main game loop, all rendering, GUI layout, input handling

**Key Components**:
- `PanzerGame(arcade.Window)` - Main game window class
- Text batching with Pyglet for performance (15+ text objects)
- 2-panel GUI: game panel (850px) + HUD panel (350px)
- Event handlers: `on_draw()`, `on_update()`, `on_mouse_press()`, `on_key_press()`

**Important Methods**:
- `setup()` - Initialize game state and GUI
- `setup_gui()` - Configure 2-panel layout with arcade.gui
- `create_text_objects()` - Pre-create all Text objects for batch rendering
- `update_text_objects()` - Update text content without recreation
- `draw_map()` - Render hex grid and terrain
- `draw_units()` - Render all units with health bars
- `select_unit()` / `deselect_unit()` - Handle unit selection

**Performance Critical**:
- Uses `self.text_batch.draw()` for efficient text rendering
- All text objects pre-created, only content updated
- Panel boundaries checked to avoid unnecessary draws

### constants.py - Game Configuration
**Purpose**: All game constants, enums, and static data

**Key Sections**:
- Screen/hex size constants
- Enums: `UnitClass`, `UnitType`, `TerrainType`, `Side`, `PlayerType`, `ActionType`
- `UNIT_DATA` dict: Complete unit statistics for all 8 unit types
- `TERRAIN_COLORS`, `MOVEMENT_COSTS`, `TERRAIN_DEFENSE` dicts
- Color definitions for rendering

**Adding New Units**:
```python
UNIT_DATA["New_Unit_Name"] = {
    "name": "Display Name",
    "unit_class": UnitClass.INFANTRY,
    "unit_type": UnitType.SOFT,
    "attack": 8,
    "defense": 6,
    "move_points": 5,
    "fuel": 50,
    "ammo": 12,
    "cost": 75
}
```

### hex_map.py - Hex Grid System
**Purpose**: Hex coordinate system, map generation, pathfinding

**Key Classes**:
- `Hex` - Single hex tile with terrain, unit, position
- `HexMap` - Complete map with generation and pathfinding

**Coordinate System**: Axial coordinates with offset columns
- Even columns: neighbors are straightforward
- Odd columns: neighbors have row offset

**Key Algorithms**:
- `get_reachable_hexes()` - Dijkstra for movement range
- `find_path()` - A* pathfinding with movement cost
- `get_neighbors()` - 6-direction hex neighbor calculation

**Important**: Uses `HEX_SIZE = 40` for rendering calculations

### unit.py - Unit Logic
**Purpose**: Unit class with combat, movement, state management

**Key Attributes**:
- Stats: `strength` (HP), `fuel`, `ammo`, `experience`
- State: `has_moved`, `has_fired`, `move_left`
- Position: `hex` (reference to Hex object)

**Key Methods**:
- `attack(defender)` - Combat resolution, returns (attacker_losses, defender_losses)
- `move_to(hex, cost)` - Move to hex, consume fuel/movement
- `get_attack()` / `get_defense()` - Calculate effective combat values with modifiers
- `end_turn()` - Reset turn state

**Combat Formula**:
```python
defender_losses = max(1, (attacker_power - defender_power // 2) // 3)
attacker_losses = max(0, (defender_power - attacker_power // 2) // 4)
```

### game_state.py - Game Management
**Purpose**: Overall game state, turn management, victory conditions

**Key Classes**:
- `Player` - Represents a player/side with units and resources
- `GameState` - Manages game flow, turn switching, win conditions

**Game Flow**:
1. Player's turn starts
2. Player moves/attacks with units
3. Player ends turn
4. Switch to next player
5. Check victory conditions
6. Repeat

**Key Methods**:
- `can_unit_move_to()` / `can_unit_attack()` - Validate actions
- `move_unit()` / `attack_unit()` - Execute actions
- `end_turn()` - Switch players, increment turn counter
- `check_game_over()` - Check victory conditions

### ai.py - AI Opponent
**Purpose**: Simple but functional AI for computer players

**AI Strategy**:
1. Try to attack any enemy in range (prioritize weak targets)
2. Move towards objectives or nearest enemies
3. Artillery uses range-2 attacks
4. End turn when no valid actions

**Key Methods**:
- `get_action()` - Returns (ActionType, unit, target_hex)
- `find_attack_target()` - Find best enemy to attack
- `find_move_target()` - Find best hex to move to

**AI Delay**: 0.3-0.5 seconds between actions for visibility

## Critical Design Patterns

### 1. Text Rendering Performance Pattern
**Problem**: `arcade.draw_text()` is extremely slow (5-10ms/frame)

**Solution**: Pre-create Text objects with Batch rendering
```python
# Setup phase
self.text_batch = Batch()
self.turn_text = arcade.Text("Turn: 1", x, y, color, 16, batch=self.text_batch)

# Update phase (cheap)
self.turn_text.text = f"Turn: {turn_number}"

# Render phase (very fast - single draw call)
self.text_batch.draw()
```

**Rule**: NEVER use `arcade.draw_text()` in loops or for frequently updated text

### 2. Coordinate System Pattern
**Arcade 3.x uses LRBT** (Left-Right-Bottom-Top) for rectangles:
```python
# CORRECT
arcade.draw_lrbt_rectangle_filled(left, right, bottom, top, color)
# where bottom < top

# WRONG (will throw ValueError)
arcade.draw_lrbt_rectangle_filled(left, right, top, bottom, color)
```

### 3. Unit Selection Pattern
```python
# Select unit
self.selected_unit = unit
self.reachable_hexes = calculate_movement_range()
self.attackable_hexes = calculate_attack_range()
self.update_text_objects()  # Update HUD

# Deselect unit
self.selected_unit = None
self.reachable_hexes = {}
self.attackable_hexes = []
self.update_text_objects()  # Clear HUD
```

### 4. GUI Panel Boundary Pattern
```python
# Game panel is 0 to GAME_PANEL_WIDTH (850)
# HUD panel is GAME_PANEL_WIDTH to SCREEN_WIDTH (1200)

# Mouse clicks should respect boundaries
if x > GAME_PANEL_WIDTH:
    return  # Click was in HUD, ignore for game actions

# Drawing should respect boundaries
if hex_x > GAME_PANEL_WIDTH:
    continue  # Don't draw hexes outside game panel
```

## Common Development Tasks

### Adding a New Unit Type

1. **Add to constants.py**:
```python
class UnitClass(IntEnum):
    NEW_UNIT = 13  # Add new enum

UNIT_DATA["German_NewUnit"] = {
    "name": "New Unit",
    "unit_class": UnitClass.NEW_UNIT,
    "unit_type": UnitType.HARD,
    "attack": 10,
    "defense": 8,
    "move_points": 5,
    "fuel": 50,
    "ammo": 10,
    "cost": 100
}
```

2. **Add symbol in main.py**:
```python
def get_unit_symbol(self, unit):
    symbols = {
        UnitClass.NEW_UNIT: "N",  # Add here
        # ... existing symbols
    }
```

3. **Update game_state.py** if needed for initial placement

### Adding a New Terrain Type

1. **Add to constants.py**:
```python
class TerrainType(IntEnum):
    NEW_TERRAIN = 6

TERRAIN_NAMES[TerrainType.NEW_TERRAIN] = "New Terrain"
TERRAIN_COLORS[TerrainType.NEW_TERRAIN] = (R, G, B)
MOVEMENT_COSTS[TerrainType.NEW_TERRAIN] = 2
TERRAIN_DEFENSE[TerrainType.NEW_TERRAIN] = 15
```

2. **Update hex_map.py** generation if needed

### Adding New HUD Information

1. **Create Text object in main.py**:
```python
def create_text_objects(self):
    # ... existing text objects
    self.new_info_text = arcade.Text(
        "New Info: 0",
        hud_x, hud_y - 80,  # Position
        arcade.color.WHITE,
        12,
        batch=self.text_batch  # Important!
    )
```

2. **Update in update_text_objects()**:
```python
def update_text_objects(self):
    # ... existing updates
    self.new_info_text.text = f"New Info: {self.game_state.new_value}"
```

### Improving AI Behavior

Edit `ai.py`:
- `find_attack_target()` - Change target prioritization
- `find_move_target()` - Change movement strategy
- Add new methods for more complex behaviors

### Adding Sound Effects

1. **Load sounds in __init__**:
```python
self.attack_sound = arcade.load_sound("sounds/attack.wav")
```

2. **Play in appropriate methods**:
```python
def attack_unit(self, attacker, target_hex):
    # ... combat logic
    arcade.play_sound(self.attack_sound)
```

## Arcade 3.x Specific Notes

### Drawing Functions
- ✅ Use: `arcade.draw_lrbt_rectangle_filled/outline()`
- ❌ Avoid: `arcade.draw_rectangle_filled/outline()` (deprecated)
- ✅ Use: `arcade.Text` objects with Batch
- ❌ Avoid: `arcade.draw_text()` (extremely slow)

### GUI System
- Use `arcade.gui.UIManager()` for layouts
- `UIBoxLayout` for horizontal/vertical arrangements
- `UIAnchorLayout` for positioning
- Always call `ui_manager.enable()` and `ui_manager.draw()`

### Text Rendering
```python
# Create once
text = arcade.Text("Hello", x, y, color, size, batch=batch)

# Update many times (cheap)
text.text = "New content"
text.color = new_color
text.x = new_x

# Draw (batch draws all at once)
batch.draw()
```

## Performance Considerations

### Current Performance
- Target: 60 FPS
- Actual: Stable 60 FPS
- Text rendering: <1ms/frame
- Map rendering: ~2-3ms/frame
- Unit rendering: ~1ms/frame

### Performance Rules
1. **Text**: Always use Text objects with Batch, never draw_text()
2. **Rendering**: Minimize draw calls, batch when possible
3. **Updates**: Only update what changed
4. **Pathfinding**: Cache results when possible
5. **AI**: Limit AI thinking time per frame

### Known Performance Bottlenecks
- None currently! All optimizations applied.

## Testing Guidelines

### Manual Testing Checklist
- [ ] Unit selection and deselection
- [ ] Movement range highlighting
- [ ] Attack range highlighting (including artillery range-2)
- [ ] Combat resolution
- [ ] Turn switching
- [ ] AI takes actions
- [ ] Victory conditions (elimination and turn limit)
- [ ] Game over screen
- [ ] Restart functionality
- [ ] HUD updates correctly
- [ ] Mouse only works in game panel
- [ ] All text renders without warnings

### Common Issues to Check
- Ensure bottom < top in all lrbt rectangle calls
- Verify text objects are added to batch
- Check that update_text_objects() is called after state changes
- Verify panel boundaries are respected

## Debugging Tips

### Enable Debug Output
Add to relevant methods:
```python
print(f"Unit {unit.id} attacking {target.id}")
print(f"Combat: Attacker {attacker_losses}, Defender {defender_losses}")
```

### Visualize Hex Coordinates
In `draw_map()`:
```python
arcade.draw_text(f"{hex_tile.row},{hex_tile.col}", 
                 x, y, arcade.color.WHITE, 8,
                 anchor_x="center", anchor_y="center")
```

### Monitor Performance
```python
import time
start = time.time()
# ... code to measure
print(f"Took {(time.time() - start) * 1000:.2f}ms")
```

## Code Style Guidelines

### Naming Conventions
- Classes: `PascalCase` (e.g., `GameState`, `HexMap`)
- Functions/methods: `snake_case` (e.g., `get_hex_at_position()`)
- Constants: `UPPER_SNAKE_CASE` (e.g., `GAME_PANEL_WIDTH`)
- Private methods: `_leading_underscore()` (not currently used much)

### Documentation
- Docstrings for all classes and complex methods
- Inline comments for non-obvious logic
- Type hints encouraged but not required

### File Organization
- Imports at top
- Constants after imports
- Classes after constants
- Helper functions at bottom
- `if __name__ == "__main__"` at very end

## Known Issues / TODOs

### Current Limitations
- No save/load system
- No campaign mode
- Simple AI (no strategic planning)
- No sound effects
- Basic graphics (no sprites)
- No multiplayer
- No fog of war

### Potential Improvements
- Add unit veterancy system
- Implement supply lines
- Add weather effects
- More terrain types (rivers, bridges)
- Air and naval units
- Unit production system
- Better pathfinding visualization
- Mini-map
- Strategic zoom mode

## Git Workflow (if using version control)

```bash
# Feature branch
git checkout -b feature/new-unit-type

# Make changes
git add constants.py main.py
git commit -m "Add new unit type: Commandos"

# Merge to main
git checkout main
git merge feature/new-unit-type
```

## Useful Resources

- **Arcade Documentation**: https://api.arcade.academy/
- **Arcade GUI Guide**: https://api.arcade.academy/en/latest/programming_guide/gui/index.html
- **Pyglet Batch**: https://pyglet.readthedocs.io/en/latest/programming_guide/graphics.html#batches-and-groups
- **Hex Grid Guide**: https://www.redblobgames.com/grids/hexagons/
- **Original PG2**: OpenPanzer project at http://openpanzer.net

## Quick Reference

### Key Keyboard Shortcuts
- `SPACE` - End turn
- `ESC` - Deselect unit
- `R` - Restart game (when game over)

### Key Mouse Actions
- `Left Click` - Select/Move unit
- `Right Click` - Attack
- `Mouse Move` - Highlight hex

### Important Constants
- `SCREEN_WIDTH = 1200`, `SCREEN_HEIGHT = 800`
- `GAME_PANEL_WIDTH = 850`, `HUD_PANEL_WIDTH = 350`
- `HEX_SIZE = 40`
- `MAP_ROWS = 12`, `MAP_COLS = 16`

### Unit Symbol Mapping
- `I` = Infantry
- `T` = Tank
- `R` = Recon
- `A` = Anti-Tank
- `X` = Artillery
- `F` = Fighter
- `B` = Bomber

---

**Last Updated**: v0.2.0  
**For Claude Code**: This file provides essential context for AI-assisted development
