# Open Wanzer

A turn-based tactical mech combat game inspired by classic titles like Panzer General, featuring hex-based tactical gameplay with BattleTech-inspired armor systems.

## Features

- **Hex-Based Tactical Combat**: Classic turn-based strategy on a hex grid
- **Advanced Armor System**: BattleTech-style 8-location damage tracking (Head, Center Torso, Left/Right Torso, Left/Right Arm, Left/Right Leg)
- **Multiple Attack Arcs**: Front, side, and rear attack positioning affects combat
- **Hit Location Tables**: Detailed damage resolution with location-specific targeting
- **Paperdoll UI**: Visual mech status display showing armor damage per location
- **Fog of War**: Dynamic line-of-sight and spotting mechanics
- **Multiple Unit Types**: Infantry, mechs, vehicles, and aircraft
- **Customizable Themes**: 14+ visual themes with custom fonts and color schemes

## Technology Stack

- **Language**: C++17
- **Graphics**: [raylib](https://www.raylib.com/) - Simple and easy-to-use game development library
- **UI**: [raygui](https://github.com/raysan5/raygui) - Immediate mode GUI for raylib
- **Hex Math**: Custom hex coordinate system with axial/offset conversion

## Project Structure

```
openwanzer/
├── src/                   # Source code (all .cpp files, PascalCase naming)
├── include/               # Headers (all .h files, PascalCase naming)
├── lib/                   # External library binaries
├── resources/             # Game assets (fonts, themes, styles)
├── tests/                 # Unit tests (future)
├── docs/                  # Documentation
├── scripts/               # Build and utility scripts
└── examples/              # Example scenarios and maps
```

### Module Organization

The codebase is organized into logical modules (all files use PascalCase):

- **Core**: GameState, Unit, Enums, Types, Constants, HexCoord, GameHex, ArmorLocation
- **Game Logic**: GameLogic, Combat, Pathfinding, Systems, Utilities, AttackLines, CombatArcs, DamageSystem, HitTables
- **Rendering**: Rendering, HexDrawing, UIDrawing, CombatVisuals, PaperdollUI
- **Input**: Input, Camera
- **Config**: Config, Persistence, StyleManager
- **UI**: UIPanels

## Building

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- raylib 5.5.0 (included in `lib/` and `include/`)
- Linux: X11 development libraries
- Make

### Build Commands

```bash
# Build release version
make

# Build debug version with symbols
make debug

# Build and run
make run

# Clean build artifacts
make clean

# Format code
make format

# Show all available targets
make help
```

### Installation

```bash
# Install to /usr/local/bin (requires sudo on Linux/macOS)
sudo make install

# Uninstall
sudo make uninstall
```

## Running the Game

```bash
./openwanzer
```

### Controls

- **Left Click**: Select unit, move, attack
- **Right Click**: Deselect unit
- **Mouse Wheel**: Zoom in/out
- **Middle Mouse / Space + Drag**: Pan camera
- **ESC**: Open options menu
- **Tab**: Cycle through units
- **Enter**: End turn

## Configuration

Game settings are saved to `config.txt` including:
- Resolution and fullscreen mode
- Graphics settings (VSync, MSAA, hex size)
- UI theme selection
- Camera pan speed

## Game Mechanics

### Movement
- Each unit has movement points based on type
- Terrain affects movement cost
- Units can move and attack in the same turn

### Combat
- Attack arcs determine hit tables (front/side/rear)
- Damage is applied to specific armor locations
- Critical hits can destroy locations
- Destroyed locations affect unit performance

### Armor Locations
- **Head**: Critical systems, low armor
- **Center Torso**: Main body, destruction is fatal
- **Left/Right Torso**: Side armor sections
- **Left/Right Arm**: Weapon mounts
- **Left/Right Leg**: Mobility systems

## Development

### Code Style
- C++17 standard library
- PascalCase for file names (e.g., GameState.cpp, Unit.h)
- snake_case for functions and variables
- PascalCase for classes and structs
- Flat directory structure (all headers in `include/`, all source in `src/`)
- Consistent header guards: `OPENWANZER_FILENAME_H`
- Namespaces for logical grouping (GameLogic, Rendering, Config)

### Adding New Features

1. Read `docs/ARCHITECTURE.md` for codebase structure
2. Read `docs/CONTRIBUTING.md` for contribution guidelines
3. Make changes in appropriate module
4. Update relevant documentation
5. Test thoroughly
6. Submit pull request

## Roadmap

- [ ] Multiplayer support
- [ ] Campaign mode
- [ ] Unit customization system
- [ ] Map editor
- [ ] Sound effects and music
- [ ] AI improvements
- [ ] Save/load game state
- [ ] Unit experience and veterancy
- [ ] More unit types and weapons

## License

[Specify your license here]

## Credits

- Built with [raylib](https://www.raylib.com/) by Ramon Santamaria
- Hex math based on [Red Blob Games](https://www.redblobgames.com/grids/hexagons/) by Amit Patel
- Inspired by Panzer General (SSI) and BattleTech (FASA)

## Contributing

Contributions are welcome! Please see `docs/CONTRIBUTING.md` for guidelines.

## Support

For issues, questions, or suggestions, please open an issue on GitHub.
