# OpenWanzer - LLM Development Context

## Project Overview

OpenWanzer is a turn-based tactical strategy game inspired by Panzer General 2, built with Python Arcade 3.x. This is an early-stage prototype focusing on core hex-based gameplay mechanics.

**Technology Stack:**
- Python 3.8+
- Arcade 3.x (2D game engine)
- Event-driven architecture
- Component-based design pattern

## Quick Start

```bash
# Run the game
uv run main.py

# Or with pip
pip install arcade
python main.py
```

**Note for Claude Code:**
- Dependencies are automatically installed via `.claude/settings.json` sessionStart hook
- The hook runs `uv sync` which installs arcade and all dependencies from `pyproject.toml`
- No manual dependency installation needed when starting a Claude Code session

## Project Structure

```
openwanzer/
├── main.py           # Game window, rendering, GUI, input handling
├── constants.py      # Game data, enums, unit stats, configurations
├── hex_map.py        # Hex grid system, coordinate math, pathfinding
├── unit.py           # Unit class, combat mechanics, state
├── game_state.py     # Game state management, turn logic, rules
├── ai.py             # AI decision making
├── doc/arcade/       # Python Arcade documentation (see doc/arcade/README.md)
└── CLAUDE.md         # This file
```

## Architecture Guidelines

### Separation of Concerns

**main.py** - Presentation layer only
- All rendering and visual presentation
- GUI layout and HUD management
- Input event handling (mouse, keyboard)
- Camera and viewport control
- NO game logic or state mutation (delegate to game_state.py)

**game_state.py** - Business logic layer
- Game rules and validation
- Turn management and flow control
- Victory condition checking
- Unit action coordination (move, attack)

**hex_map.py, unit.py** - Domain model
- Core game entities and their behaviors
- Coordinate systems and spatial calculations
- Pathfinding and movement calculations
- Combat resolution

**constants.py** - Configuration
- All configurable values
- Enums for type safety
- Static game data (unit stats, terrain properties)

**ai.py** - AI agents
- Autonomous decision making for computer players
- Strategy and tactical evaluation

### Key Design Patterns

#### 1. Component-Based Architecture
Each module has a single, well-defined responsibility. Avoid cross-cutting concerns.

#### 2. Event-Driven Game Loop
Arcade's event system (`on_draw`, `on_update`, `on_mouse_press`, etc.) drives all game interactions.

#### 3. Separation of Data and Presentation
- Game state is independent of rendering
- Multiple views could render the same game state
- Enables testing without graphics

## Python Arcade 3.x Best Practices

> **Full documentation:** See `doc/arcade/README.md` for quick reference to Arcade docs

### Modern Drawing API (Critical)

**Always use LRBT (Left-Right-Bottom-Top) format:**
```python
# ✅ CORRECT - LRBT format (bottom < top)
arcade.draw_lrbt_rectangle_filled(left, right, bottom, top, color)
# where bottom < top

# ❌ WRONG - Will throw ValueError
arcade.draw_lrbt_rectangle_filled(left, right, top, bottom, color)
```

**Deprecated functions to avoid:**
- ❌ `arcade.draw_rectangle_filled/outline()` - Use LRBT versions
- ❌ `arcade.draw_text()` in game loop - Use `arcade.Text` objects or `arcade.gui.UILabel`

### GUI System

**UIManager and Layouts:**
- `UIBoxLayout` - For horizontal/vertical splits (e.g., game panel + HUD)
- `UIAnchorLayout` - For positioning widgets within containers
- `UIGridLayout` - For grid-based layouts
- `size_hint=(width_ratio, height_ratio)` - Proportional sizing (e.g., `(0.7, 1)` = 70% width)

**Example: Two-panel layout**
```python
# Horizontal split: 70% game panel, 30% HUD
main_layout = arcade.gui.UIBoxLayout(vertical=False)
game_panel = arcade.gui.UISpace(size_hint=(0.7, 1), color=(0, 0, 0, 0))
hud_panel = arcade.gui.UIAnchorLayout(size_hint=(0.3, 1))
main_layout.add(game_panel)
main_layout.add(hud_panel)
```

