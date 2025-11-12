# OpenWanzer - Local Development Setup

This guide will help you get OpenWanzer running locally for development and testing.

## Quick Start

1. **Start the development server:**
   ```bash
   python3 dev-server.py
   ```

2. **Open Firefox and navigate to:**
   ```
   http://localhost:8000/index.html
   ```

3. **Start playing!** The game should load with a minimal tutorial scenario.

## What's Included

### Minimal Data Files
- **Equipment Data**: Basic units for Germany (country 7) and Spanish Republicans (country 25)
  - Infantry, Tanks, Artillery, Recon, Fighters, Transports, Engineers, AT Guns, AA Guns
  - Located in `resources/equipment/`

- **Tutorial Scenario**: A small 15x20 hex map with basic combat setup
  - Germans vs Spanish Republicans
  - Located in `resources/scenarios/data/tutorial.xml`

### Placeholder Assets
All created as solid-color PNG files for development:
- Map backgrounds (600x450px)
- Unit sprite sheets (324x32px, 9 orientations)
- Flags and UI indicators
- Generated via `create_placeholders.py`

### Error Handling
Modified `js/render.js` to gracefully handle missing assets:
- Images that fail to load will show console warnings instead of crashing
- Game will continue running even with missing graphics

## Development Workflow

### Running the Game
```bash
# Start the server
python3 dev-server.py

# In another terminal, you can regenerate placeholders if needed
python3 create_placeholders.py
```

### Testing in Firefox
- Firefox is the primary development browser
- Open DevTools (F12) to see console logs and warnings
- Check the Console tab for any loading errors

### Adding New Content

#### Adding Units
1. Edit equipment files in `resources/equipment/`
2. Add unit ID to scenario XML
3. Create unit sprite image (or use placeholders)

#### Modifying the Scenario
Edit `resources/scenarios/data/tutorial.xml`:
- Adjust map size: `rows` and `cols` attributes
- Add/remove units: `<unit>` elements within `<hex>` elements
- Change terrain: `terrain` attribute (0=clear, 1=city, 2=airfield, 3=forest, etc.)

#### Creating New Scenarios
1. Create new XML file in `resources/scenarios/data/`
2. Add entry to `resources/scenarios/scenariolist.js`
3. Update `Game.defaultScenario` in `js/game.js` if you want it as default

## Project Structure

```
openwanzer/
├── index.html              # Game entry point
├── dev-server.py          # Development server
├── create_placeholders.py # Asset generation script
├── js/                    # Game engine
│   ├── game.js           # Main game loop
│   ├── render.js         # Canvas rendering (modified for error handling)
│   ├── scenarioloader.js # XML scenario parser
│   ├── equipment.js      # Unit equipment system
│   └── ...               # Other game modules
├── css/                   # UI stylesheets
├── resources/
│   ├── equipment/        # Unit data (JSON)
│   ├── scenarios/        # Scenario definitions (XML)
│   ├── maps/images/      # Map backgrounds
│   ├── units/images/     # Unit sprites
│   └── ui/               # UI elements
└── tools/                # Conversion tools (for PG2 data)
```

## Game Controls

Once loaded, you should see a start menu. The tutorial scenario includes:
- **German forces** (left side): Infantry, Tank, Artillery, Recon
- **Spanish Republican forces** (right side): Militia, Infantry, T-26, Field Gun
- **Victory hexes**: Central Town and Fortress
- **Turn limit**: 20 turns

### Basic Controls (in-game)
- Click units to select
- Click hexes to move/attack
- Right-click for context menu
- Mouse wheel to zoom (if implemented)

## Troubleshooting

### Game won't load
- Check browser console for errors (F12)
- Verify dev server is running on port 8000
- Make sure you're accessing via `http://localhost:8000`, not `file://`

### Missing graphics
- This is expected with placeholder images
- Check console for "Failed to load" warnings
- Regenerate placeholders: `python3 create_placeholders.py`

### Equipment not loading
- Verify JSON syntax in `resources/equipment/equipment-country-*.json`
- Check that country IDs in scenario match equipment files
- Console will show "Loading equipment" messages

### Scenario won't load
- Validate XML syntax in scenario file
- Ensure all referenced unit IDs exist in equipment files
- Check that map dimensions are reasonable (< 99x99)

## Next Steps

### Adding Real Assets
Replace placeholder images with actual game graphics:
- Unit sprites: 324x32px PNG (9x 36x32 sprites)
- Map backgrounds: Variable size, referenced in scenario XML
- Flags: 21x14px sprites in a horizontal sheet

### Expanding Content
1. Create more equipment types in JSON files
2. Design larger, more complex scenarios
3. Add campaign support (JSON campaign files)
4. Import data from Panzer General 2 using tools in `tools/`

### Code Modernization
Current code uses ES5. Consider modernizing:
- Convert to ES6+ modules
- Add TypeScript for type safety
- Implement modern build tools (Vite, Webpack)
- Add unit tests

## Resources

- Original OpenPanzer: https://github.com/nicupavel/openpanzer
- Panzer General 2 community resources
- HTML5 Canvas API documentation
- Game development references for hex-grid strategy games

## Notes

- No external JavaScript libraries are used (vanilla JS only)
- Game uses HTML5 Canvas for rendering
- Scenarios stored as XML for compatibility with PG2 tools
- Equipment data is JSON for easy editing
- All game state can be saved to browser localStorage

Happy coding! 🎮
