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
- 2-panel GUI using horizontal UIBoxLayout: game panel (70%) + HUD panel (30%)
- Camera2D with viewport for scrollable game view constrained to game panel
- Event handlers: `on_draw()`, `on_update()`, `on_mouse_press()`, `on_key_press()`

**Important Methods**:
- `setup()` - Initialize game state and GUI
- `setup_gui()` - Configure 2-panel layout with arcade.gui.UIBoxLayout (horizontal)
- `update_hud()` - Update HUD labels with current game state
- `draw_map()` - Render hex grid and terrain
- `draw_units()` - Render all units with health bars
- `select_unit()` / `deselect_unit()` - Handle unit selection
- `on_draw()` - Sets viewport for game camera, renders game and GUI

**Performance Critical**:
- Uses arcade.gui.UILabel for HUD text (efficient rendering)
- Viewport constrains game rendering to left 70% of screen
- Panel boundaries checked for mouse interactions

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

### 1. GUI Layout Pattern (70/30 Split)
**Problem**: Creating a responsive 2-panel layout with game panel and HUD

**Solution**: Use horizontal UIBoxLayout with size_hint
```python
def setup_gui(self):
    # Create horizontal layout for 70/30 split
    main_layout = arcade.gui.UIBoxLayout(
        vertical=False,  # Horizontal layout
        space_between=0
    )

    # Game panel (left, 70%) - transparent spacer
    game_panel = arcade.gui.UISpace(
        color=(0, 0, 0, 0),
        size_hint=(0.7, 1)  # 70% width, 100% height
    )

    # HUD panel (right, 30%) - with content
    hud_container = arcade.gui.UIAnchorLayout(
        size_hint=(0.3, 1)  # 30% width, 100% height
    )
    # ... add HUD widgets to hud_container ...

    # Add both to main layout
    main_layout.add(game_panel)
    main_layout.add(hud_container)
    self.ui_manager.add(main_layout)
```

**Rule**: Use UIBoxLayout for horizontal/vertical splits, UIAnchorLayout for positioning within containers

### 2. Camera and Viewport Pattern
**Problem**: Constraining game rendering to only the game panel area

**Solution**: Set camera viewport before using camera
```python
def on_draw(self):
    self.clear()

    # Set viewport to game panel area (left 70%)
    self.game_camera.viewport = (0, 0, self.game_panel_width, self.height)

    # Position camera for scrolling (world space position)
    self.game_camera.position = (self.camera_x, self.camera_y)
    self.game_camera.use()

    # Draw game content (constrained to viewport)
    self.draw_map()
    self.draw_units()

    # Reset to default camera for UI (full screen)
    self.default_camera.use()
    self.ui_manager.draw()
```

**Camera Movement** (keyboard controls):
```python
# UP key: view moves up -> camera looks lower
elif key in (arcade.key.UP, arcade.key.W):
    self.camera_y -= self.camera_speed

# DOWN key: view moves down -> camera looks higher
elif key in (arcade.key.DOWN, arcade.key.S):
    self.camera_y += self.camera_speed

# LEFT key: view moves left -> camera looks right
elif key in (arcade.key.LEFT, arcade.key.A):
    self.camera_x += self.camera_speed

# RIGHT key: view moves right -> camera looks left
elif key in (arcade.key.RIGHT, arcade.key.D):
    self.camera_x -= self.camera_speed
```

**Rule**: Camera position is where the camera looks in world space (inverse of view movement direction)

### 3. Unit Selection Pattern
```python
# Select unit
self.selected_unit = unit
self.reachable_hexes = calculate_movement_range()
self.attackable_hexes = calculate_attack_range()
self.update_hud()  # Update HUD

# Deselect unit
self.selected_unit = None
self.reachable_hexes = {}
self.attackable_hexes = []
self.update_hud()  # Clear HUD
```

### 4. GUI Panel Boundary Pattern
```python
# Game panel is left 70% of screen width
game_panel_width = int(self.width * 0.7)

# Mouse clicks should respect boundaries
if x >= game_panel_width:
    return  # Click was in HUD, ignore for game actions

# Viewport automatically constrains drawing to game panel
self.game_camera.viewport = (0, 0, game_panel_width, self.height)
# No need to manually check drawing boundaries
```