**Text Rendering:**
```python
# ✅ For HUD/GUI - Efficient, managed rendering
label = arcade.gui.UILabel(text="Score: 100", font_size=14)
hud_layout.add(label)
label.text = "Score: 200"  # Update dynamically

# For game world text, use arcade.Text objects with batches
```

### Camera and Viewport

**Camera2D for scrollable viewports:**
```python
# Set viewport to constrain rendering area
camera.viewport = arcade.types.LBWH(left, bottom, width, height)

# Position camera in world space
camera.position = (world_x, world_y)
camera.use()

# Camera position is where camera LOOKS in world space
# Movement is INVERSE: pan view right → camera looks left (decrease camera_x)
```

See: `doc/arcade/programming_guide/camera.rst`

### Performance Considerations

1. **Text rendering:** Use UILabel (not draw_text in loops)
2. **Sprite batching:** Use SpriteLists for many sprites
3. **Texture atlases:** Combine textures to reduce draw calls
4. **Viewport culling:** Only render what's visible
5. **Cache calculations:** Avoid recalculating paths/ranges every frame

See: `doc/arcade/programming_guide/performance_tips.rst`

## Development Workflow

### Adding New Features

**CRITICAL: Always check Arcade documentation BEFORE implementing any GUI/rendering feature!**

The workflow for any Arcade-related feature should be:

1. **Check Arcade docs FIRST** (`doc/arcade/`) - Search for existing widgets, APIs, and patterns
   - For GUI features: Check `doc/arcade/programming_guide/gui/`
   - For drawing: Check `doc/arcade/api_docs/api/`
   - For examples: Check `doc/arcade/example_code/`
   - Use `grep` to search docs: `grep -r "keyword" doc/arcade/`

2. **Use Arcade's built-in solutions** - Prefer Arcade's widgets and APIs over custom code
   - Example: Use `UITextArea`, `UILabel`, `UIBoxLayout` instead of custom text rendering
   - Example: Use `Camera2D` instead of manual viewport math
   - Example: Use `UIBorder` instead of drawing custom borders

3. **Only create custom code if:**
   - No Arcade widget/API exists for the requirement
   - Arcade's solution is insufficient for the specific use case
   - Custom code is explicitly required by the design

4. **Define data structures** (constants.py) - Add enums, unit data, etc.

5. **Implement domain logic** (hex_map.py, unit.py, game_state.py) - Core mechanics

6. **Add presentation** (main.py) - Rendering and UI using Arcade's APIs

7. **Test thoroughly** - **ALWAYS test before committing**
   - Run the game: `uv run main.py` or `python main.py`
   - Verify all functionality works as expected
   - Check for runtime errors and exceptions
   - Test edge cases and interactions

8. **Commit changes** - Only after successful testing

9. **Update documentation** - If architecture changes

**Example workflow for adding a message box:**
```bash
# 1. Check Arcade docs for text/scroll widgets
grep -r "UITextArea\|scroll" doc/arcade/programming_guide/gui/

# 2. Found UITextArea widget - use it instead of custom implementation
# 3. Check if UIBorder exists for borders
# 4. Implement using Arcade's widgets
# 5. Test and commit
```

### Debugging Approach

**Enable debug visualizations:**
```python
# In draw_map() - Show hex coordinates
arcade.draw_text(f"{hex.row},{hex.col}", x, y, arcade.color.WHITE, 8,
                 anchor_x="center", anchor_y="center")

# Print game state
print(f"Camera: ({self.camera_x}, {self.camera_y})")
print(f"Selected hex: {hex}")
```

**Common issues:**
- Hex selection not working? Check camera offset in `get_hex_at_position()`
- GUI not updating? Ensure `update_hud()` called after state changes
- Drawing errors? Verify LRBT format (bottom < top)
- Mouse clicks wrong panel? Check `x < game_panel_width` boundaries

### Code Style

**Follow PEP 8 conventions:**
- Classes: `PascalCase`
- Functions/methods: `snake_case`
- Constants: `UPPER_SNAKE_CASE`
- Private methods: `_leading_underscore()`

**Documentation:**
- Docstrings for all classes and non-trivial methods
- Inline comments for complex algorithms
- Type hints encouraged but not required

## Hex Grid System

**Coordinate System:** Axial coordinates with offset columns
- Even columns: straightforward neighbor calculation
- Odd columns: row offset for neighbors
- See: `hex_map.py:get_neighbors()` for neighbor calculation

