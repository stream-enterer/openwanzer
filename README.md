# Panzer General 2 - Python Prototype v0.2.0

A Python prototype of Panzer General 2 using the Arcade game engine, based on the JavaScript open source implementation.

## 🎮 What's New in v0.2.0

### Optimized Text Rendering
- Replaced slow `arcade.draw_text()` calls with efficient `arcade.Text` objects
- Implemented batch rendering using Pyglet's Batch system
- **10-100x performance improvement** for text rendering
- Eliminates performance warnings

### 2-Panel GUI Layout
- Professional split-screen layout with game on left, HUD on right
- Game view: 850px wide tactical map
- HUD panel: 350px wide information display
- Clean separation of gameplay and interface
- Responsive layout using arcade.gui system

### Enhanced HUD
- Real-time turn counter and player info
- Dynamic unit information display
- Clear control instructions
- Selected unit stats (strength, fuel, ammo, attack, defense)
- Visual feedback for current player

## About

This is a simplified but functional prototype featuring:
- **Hex-based tactical map** with different terrain types
- **Turn-based gameplay** with human vs AI
- **Multiple unit types**: Infantry, Tanks, Recon, Artillery
- **Combat system** with strength, fuel, and ammunition tracking
- **Movement mechanics** based on terrain and unit type
- **Objective capture** system
- **Simple AI opponent** that can move and attack
- **Optimized rendering** with Text batching
- **Professional GUI** with 2-panel layout

## Requirements

- Python 3.8+
- Arcade library (automatically installed)

## Installation & Running

### Using uv (Recommended - Fast!)
```bash
# Install uv if you don't have it
curl -LsSf https://astral.sh/uv/install.sh | sh

# Run the game directly (uv handles dependencies automatically)
uv run main.py
```

### Using pip (Traditional method)

#### On Linux/Mac:
```bash
# Install dependencies
pip install arcade

# Run the game
python main.py
```

#### On Windows:
```bash
# Install dependencies
pip install arcade

# Run the game
python main.py
```

## How to Play

### Controls
- **Left Click**: Select your unit / Move selected unit to hex
- **Right Click**: Attack with selected unit
- **SPACE**: End your turn
- **ESC**: Deselect current unit
- **R**: Restart game (when game is over)

### Gameplay
1. Select one of your units (red squares = Axis player)
2. Green highlighted hexes show where you can move
3. Red highlighted hexes show enemies you can attack
4. Move units to capture objectives (white/colored circles)
5. Check the HUD panel on the right for unit stats and info
6. End your turn when done - AI will take its turn automatically
7. Win by eliminating all enemy units or controlling objectives

### Unit Types
- **I** = Infantry (balanced, versatile)
- **T** = Tank (strong attack and defense)
- **R** = Recon (fast movement, good for scouting)
- **X** = Artillery (long-range attacks, weak defense)

### Terrain Types
- **Light Green**: Clear terrain (easy movement)
- **Dark Green**: Forest (moderate cover, slower movement)
- **Gray**: Mountains (strong cover, difficult movement)
- **Dark Gray**: Cities (strong cover, objectives to capture)
- **Brown**: Roads (fast movement)
- **Blue**: Water (impassable for ground units)

## Game Features

### Combat System
- Units have strength (10 max), fuel, and ammunition
- Combat effectiveness scales with unit strength
- Terrain provides defensive bonuses
- Units gain experience from combat
- Artillery can attack at range 2 hexes

### Movement System
- Different terrain costs different movement points
- Units consume fuel when moving
- Recon units can move multiple times per turn
- Roads provide fast movement

### Victory Conditions
- Eliminate all enemy units, OR
- Control the most objectives after 20 turns

### Performance Features
- Text rendering using pyglet Batch system for maximum efficiency
- No performance warnings or lag from UI updates
- Smooth 60 FPS gameplay

## Code Structure

- `main.py` - Main game loop, Arcade rendering, and GUI layout
- `constants.py` - Game constants, enums, and unit data
- `hex_map.py` - Hex coordinate system and map generation
- `unit.py` - Unit class with combat and movement
- `game_state.py` - Game state management and rules
- `ai.py` - Simple AI opponent logic

## Technical Improvements

### v0.2.0 Changes

**Text Rendering Optimization:**
- Migrated from `arcade.draw_text()` to `arcade.Text` objects
- Implemented Pyglet Batch rendering for all static text
- Dynamic text updates without recreating objects
- Eliminated PerformanceWarnings

**GUI Architecture:**
- Implemented `arcade.gui.UIManager` for layout management
- Used `UIBoxLayout` for 2-panel horizontal split
- Used `UISpace` for game panel background
- Styled HUD with padding and background color

**Code Organization:**
- Separated text creation from text updates
- Cleaner separation of concerns
- More maintainable codebase

## Based On

This prototype is based on the JavaScript open source port of Panzer General 2:
- Original game: Panzer General 2 by SSI (1997)
- JavaScript port: OpenPanzer (http://openpanzer.net)

## Version History

### v0.2.0 (Current)
- ✅ Optimized text rendering with Batch system
- ✅ Added 2-panel GUI layout
- ✅ Enhanced HUD with better information display
- ✅ Improved performance and eliminated warnings

### v0.1.1
- Fixed Arcade 3.x API compatibility
- Updated rectangle drawing functions to use lrbt format

### v0.1.0
- Initial release
- Basic hex-based tactical gameplay
- Human vs AI

## License

Based on the GPL-licensed OpenPanzer project.
Educational prototype for demonstration purposes.