### 5. Coordinate System Pattern
**Arcade 3.x uses LRBT** (Left-Right-Bottom-Top) for rectangles:
```python
# CORRECT
arcade.draw_lrbt_rectangle_filled(left, right, bottom, top, color)
# where bottom < top

# WRONG (will throw ValueError)
arcade.draw_lrbt_rectangle_filled(left, right, top, bottom, color)
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

1. **Create UILabel in setup_gui() in main.py**:
```python
def setup_gui(self):
    # ... existing setup ...

    # Create new info label
    new_info_label = arcade.gui.UILabel(
        text="New Info: 0",
        font_size=12,
        text_color=arcade.color.WHITE,
        width=self.hud_panel_width - 40
    )

    # Add to HUD box
    self.hud_box.add(new_info_label)

    # Store reference for updates
    self.new_info_label = new_info_label
```

2. **Update in update_hud()**:
```python
def update_hud(self):
    # ... existing updates
    self.new_info_label.text = f"New Info: {self.game_state.new_value}"
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
- Use `arcade.gui.UIManager()` for all GUI elements
- `UIBoxLayout` for horizontal/vertical arrangements (use for 70/30 splits)
- `UIAnchorLayout` for positioning within containers (default size_hint=(1,1))
- `UIGridLayout` for grid-based layouts with specific column/row counts
- Always call `ui_manager.enable()` in setup and `ui_manager.draw()` in on_draw
- Use `size_hint` to control proportional sizing (e.g., (0.7, 1) for 70% width, 100% height)

### GUI Text Rendering
```python
# Create UILabel for HUD text (efficient, managed by UIManager)
label = arcade.gui.UILabel(
    text="Hello",
    font_size=14,
    text_color=arcade.color.WHITE,
    width=300
)
hud_box.add(label)  # Add to layout

# Update text content (cheap)
label.text = "New content"
label.text_color = new_color

# UIManager automatically renders all GUI elements
ui_manager.draw()  # In on_draw()
```

### Camera and Viewport
```python
# Set viewport to constrain rendering area
camera.viewport = (left, bottom, width, height)  # In pixels

# Position camera in world space
camera.position = (world_x, world_y)
camera.use()

# Camera movement: position is inverse of view direction
# UP -> decrease camera_y (view moves up, camera looks lower)
# LEFT -> increase camera_x (view moves left, camera looks right)
```

## Performance Considerations

### Current Performance
- Target: 60 FPS
- Actual: Stable 60 FPS
- Text rendering: <1ms/frame
- Map rendering: ~2-3ms/frame
- Unit rendering: ~1ms/frame

### Performance Rules
1. **GUI**: Use arcade.gui.UILabel for HUD text, never arcade.draw_text()
2. **Camera**: Set viewport to constrain rendering to specific screen areas
3. **Rendering**: Minimize draw calls, use layouts efficiently
4. **Updates**: Only update what changed (e.g., only update_hud() when state changes)
5. **Pathfinding**: Cache results when possible
6. **AI**: Limit AI thinking time per frame

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
- Verify UILabels are added to HUD layout (hud_box.add())
- Check that update_hud() is called after state changes
- Verify panel boundaries are respected in mouse event handlers
- Ensure camera viewport is set before using game camera
- Confirm camera movement directions are correct (inverse of view movement)

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
- `Arrow Keys / WASD` - Pan camera (scroll the map)

### Key Mouse Actions
- `Left Click` - Select/Move unit
- `Right Click` - Attack
- `Middle Click + Drag` - Pan camera
- `Mouse Move` - Highlight hex

### Important Constants
- `SCREEN_WIDTH = 1200`, `SCREEN_HEIGHT = 800` (initial window size, resizable)
- `game_panel_ratio = 0.7` (70% of width for game panel)
- `hud_panel_ratio = 0.3` (30% of width for HUD panel)
- Panel widths calculated dynamically: `int(width * ratio)`
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