**Pixel Conversion:**
```python
x = offset_x + HEX_SIZE * 1.5 * col
y = offset_y + HEX_HEIGHT * (row + 0.5 * (col % 2))
```

**Pathfinding:**
- Movement: Dijkstra (considers terrain cost)
- Shortest path: A* with Manhattan heuristic

**Reference:** https://www.redblobgames.com/grids/hexagons/

## Game Controls

**Keyboard:**
- `SPACE` - End turn
- `ESC` - Deselect unit
- `R` - Restart (when game over)
- `Arrow Keys / WASD` - Pan camera

**Mouse:**
- `Left Click` - Select unit / Move unit
- `Right Click` - Attack
- `Middle Click + Drag` - Pan camera
- `Mouse Move` - Highlight hex

## Current Limitations & TODOs

**Known limitations:**
- No save/load system
- No campaign mode
- Basic AI (no strategic planning)
- No sound effects
- Placeholder graphics (no sprites)
- No fog of war
- No multiplayer

**Potential improvements:**
- Unit veterancy/experience system
- Supply lines and logistics
- Weather and time-of-day effects
- More terrain types (rivers, bridges)
- Air and naval units
- Unit production/deployment
- Minimap
- Strategic zoom levels

## Arcade Documentation Quick Links

All Arcade documentation is in `doc/arcade/`. Key references:

- **Camera system:** `programming_guide/camera.rst`
- **GUI system:** `programming_guide/gui/`
- **Performance tips:** `programming_guide/performance_tips.rst`
- **Drawing API:** `api_docs/api/`
- **Keyboard input:** `programming_guide/keyboard.rst`

**Search example:**
```bash
# Find camera viewport documentation
grep -r "viewport" doc/arcade/programming_guide/

# Find available GUI widgets
grep -r "class UI.*Widget" doc/arcade/programming_guide/gui/

# Find text-related widgets
grep -r "UIText\|UILabel" doc/arcade/
```

## Environment Setup

**Claude Code Integration:**

The project uses `.claude/settings.json` for environment configuration:

```json
{
  "sessionStart": {
    "hook": {
      "command": "uv sync"
    }
  },
  "projectContext": {
    "name": "OpenWanzer - Panzer General 2 Prototype",
    "description": "Turn-based tactical strategy game built with Python Arcade 3.x",
    "keyFiles": [
      "main.py",
      "constants.py",
      "hex_map.py",
      "unit.py",
      "game_state.py",
      "ai.py",
      "CLAUDE.md"
    ]
  },
  "tools": {
    "packageManager": "uv",
    "testRunner": "pytest"
  }
}
```

**Session Start Hook:**
- Automatically runs `uv sync` when Claude Code session starts
- Installs all dependencies from `pyproject.toml` (including arcade)
- Creates/updates virtual environment in `.venv/`
- No manual setup required for development

## Git Workflow

```bash
# Feature branch
git checkout -b feature/feature-name

# Commit changes
git add .
git commit -m "Descriptive message"

# Push to remote
git push -u origin feature/feature-name
```

## Testing Checklist

When making changes, verify:
- [ ] Unit selection/deselection works
- [ ] Movement range highlighting correct
- [ ] Attack range highlighting correct (including artillery range-2)
- [ ] Combat resolution produces expected results
- [ ] Turn switching functions
- [ ] AI takes reasonable actions
- [ ] Victory conditions trigger correctly
- [ ] HUD updates reflect current state
- [ ] Mouse interactions respect panel boundaries
- [ ] Camera panning works correctly
- [ ] Window resize maintains layout
- [ ] No console errors or warnings

## Resources

- **Arcade API:** https://api.arcade.academy/
- **Arcade Examples:** https://api.arcade.academy/en/latest/examples/index.html
- **Hex Grid Math:** https://www.redblobgames.com/grids/hexagons/
- **OpenPanzer (PG2 project):** http://openpanzer.net

---

**Project Status:** Early prototype (v0.2.0)
**Last Updated:** 2025-11-13

**For LLM Developers:**
- This file provides high-level architecture and patterns
- Consult `doc/arcade/` for Arcade-specific implementation details
- Code is self-documenting - read source files for specifics
- Architecture may change as project matures
- Focus on separation of concerns and maintainability
