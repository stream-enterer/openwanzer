//==============================================================================
// PANZER GENERAL 2 PROTOTYPE - SINGLE FILE ARCHITECTURE
//==============================================================================
// A hex-based turn-based strategy game using raylib and raygui
// Organized with namespaces for maintainability while keeping single-file structure
//==============================================================================

//==============================================================================
// SECTION 1: INCLUDES AND PREPROCESSOR
//==============================================================================

#include "raylib.h"
#include "raymath.h"

// Suppress warnings from raygui.h (external library)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wstringop-overflow"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#pragma GCC diagnostic pop

#include "hex.h"

#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

//==============================================================================
// SECTION 2: CONSTANTS AND GLOBALS
//==============================================================================

// Default configuration values
const int DEFAULT_SCREEN_WIDTH = 1920;
const int DEFAULT_SCREEN_HEIGHT = 1080;
const float DEFAULT_HEX_SIZE = 40.0f;
const int DEFAULT_MAP_ROWS = 12;
const int DEFAULT_MAP_COLS = 16;

// Current settings (can be modified at runtime)
int SCREEN_WIDTH = DEFAULT_SCREEN_WIDTH;
int SCREEN_HEIGHT = DEFAULT_SCREEN_HEIGHT;
float HEX_SIZE = DEFAULT_HEX_SIZE;
int MAP_ROWS = DEFAULT_MAP_ROWS;
int MAP_COLS = DEFAULT_MAP_COLS;

// Color definitions
const Color COLOR_BACKGROUND = BLACK;
const Color COLOR_GRID = Color {245, 245, 220, 255}; // Pale beige
const Color COLOR_FPS = Color {192, 192, 192, 255};  // Light grey

//==============================================================================
// SECTION 3: ENUMS AND DATA TABLES
//==============================================================================

// Enums
enum class TerrainType {
	PLAINS,   // Open grassland
	FOREST,   // Woods/Trees
	MOUNTAIN, // High elevation
	HILL,     // Low elevation
	DESERT,   // Sandy/arid
	SWAMP,    // Marsh/wetland
	CITY,     // Urban
	WATER,    // River/lake
	ROAD,     // Paved road
	ROUGH     // Rocky/broken terrain
};

enum class UnitClass {
	INFANTRY,
	TANK,
	ARTILLERY,
	RECON,
	ANTI_TANK,
	AIR_DEFENSE
};

enum class Side { AXIS = 0,
	          ALLIED = 1 };

// Movement methods (12 types)
enum class MovMethod {
	TRACKED = 0,
	HALF_TRACKED = 1,
	WHEELED = 2,
	LEG = 3,
	TOWED = 4,
	AIR = 5,
	DEEP_NAVAL = 6,
	COSTAL = 7,
	ALL_TERRAIN_TRACKED = 8,
	AMPHIBIOUS = 9,
	NAVAL = 10,
	ALL_TERRAIN_LEG = 11
};

// Terrain type indices for movement tables
enum TerrainIndex {
	TI_CLEAR = 0,
	TI_CITY = 1,
	TI_AIRFIELD = 2,
	TI_FOREST = 3,
	TI_BOCAGE = 4,
	TI_HILL = 5,
	TI_MOUNTAIN = 6,
	TI_SAND = 7,
	TI_SWAMP = 8,
	TI_OCEAN = 9,
	TI_RIVER = 10,
	TI_FORTIFICATION = 11,
	TI_PORT = 12,
	TI_STREAM = 13,
	TI_ESCARPMENT = 14,
	TI_IMPASSABLE_RIVER = 15,
	TI_ROUGH = 16,
	TI_ROAD = 17
};

// Movement cost table [movMethod][terrain]
// 254 = Stop move (can enter but stops there), 255 = Don't enter (impassable)
const int MOV_TABLE_DRY[12][18] = {
    // Clear, City, Airfield, Forest, Bocage, Hill, Mountain, Sand, Swamp, Ocean, River, Fort, Port, Stream, Escarp, ImpassRiver, Rough, Road
    {1, 1, 1, 2, 4, 2, 254, 1, 4, 255, 254, 1, 1, 2, 255, 255, 2, 1},                       // Tracked
    {1, 1, 1, 2, 254, 2, 254, 1, 4, 255, 254, 1, 1, 2, 255, 255, 2, 1},                     // Half Tracked
    {2, 1, 1, 4, 254, 3, 254, 3, 254, 255, 254, 2, 1, 4, 255, 255, 2, 1},                   // Wheeled
    {1, 1, 1, 2, 2, 2, 254, 2, 2, 255, 254, 1, 1, 1, 255, 255, 2, 1},                       // Leg
    {1, 1, 1, 1, 1, 1, 254, 1, 255, 255, 254, 1, 1, 254, 255, 255, 1, 1},                   // Towed
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},                                 // Air
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 1, 255, 255, 1, 255, 255, 255, 255, 255}, // Deep Naval
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 2, 1, 255, 1, 255, 255, 255, 255, 255},   // Costal
    {1, 1, 1, 2, 3, 3, 254, 2, 254, 255, 254, 1, 1, 1, 255, 255, 3, 1},                     // All Terrain Tracked
    {1, 1, 1, 2, 4, 2, 254, 1, 3, 254, 3, 1, 1, 2, 255, 255, 2, 1},                         // Amphibious
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 1, 255, 255, 1, 255, 255, 255, 255, 255}, // Naval
    {1, 1, 1, 1, 2, 1, 1, 2, 2, 255, 254, 1, 1, 1, 255, 255, 1, 1}                          // All Terrain Leg (Mountain)
};

//==============================================================================
// SECTION 4: UTILITY FUNCTIONS (GAME LOGIC)
//==============================================================================

namespace GameLogic {

// Helper function to map TerrainType to movement table index
int getTerrainIndex(TerrainType terrain) {
	switch (terrain) {
		case TerrainType::PLAINS:
			return TI_CLEAR;
		case TerrainType::CITY:
			return TI_CITY;
		case TerrainType::FOREST:
			return TI_FOREST;
		case TerrainType::HILL:
			return TI_HILL;
		case TerrainType::MOUNTAIN:
			return TI_MOUNTAIN;
		case TerrainType::DESERT:
			return TI_SAND;
		case TerrainType::SWAMP:
			return TI_SWAMP;
		case TerrainType::WATER:
			return TI_OCEAN;
		case TerrainType::ROAD:
			return TI_ROAD;
		case TerrainType::ROUGH:
			return TI_ROUGH;
		default:
			return TI_CLEAR;
	}
}

// Terrain entrenchment levels (max entrenchment from terrain)
const int TERRAIN_ENTRENCHMENT[10] = {
    0, // PLAINS
    3, // CITY
    2, // FOREST
    2, // MOUNTAIN
    1, // HILL
    0, // DESERT
    0, // SWAMP
    3, // (unused - would be FORTIFICATION)
    0, // WATER
    2  // ROUGH
};

// Unit entrenchment rates (how fast they entrench)
const int UNIT_ENTRENCH_RATE[6] = {
    3, // INFANTRY (fast)
    1, // TANK (slow)
    2, // ARTILLERY (medium)
    2, // RECON (medium)
    2, // ANTI_TANK (medium)
    1  // AIR_DEFENSE (slow)
};

// Get terrain entrenchment level
int getTerrainEntrenchment(TerrainType terrain) {
	int idx = static_cast<int>(terrain);
	if (idx >= 0 && idx < 10) {
		return TERRAIN_ENTRENCHMENT[idx];
	}
	return 0;
}

// Get terrain type as display string
std::string getTerrainName(TerrainType terrain) {
	switch (terrain) {
		case TerrainType::PLAINS:
			return "Plains";
		case TerrainType::FOREST:
			return "Forest";
		case TerrainType::MOUNTAIN:
			return "Mountain";
		case TerrainType::HILL:
			return "Hill";
		case TerrainType::DESERT:
			return "Desert";
		case TerrainType::SWAMP:
			return "Swamp";
		case TerrainType::CITY:
			return "City";
		case TerrainType::WATER:
			return "Water";
		case TerrainType::ROAD:
			return "Road";
		case TerrainType::ROUGH:
			return "Rough";
		default:
			return "Unknown";
	}
}

// Get movement cost for a given terrain and movement method
int getMovementCost(MovMethod movMethod, TerrainType terrain) {
	int movMethodIdx = static_cast<int>(movMethod);
	int terrainIdx = getTerrainIndex(terrain);
	if (movMethodIdx >= 0 && movMethodIdx < 12 && terrainIdx >= 0 && terrainIdx < 18) {
		return MOV_TABLE_DRY[movMethodIdx][terrainIdx];
	}
	return 255; // Impassable by default
}

// Convert facing angle (0-360 degrees) to hybrid intercardinal/geometric notation
// Internal representation uses mathematical convention (E=0°, S=90°, W=180°, N=270°)
// Display uses military compass convention (N=0°, E=90°, S=180°, W=270°)
std::string getFacingName(float facing) {
	// Normalize angle to 0-360
	while (facing < 0)
		facing += 360.0f;
	while (facing >= 360.0f)
		facing -= 360.0f;

	// Convert from mathematical convention to compass convention
	// Math: E=0°, rotates clockwise → Compass: N=0°, rotates clockwise
	float compass = 90.0f - facing;
	while (compass < 0)
		compass += 360.0f;
	while (compass >= 360.0f)
		compass -= 360.0f;

	// Determine compass direction based on 16-point compass rose
	const char *direction;
	if (compass >= 348.75f || compass < 11.25f)
		direction = "N";
	else if (compass >= 11.25f && compass < 33.75f)
		direction = "NNE";
	else if (compass >= 33.75f && compass < 56.25f)
		direction = "NE";
	else if (compass >= 56.25f && compass < 78.75f)
		direction = "ENE";
	else if (compass >= 78.75f && compass < 101.25f)
		direction = "E";
	else if (compass >= 101.25f && compass < 123.75f)
		direction = "ESE";
	else if (compass >= 123.75f && compass < 146.25f)
		direction = "SE";
	else if (compass >= 146.25f && compass < 168.75f)
		direction = "SSE";
	else if (compass >= 168.75f && compass < 191.25f)
		direction = "S";
	else if (compass >= 191.25f && compass < 213.75f)
		direction = "SSW";
	else if (compass >= 213.75f && compass < 236.25f)
		direction = "SW";
	else if (compass >= 236.25f && compass < 258.75f)
		direction = "WSW";
	else if (compass >= 258.75f && compass < 281.25f)
		direction = "W";
	else if (compass >= 281.25f && compass < 303.75f)
		direction = "WNW";
	else if (compass >= 303.75f && compass < 326.25f)
		direction = "NW";
	else
		direction = "NNW";

	// Format as "E (090°)" - display compass bearing
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "%s (%03d°)", direction, (int)compass);
	return std::string(buffer);
}

} // namespace GameLogic

//==============================================================================
// SECTION 5: DATA STRUCTURES
//==============================================================================

// Forward declaration for calculateCenteredCameraOffset
struct CameraState;
void calculateCenteredCameraOffset(CameraState &camera, int screenWidth, int screenHeight);

// Camera state structure
struct CameraState {
	float offsetX;
	float offsetY;
	float zoom;        // 0.5 to 2.0 (50% to 200%)
	int zoomDirection; // -1 for zooming out, 1 for zooming in, 0 for neutral
	bool isPanning;
	Vector2 panStartMouse;
	Vector2 panStartOffset;

	CameraState()
	    : offsetX(0.0f), offsetY(0.0f), zoom(1.0f), zoomDirection(0), isPanning(false), panStartMouse {0, 0}, panStartOffset {0, 0} {
		// Initialize to centered position (will be properly calculated after layout is set)
		offsetX = 100.0f;
		offsetY = 100.0f;
	}
};

// Settings Structure
struct VideoSettings {
	int resolutionIndex;
	bool fullscreen;
	bool vsync;
	int fpsIndex;
	float hexSize;
	float panSpeed;
	bool msaa;
	int guiScaleIndex;
	std::string styleTheme;
	bool resolutionDropdownEdit;
	bool fpsDropdownEdit;
	bool guiScaleDropdownEdit;
	bool styleThemeDropdownEdit;

	VideoSettings()
	    : resolutionIndex(6), fullscreen(true), vsync(false), fpsIndex(6), hexSize(40.0f), panSpeed(5.0f), msaa(false), guiScaleIndex(0), styleTheme("dark"), resolutionDropdownEdit(false), fpsDropdownEdit(false), guiScaleDropdownEdit(false), styleThemeDropdownEdit(false) {
	}
};

// Game Layout Structure
struct GameLayout {
	Rectangle statusBar; // Top status bar
	Rectangle unitPanel; // Right unit info panel
	Rectangle helpBar;   // Bottom help text area
	Rectangle playArea;  // Map rendering area

	void recalculate(int w, int h) {
		statusBar = {0, 0, (float)w, 40};
		unitPanel = {(float)w - 250, 50, 250, 300};
		helpBar = {0, (float)h - 30, (float)w, 30};
		playArea = {0, 40, (float)w - 250, (float)h - 70};
	}

	GameLayout() {
		recalculate(SCREEN_WIDTH, SCREEN_HEIGHT);
	}
};

// Resolution options
struct Resolution {
	int width;
	int height;
	const char *label;
};

const Resolution RESOLUTIONS[] = {
    {800, 600, "800x600"}, {1024, 768, "1024x768"}, {1280, 720, "1280x720"}, {1280, 800, "1280x800"}, {1366, 768, "1366x768"}, {1600, 900, "1600x900"}, {1920, 1080, "1920x1080"}, {2560, 1440, "2560x1440"}, {3840, 2160, "3840x2160"}};
const int RESOLUTION_COUNT = 9;

const int FPS_VALUES[] = {30, 60, 75, 120, 144, 240, 0};
const char *FPS_LABELS = "30;60;75;120;144;240;Unlimited";

const float GUI_SCALE_VALUES[] = {1.0f, 1.5f, 2.0f};
const char *GUI_SCALE_LABELS = "1.00;1.50;2.00";
const int GUI_SCALE_COUNT = 3;

//==============================================================================
// SECTION 5A: CONFIGURATION - STYLE DISCOVERY
//==============================================================================

namespace Config {

// Global variables for style themes
std::vector<std::string> AVAILABLE_STYLES;
std::string STYLE_LABELS_STRING;

// Function to discover available styles
void discoverStyles() {
	AVAILABLE_STYLES.clear();
	const char *stylesPath = "resources/styles";

	DIR *dir = opendir(stylesPath);
	if (dir == nullptr) {
		TraceLog(LOG_WARNING, "Failed to open styles directory");
		// Add default style as fallback
		AVAILABLE_STYLES.push_back("default");
		STYLE_LABELS_STRING = "default";
		return;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != nullptr) {
		// Skip . and ..
		if (entry->d_name[0] == '.')
			continue;

		// Check if it's a directory
		std::string fullPath = std::string(stylesPath) + "/" + entry->d_name;
		struct stat statbuf;
		if (stat(fullPath.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
			// Check if .rgs file exists
			std::string rgsPath = fullPath + "/style_" + entry->d_name + ".rgs";
			std::ifstream rgsFile(rgsPath);
			if (rgsFile.good()) {
				AVAILABLE_STYLES.push_back(entry->d_name);
			}
		}
	}
	closedir(dir);

	// Sort styles alphabetically
	std::sort(AVAILABLE_STYLES.begin(), AVAILABLE_STYLES.end());

	// Create labels string
	STYLE_LABELS_STRING.clear();
	for (size_t i = 0; i < AVAILABLE_STYLES.size(); i++) {
		if (i > 0)
			STYLE_LABELS_STRING += ";";
		STYLE_LABELS_STRING += AVAILABLE_STYLES[i];
	}

	TraceLog(LOG_INFO, TextFormat("Found %d styles", (int)AVAILABLE_STYLES.size()));
}

// Get index of a style by name
int getStyleIndex(const std::string &styleName) {
	for (size_t i = 0; i < AVAILABLE_STYLES.size(); i++) {
		if (AVAILABLE_STYLES[i] == styleName) {
			return (int)i;
		}
	}
	return 0; // Default to first style if not found
}

} // namespace Config

// Structures
struct HexCoord {
	int row;
	int col;

	bool operator==(const HexCoord &other) const {
		return row == other.row && col == other.col;
	}
};

struct GameHex {
	HexCoord coord;
	TerrainType terrain;
	int owner; // -1 = neutral, 0 = axis, 1 = allied
	bool isVictoryHex;
	bool isDeployment;
	int spotted[2];   // spotting counter per side (team-based FOW)
	int zoc[2];       // zone of control counter per side
	bool isMoveSel;   // highlighted for movement
	bool isAttackSel; // highlighted for attack

	GameHex()
	    : terrain(TerrainType::PLAINS), owner(-1), isVictoryHex(false), isDeployment(false), isMoveSel(false), isAttackSel(false) {
		spotted[0] = 0;
		spotted[1] = 0;
		zoc[0] = 0;
		zoc[1] = 0;
	}

	void setSpotted(int side, bool on) {
		if (on) {
			spotted[side]++;
		} else if (spotted[side] > 0) {
			spotted[side]--;
		}
	}

	void setZOC(int side, bool on) {
		if (on) {
			zoc[side]++;
		} else if (zoc[side] > 0) {
			zoc[side]--;
		}
	}

	bool isSpotted(int side) const {
		return spotted[side] > 0;
	}
	bool isZOC(int side) const {
		return zoc[side] > 0;
	}
};

struct Unit {
	std::string name;
	UnitClass unitClass;
	int side;     // 0 = axis, 1 = allied
	int strength; // 1-10
	int maxStrength;
	int experience;   // 0-5 bars
	int entrenchment; // 0-5
	HexCoord position;

	// Combat stats
	int hardAttack;
	int softAttack;
	int groundDefense;
	int closeDefense;
	int initiative;

	// Movement & logistics
	MovMethod movMethod;
	int movementPoints;
	int movesLeft;
	int fuel;
	int maxFuel;
	int ammo;
	int maxAmmo;
	int spotRange;
	int rangeDefMod;   // Range defense modifier
	int hits;          // Accumulated hits (reduces defense)
	int entrenchTicks; // Ticks toward next entrenchment level

	bool hasMoved;
	bool hasFired;
	bool isCore; // campaign unit

	// Facing system (0-360 degrees, exact angle)
	float facing;

	Unit()
	    : strength(10), maxStrength(10), experience(0), entrenchment(0), hardAttack(8), softAttack(10), groundDefense(6), closeDefense(5), initiative(5), movMethod(MovMethod::TRACKED), movementPoints(6), movesLeft(6), fuel(50), maxFuel(50), ammo(20), maxAmmo(20), spotRange(2), rangeDefMod(0), hits(0), entrenchTicks(0), hasMoved(false), hasFired(false), isCore(false), facing(0.0f) {
	}
};

// Combat Log Message
struct LogMessage {
	int turn;
	std::string message;
	int count;

	LogMessage(int t, const std::string &msg)
	    : turn(t), message(msg), count(1) {
	}
};

// Combat Log Display
struct CombatLog {
	std::vector<LogMessage> messages;
	float scrollOffset;      // Current scroll position (0 = top, higher = scrolled down more)
	float maxScrollOffset;   // Maximum scroll offset
	Rectangle bounds;        // Display area
	bool isHovering;         // Is mouse hovering over log?
	bool isDragging;         // Is being dragged?
	Vector2 dragOffset;      // Offset from top-left corner to mouse when drag started
	Rectangle initialBounds; // Initial position for reset

	CombatLog() : scrollOffset(0.0f), maxScrollOffset(0.0f), isHovering(false), isDragging(false), dragOffset {0, 0} {
		// Lower right corner, 350px wide, 400px tall, 10px margin from edges
		bounds = {SCREEN_WIDTH - 360.0f, SCREEN_HEIGHT - 410.0f, 350.0f, 400.0f};
		initialBounds = bounds;
	}

	void recalculateBounds(int screenWidth, int screenHeight) {
		// Position in lower right (preserving offset from initial if dragged)
		float defaultX = screenWidth - 360.0f;
		float defaultY = screenHeight - 410.0f;
		float offsetX = bounds.x - initialBounds.x;
		float offsetY = bounds.y - initialBounds.y;

		initialBounds = {defaultX, defaultY, 350.0f, 400.0f};
		bounds = {defaultX + offsetX, defaultY + offsetY, 350.0f, 400.0f};
	}

	void resetPosition() {
		bounds = initialBounds;
	}
};

// Unit info box structure (upper right panel for selected unit)
struct UnitInfoBox {
	Rectangle bounds;        // Display area
	bool isHovering;         // Is mouse hovering over box?
	bool isDragging;         // Is being dragged?
	Vector2 dragOffset;      // Offset from top-left corner to mouse when drag started
	Rectangle initialBounds; // Initial position for reset

	UnitInfoBox() : isHovering(false), isDragging(false), dragOffset {0, 0} {
		// Upper right corner, below status bar (40px), 250px wide, 300px tall
		bounds = {SCREEN_WIDTH - 250.0f, 50.0f, 250.0f, 300.0f};
		initialBounds = bounds;
	}

	void recalculateBounds(int screenWidth, int screenHeight) {
		// Position in upper right (preserving offset from initial if dragged)
		float defaultX = screenWidth - 250.0f;
		float defaultY = 50.0f;
		float offsetX = bounds.x - initialBounds.x;
		float offsetY = bounds.y - initialBounds.y;

		initialBounds = {defaultX, defaultY, 250.0f, 300.0f};
		bounds = {defaultX + offsetX, defaultY + offsetY, 250.0f, 300.0f};
	}

	void resetPosition() {
		bounds = initialBounds;
	}
};

// Movement selection state (for two-phase selection system)
// Phase 1: Select unit, show movement range
// Phase 2: Unit has moved, selecting facing direction
struct MovementSelection {
	bool isFacingSelection; // Phase 2: unit has moved, now selecting facing
	HexCoord oldPosition;   // Position before move (for undo)
	int oldMovesLeft;       // Movement points before move (for undo)
	bool oldHasMoved;       // hasMoved state before move (for undo)
	int oldFuel;            // Fuel before move (for undo)
	float selectedFacing;   // The facing direction being previewed (0-360 degrees)

	MovementSelection() : isFacingSelection(false), oldPosition {-1, -1}, oldMovesLeft(0), oldHasMoved(false), oldFuel(0), selectedFacing(0.0f) {
	}

	void reset() {
		isFacingSelection = false;
		oldPosition = {-1, -1};
		oldMovesLeft = 0;
		oldHasMoved = false;
		oldFuel = 0;
		selectedFacing = 0.0f;
	}
};

// Game State
struct GameState {
	std::vector<std::vector<GameHex>> map;
	std::vector<std::unique_ptr<Unit>> units;
	Unit *selectedUnit;
	int currentTurn;
	int currentPlayer; // 0 or 1
	int maxTurns;
	bool showOptionsMenu;
	VideoSettings settings;
	GameLayout layout;
	CameraState camera;
	CombatLog combatLog;
	UnitInfoBox unitInfoBox;
	MovementSelection movementSel; // Two-phase selection state

	GameState()
	    : selectedUnit(nullptr), currentTurn(1), currentPlayer(0), maxTurns(20), showOptionsMenu(false) {
		initializeMap();
	}

	void initializeMap() {
		map.resize(MAP_ROWS);
		for (int row = 0; row < MAP_ROWS; row++) {
			map[row].resize(MAP_COLS);
			for (int col = 0; col < MAP_COLS; col++) {
				map[row][col].coord = {row, col};

				// Wargame terrain generation with realistic distribution
				int randVal = GetRandomValue(0, 100);
				if (randVal < 35)
					map[row][col].terrain = TerrainType::PLAINS; // 35% plains (most common)
				else if (randVal < 55)
					map[row][col].terrain = TerrainType::FOREST; // 20% forest
				else if (randVal < 68)
					map[row][col].terrain = TerrainType::HILL; // 13% hills
				else if (randVal < 75)
					map[row][col].terrain = TerrainType::ROUGH; // 7% rough
				else if (randVal < 82)
					map[row][col].terrain = TerrainType::DESERT; // 7% desert
				else if (randVal < 87)
					map[row][col].terrain = TerrainType::MOUNTAIN; // 5% mountain
				else if (randVal < 91)
					map[row][col].terrain = TerrainType::SWAMP; // 4% swamp
				else if (randVal < 95)
					map[row][col].terrain = TerrainType::CITY; // 4% city
				else
					map[row][col].terrain = TerrainType::WATER; // 5% water

				// Set some victory hexes
				if (row == 5 && (col == 4 || col == 12)) {
					map[row][col].isVictoryHex = true;
					map[row][col].owner = 1; // Start owned by Allies
				}
			}
		}
	}

	Unit *getUnitAt(const HexCoord &coord) {
		for (auto &unit : units) {
			if (unit->position == coord) {
				return unit.get();
			}
		}
		return nullptr;
	}

	void addUnit(UnitClass uClass, int side, int row, int col) {
		auto unit = std::make_unique<Unit>();
		unit->unitClass = uClass;
		unit->side = side;
		unit->position = {row, col};

		// Set unit name and movement method based on class
		switch (uClass) {
			case UnitClass::INFANTRY:
				unit->name = "Infantry";
				unit->movMethod = MovMethod::LEG;
				break;
			case UnitClass::TANK:
				unit->name = "Tank";
				unit->movMethod = MovMethod::TRACKED;
				break;
			case UnitClass::ARTILLERY:
				unit->name = "Artillery";
				unit->movMethod = MovMethod::HALF_TRACKED;
				break;
			case UnitClass::RECON:
				unit->name = "Recon";
				unit->movMethod = MovMethod::WHEELED;
				break;
			case UnitClass::ANTI_TANK:
				unit->name = "Anti-Tank";
				unit->movMethod = MovMethod::HALF_TRACKED;
				break;
			case UnitClass::AIR_DEFENSE:
				unit->name = "Air Defense";
				unit->movMethod = MovMethod::HALF_TRACKED;
				break;
		}

		// Initialize unit facing based on side (using degrees: E=0°, S=90°, W=180°, N=270°)
		// Axis units (left side) face generally East (toward right/enemy)
		// Allied units (right side) face generally West (toward left/enemy)
		if (side == 0) {
			unit->facing = 0.0f; // East (0°) - facing toward the right side of map
		} else {
			unit->facing = 180.0f; // West (180°) - facing toward the left side of map
		}

		units.push_back(std::move(unit));
	}
};

//==============================================================================
// SECTION 6: RENDERING FUNCTIONS
//==============================================================================

// Forward declarations from GameLogic (for cross-namespace calls)
namespace GameLogic {
std::vector<HexCoord> findPath(GameState &game, Unit *unit, const HexCoord &start, const HexCoord &goal);
}

namespace Rendering {

// Hex layout and coordinate conversion functions
Layout createHexLayout(float hexSize, float offsetX, float offsetY, float zoom) {
	Point size(hexSize * zoom, hexSize * zoom);
	Point origin(offsetX, offsetY);
	return Layout(layout_pointy, size, origin);
}

// Convert our game's row/col to hex library's offset coordinates
OffsetCoord gameCoordToOffset(const HexCoord &coord) {
	return OffsetCoord(coord.col, coord.row);
}

// Convert hex library's offset coordinates to our game's row/col
HexCoord offsetToGameCoord(const OffsetCoord &offset) {
	return HexCoord {offset.row, offset.col};
}

// Draw a hexagon using raylib
void drawHexagon(const std::vector<Point> &corners, Color color, bool filled) {
	if (filled) {
		// Draw filled hexagon using triangles from center
		Vector2 center = {0, 0};
		for (const auto &corner : corners) {
			center.x += corner.x;
			center.y += corner.y;
		}
		center.x /= corners.size();
		center.y /= corners.size();

		for (size_t i = 0; i < corners.size(); i++) {
			size_t next = (i + 1) % corners.size();
			DrawTriangle(
			    Vector2 {(float)corners[i].x, (float)corners[i].y},
			    Vector2 {(float)corners[next].x, (float)corners[next].y},
			    center,
			    color);
		}
	} else {
		// Draw hexagon outline
		for (size_t i = 0; i < corners.size(); i++) {
			size_t next = (i + 1) % corners.size();
			DrawLineEx(
			    Vector2 {(float)corners[i].x, (float)corners[i].y},
			    Vector2 {(float)corners[next].x, (float)corners[next].y},
			    2.0f, color);
		}
	}
}

Color getTerrainColor(TerrainType terrain) {
	switch (terrain) {
		case TerrainType::PLAINS:
			return Color {144, 186, 96, 255}; // Light green grass
		case TerrainType::FOREST:
			return Color {34, 102, 34, 255}; // Dark green woods
		case TerrainType::MOUNTAIN:
			return Color {120, 100, 80, 255}; // Gray-brown peaks
		case TerrainType::HILL:
			return Color {160, 140, 100, 255}; // Tan hills
		case TerrainType::DESERT:
			return Color {220, 200, 140, 255}; // Sandy yellow
		case TerrainType::SWAMP:
			return Color {100, 120, 80, 255}; // Murky green-brown
		case TerrainType::CITY:
			return Color {140, 140, 140, 255}; // Gray urban
		case TerrainType::WATER:
			return Color {80, 140, 200, 255}; // Blue water
		case TerrainType::ROAD:
			return Color {100, 100, 100, 255}; // Dark gray pavement
		case TerrainType::ROUGH:
			return Color {130, 110, 90, 255}; // Brown rocky
		default:
			return GRAY;
	}
}

Color getUnitColor(int side) {
	return side == 0 ? Color {200, 0, 0, 255} : Color {0, 0, 200, 255};
}

std::string getUnitSymbol(UnitClass uClass) {
	switch (uClass) {
		case UnitClass::INFANTRY:
			return "INF";
		case UnitClass::TANK:
			return "TNK";
		case UnitClass::ARTILLERY:
			return "ART";
		case UnitClass::RECON:
			return "RCN";
		case UnitClass::ANTI_TANK:
			return "AT";
		case UnitClass::AIR_DEFENSE:
			return "AA";
		default:
			return "???";
	}
}

void drawMap(GameState &game) {
	Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
	                                game.camera.offsetY, game.camera.zoom);

	// Draw hexes (all hexes always visible)
	for (int row = 0; row < MAP_ROWS; row++) {
		for (int col = 0; col < MAP_COLS; col++) {
			GameHex &hex = game.map[row][col];

			OffsetCoord offset = gameCoordToOffset(hex.coord);
			::Hex cubeHex = offset_to_cube(offset);

			std::vector<Point> corners = polygon_corners(layout, cubeHex);

			// Draw terrain
			Color terrainColor = getTerrainColor(hex.terrain);
			drawHexagon(corners, terrainColor, true);

			// Draw hex outline
			drawHexagon(corners, COLOR_GRID, false);

			// Draw victory hex marker
			if (hex.isVictoryHex) {
				Point center = hex_to_pixel(layout, cubeHex);
				DrawCircle((int)center.x, (int)center.y, 8 * game.camera.zoom, GOLD);
			}

			// Draw movement/attack selection highlights
			if (hex.isMoveSel) {
				std::vector<Point> innerCorners;
				Point center = hex_to_pixel(layout, cubeHex);
				for (int i = 0; i < 6; i++) {
					Point offset = hex_corner_offset(layout, i);
					float scale = 0.85f;
					innerCorners.push_back(Point(center.x + offset.x * scale,
					                             center.y + offset.y * scale));
				}
				drawHexagon(innerCorners, Color {0, 255, 0, 100}, true);
			}
			if (hex.isAttackSel) {
				std::vector<Point> innerCorners;
				Point center = hex_to_pixel(layout, cubeHex);
				for (int i = 0; i < 6; i++) {
					Point offset = hex_corner_offset(layout, i);
					float scale = 0.85f;
					innerCorners.push_back(Point(center.x + offset.x * scale,
					                             center.y + offset.y * scale));
				}
				drawHexagon(innerCorners, Color {255, 0, 0, 100}, true);
			}
		}
	}

	// Draw units (friendly units always visible, enemy units only if spotted)
	for (auto &unit : game.units) {
		GameHex &unitHex = game.map[unit->position.row][unit->position.col];

		// Hide enemy units that aren't spotted (FOG OF WAR)
		if (unit->side != game.currentPlayer && !unitHex.isSpotted(game.currentPlayer))
			continue;

		OffsetCoord offset = gameCoordToOffset(unit->position);
		::Hex cubeHex = offset_to_cube(offset);
		Point center = hex_to_pixel(layout, cubeHex);

		float unitWidth = 40 * game.camera.zoom;
		float unitHeight = 30 * game.camera.zoom;

		// Use facing angle directly (0-360 degrees)
		float rotation = unit->facing;

		// Draw unit rectangle with rotation
		Color unitColor = getUnitColor(unit->side);
		Rectangle unitRect = {(float)center.x, (float)center.y, unitWidth, unitHeight};
		Vector2 origin = {unitWidth / 2, unitHeight / 2};
		DrawRectanglePro(unitRect, origin, rotation, unitColor);

		// Draw unit symbol (rotated with unit)
		std::string symbol = getUnitSymbol(unit->unitClass);
		int fontSize = (int)(10 * game.camera.zoom);
		if (fontSize >= 8) { // Only draw text if it's readable
			int textWidth = MeasureText(symbol.c_str(), fontSize);

			// Note: Text is drawn unrotated at center for readability
			// TODO: When using sprite textures, use DrawTexturePro with rotation parameter
			// Example: DrawTexturePro(texture, sourceRec, destRec, origin, rotation, WHITE);
			DrawText(symbol.c_str(),
			         (int)(center.x - textWidth / 2),
			         (int)(center.y - fontSize / 2 - 5),
			         fontSize, WHITE);

			// Draw strength
			std::string strength = std::to_string(unit->strength);
			fontSize = (int)(12 * game.camera.zoom);
			textWidth = MeasureText(strength.c_str(), fontSize);
			DrawText(strength.c_str(),
			         (int)(center.x - textWidth / 2),
			         (int)(center.y + 5 * game.camera.zoom),
			         fontSize, YELLOW);
		}

		// Draw selection highlight (rotated to match unit facing)
		if (unit.get() == game.selectedUnit) {
			float rotRad = rotation * M_PI / 180.0f;
			float cosR = cos(rotRad);
			float sinR = sin(rotRad);

			// Draw first highlight box (4 pixels larger)
			float w1 = (unitWidth + 4) / 2;
			float h1 = (unitHeight + 4) / 2;
			Vector2 corners1[4] = {
			    {(float)(center.x + (-w1 * cosR - -h1 * sinR)), (float)(center.y + (-w1 * sinR + -h1 * cosR))},
			    {(float)(center.x + (w1 * cosR - -h1 * sinR)), (float)(center.y + (w1 * sinR + -h1 * cosR))},
			    {(float)(center.x + (w1 * cosR - h1 * sinR)), (float)(center.y + (w1 * sinR + h1 * cosR))},
			    {(float)(center.x + (-w1 * cosR - h1 * sinR)), (float)(center.y + (-w1 * sinR + h1 * cosR))}};
			for (int i = 0; i < 4; i++) {
				DrawLineEx(corners1[i], corners1[(i + 1) % 4], 2.0f, YELLOW);
			}

			// Draw second highlight box (6 pixels larger)
			float w2 = (unitWidth + 6) / 2;
			float h2 = (unitHeight + 6) / 2;
			Vector2 corners2[4] = {
			    {(float)(center.x + (-w2 * cosR - -h2 * sinR)), (float)(center.y + (-w2 * sinR + -h2 * cosR))},
			    {(float)(center.x + (w2 * cosR - -h2 * sinR)), (float)(center.y + (w2 * sinR + -h2 * cosR))},
			    {(float)(center.x + (w2 * cosR - h2 * sinR)), (float)(center.y + (w2 * sinR + h2 * cosR))},
			    {(float)(center.x + (-w2 * cosR - h2 * sinR)), (float)(center.y + (-w2 * sinR + h2 * cosR))}};
			for (int i = 0; i < 4; i++) {
				DrawLineEx(corners2[i], corners2[(i + 1) % 4], 2.0f, YELLOW);
			}
		}
	}

	// Draw movement zone outline (yellow contiguous border)
	if (game.selectedUnit && !game.selectedUnit->hasMoved) {
		Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
		                                game.camera.offsetY, game.camera.zoom);

		// Find edge hexes (hexes with at least one neighbor that's not moveable)
		for (int row = 0; row < MAP_ROWS; row++) {
			for (int col = 0; col < MAP_COLS; col++) {
				if (!game.map[row][col].isMoveSel)
					continue;

				HexCoord coord = {row, col};
				OffsetCoord offset = gameCoordToOffset(coord);
				::Hex cubeHex = offset_to_cube(offset);

				// Check each of the 6 edges
				for (int dir = 0; dir < 6; dir++) {
					::Hex neighbor = hex_neighbor(cubeHex, dir);
					OffsetCoord neighborOffset = cube_to_offset(neighbor);

					// CRITICAL: Convert offset coordinates back to game coordinates before map lookup
					HexCoord neighborCoord = offsetToGameCoord(neighborOffset);

					bool drawEdge = false;

					// Draw edge if neighbor is out of bounds or not in movement range
					if (neighborCoord.row < 0 || neighborCoord.row >= MAP_ROWS || neighborCoord.col < 0 || neighborCoord.col >= MAP_COLS) {
						drawEdge = true;
					} else if (!game.map[neighborCoord.row][neighborCoord.col].isMoveSel) {
						drawEdge = true;
					}

					if (drawEdge) {
						// Draw the edge between this hex and its neighbor
						// CRITICAL: Direction numbering doesn't match edge numbering!
						// For pointy-top hexes, direction → edge mapping is: dir → (5 - dir)
						// Direction 0 (E) uses edge 5, Direction 1 (SE) uses edge 4, etc.
						std::vector<Point> corners = polygon_corners(layout, cubeHex);
						int edgeIndex = (5 - dir + 6) % 6; // Correct edge for this direction
						Point p1 = corners[edgeIndex];
						Point p2 = corners[(edgeIndex + 1) % 6];
						DrawLineEx(Vector2 {(float)p1.x, (float)p1.y},
						           Vector2 {(float)p2.x, (float)p2.y},
						           3.0f * game.camera.zoom, YELLOW);
					}
				}
			}
		}
	}

	// Draw path preview (semi-transparent snake showing planned path)
	// Only show in Phase 1 (before moving)
	if (game.selectedUnit && !game.movementSel.isFacingSelection && !game.selectedUnit->hasMoved) {
		Vector2 mousePos = GetMousePosition();
		Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
		                                game.camera.offsetY, game.camera.zoom);
		Point mousePoint(mousePos.x, mousePos.y);
		FractionalHex fracHex = pixel_to_hex(layout, mousePoint);
		::Hex cubeHex = hex_round(fracHex);
		OffsetCoord offset = cube_to_offset(cubeHex);
		HexCoord hoveredHex = offsetToGameCoord(offset);

		// Only show path if hovering over a valid movement hex
		if (hoveredHex.row >= 0 && hoveredHex.row < MAP_ROWS && hoveredHex.col >= 0 && hoveredHex.col < MAP_COLS && game.map[hoveredHex.row][hoveredHex.col].isMoveSel) {
			// Get path from unit position to hovered hex
			std::vector<HexCoord> path = GameLogic::findPath(game, game.selectedUnit,
			                                                 game.selectedUnit->position,
			                                                 hoveredHex);

			if (!path.empty() && path.size() > 1) {
				// Draw path as semi-transparent hexes
				for (size_t i = 1; i < path.size(); i++) { // Start at 1 to skip unit's current position
					OffsetCoord pathOffset = gameCoordToOffset(path[i]);
					::Hex pathCube = offset_to_cube(pathOffset);
					std::vector<Point> corners = polygon_corners(layout, pathCube);

					// Draw semi-transparent yellow fill
					drawHexagon(corners, Color {255, 255, 0, 80}, true);
				}

				// Draw target hex with slightly more opacity
				OffsetCoord targetOffset = gameCoordToOffset(hoveredHex);
				::Hex targetCube = offset_to_cube(targetOffset);
				std::vector<Point> corners = polygon_corners(layout, targetCube);
				drawHexagon(corners, Color {255, 255, 0, 120}, true);
			}
		}
	}

	// Draw facing indicator in Phase 2 (after moving, selecting facing)
	if (game.selectedUnit && game.movementSel.isFacingSelection) {
		Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
		                                game.camera.offsetY, game.camera.zoom);

		// Get unit's current position (where it just moved to)
		OffsetCoord unitOffset = gameCoordToOffset(game.selectedUnit->position);
		::Hex unitCube = offset_to_cube(unitOffset);
		Point center = hex_to_pixel(layout, unitCube);

		// Get mouse position for smooth tracking
		Vector2 mousePos = GetMousePosition();
		Point mousePoint(mousePos.x, mousePos.y);

		// Draw a wide angle indicator (like ">") pointing toward mouse cursor
		// Calculate direction vector from center to mouse (smooth, not snapped)
		float dx = mousePoint.x - center.x;
		float dy = mousePoint.y - center.y;
		float len = sqrt(dx * dx + dy * dy);
		if (len > 0.1f) { // Avoid division by zero
			dx /= len;
			dy /= len;
		}

		// Perpendicular vector
		float perpX = -dy;
		float perpY = dx;

		// Make indicator longer and further from center
		float arrowSize = HEX_SIZE * game.camera.zoom * 0.65f;
		float arrowWidth = HEX_SIZE * game.camera.zoom * 0.35f;

		// Calculate tip position (pointing toward mouse, further from center)
		Point tipPos = Point(center.x + dx * arrowSize,
		                     center.y + dy * arrowSize);
		// Calculate arm endpoints (forming ">"-shape)
		Point arm1 = Point(tipPos.x - dx * arrowSize * 0.35f + perpX * arrowWidth,
		                   tipPos.y - dy * arrowSize * 0.35f + perpY * arrowWidth);
		Point arm2 = Point(tipPos.x - dx * arrowSize * 0.35f - perpX * arrowWidth,
		                   tipPos.y - dy * arrowSize * 0.35f - perpY * arrowWidth);

		// Draw the angle indicator
		DrawLineEx(Vector2 {(float)arm1.x, (float)arm1.y},
		           Vector2 {(float)tipPos.x, (float)tipPos.y},
		           4.0f * game.camera.zoom, YELLOW);
		DrawLineEx(Vector2 {(float)arm2.x, (float)arm2.y},
		           Vector2 {(float)tipPos.x, (float)tipPos.y},
		           4.0f * game.camera.zoom, YELLOW);
	}

	// Draw facing angle indicator for selected unit (when not in facing selection mode)
	if (game.selectedUnit && !game.movementSel.isFacingSelection) {
		Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
		                                game.camera.offsetY, game.camera.zoom);

		OffsetCoord unitOffset = gameCoordToOffset(game.selectedUnit->position);
		::Hex unitCube = offset_to_cube(unitOffset);
		Point center = hex_to_pixel(layout, unitCube);

		// Calculate direction vector from unit's facing angle
		float facingRad = game.selectedUnit->facing * M_PI / 180.0f;
		float dx = cos(facingRad);
		float dy = sin(facingRad);

		// Perpendicular vector
		float perpX = -dy;
		float perpY = dx;

		// Same size as movement indicator
		float arrowSize = HEX_SIZE * game.camera.zoom * 0.65f;
		float arrowWidth = HEX_SIZE * game.camera.zoom * 0.35f;

		// Calculate tip position
		Point tipPos = Point(center.x + dx * arrowSize,
		                     center.y + dy * arrowSize);
		// Calculate arm endpoints (forming ">"-shape)
		Point arm1 = Point(tipPos.x - dx * arrowSize * 0.35f + perpX * arrowWidth,
		                   tipPos.y - dy * arrowSize * 0.35f + perpY * arrowWidth);
		Point arm2 = Point(tipPos.x - dx * arrowSize * 0.35f - perpX * arrowWidth,
		                   tipPos.y - dy * arrowSize * 0.35f - perpY * arrowWidth);

		// Draw the angle indicator
		DrawLineEx(Vector2 {(float)arm1.x, (float)arm1.y},
		           Vector2 {(float)tipPos.x, (float)tipPos.y},
		           4.0f * game.camera.zoom, YELLOW);
		DrawLineEx(Vector2 {(float)arm2.x, (float)arm2.y},
		           Vector2 {(float)tipPos.x, (float)tipPos.y},
		           4.0f * game.camera.zoom, YELLOW);
	}
}

void clearSelectionHighlights(GameState &game) {
	for (int row = 0; row < MAP_ROWS; row++) {
		for (int col = 0; col < MAP_COLS; col++) {
			game.map[row][col].isMoveSel = false;
			game.map[row][col].isAttackSel = false;
		}
	}
}

} // namespace Rendering

//==============================================================================
// SECTION 7: GAME LOGIC FUNCTIONS
//==============================================================================

namespace GameLogic {

// Combat Log Functions
void addLogMessage(GameState &game, const std::string &message) {
	// Check if last message is the same (for deduplication)
	if (!game.combatLog.messages.empty()) {
		LogMessage &last = game.combatLog.messages.back();
		if (last.turn == game.currentTurn && last.message == message) {
			last.count++;
			return;
		}
	}

	// Add new message
	game.combatLog.messages.emplace_back(game.currentTurn, message);

	// Auto-scroll to bottom (most recent) by setting scroll to max
	// We'll calculate the actual max in the rendering function
	game.combatLog.scrollOffset = 999999.0f; // Large value to force scroll to bottom
}

// Hex math and distance calculations
int hexDistance(const HexCoord &a, const HexCoord &b) {
	OffsetCoord offsetA = Rendering::gameCoordToOffset(a);
	OffsetCoord offsetB = Rendering::gameCoordToOffset(b);
	::Hex cubeA = offset_to_cube(offsetA);
	::Hex cubeB = offset_to_cube(offsetB);
	return hex_distance(cubeA, cubeB);
}

// Get adjacent hex coordinates
std::vector<HexCoord> getAdjacent(int row, int col) {
	std::vector<HexCoord> result;
	OffsetCoord center(col, row);
	::Hex cubeHex = offset_to_cube(center);

	for (int i = 0; i < 6; i++) {
		::Hex neighbor = hex_neighbor(cubeHex, i);
		OffsetCoord neighborOffset = cube_to_offset(neighbor);

		if (neighborOffset.row >= 0 && neighborOffset.row < MAP_ROWS && neighborOffset.col >= 0 && neighborOffset.col < MAP_COLS) {
			result.push_back({neighborOffset.row, neighborOffset.col});
		}
	}

	return result;
}

// Check if unit is air unit (ignores ZOC)
bool isAir(const Unit *unit) {
	return unit && unit->movMethod == MovMethod::AIR;
}

// Check if unit is a hard target (armored)
bool isHardTarget(const Unit *unit) {
	if (!unit)
		return false;
	return (unit->unitClass == UnitClass::TANK || unit->unitClass == UnitClass::ANTI_TANK || unit->unitClass == UnitClass::AIR_DEFENSE);
}

// Check if unit is sea unit
bool isSea(const Unit *unit) {
	if (!unit)
		return false;
	return (unit->movMethod == MovMethod::DEEP_NAVAL || unit->movMethod == MovMethod::COSTAL || unit->movMethod == MovMethod::NAVAL);
}

// Check if unit is a reconnaissance unit (exempt from ZOC)
bool isRecon(const Unit *unit) {
	return unit && unit->unitClass == UnitClass::RECON;
}

// Set or clear ZOC for a unit
void setUnitZOC(GameState &game, Unit *unit, bool on) {
	if (!unit || isAir(unit))
		return;

	std::vector<HexCoord> adjacent = getAdjacent(unit->position.row, unit->position.col);

	for (const auto &adj : adjacent) {
		game.map[adj.row][adj.col].setZOC(unit->side, on);
	}
}

// Initialize ZOC for all units on the map
void initializeAllZOC(GameState &game) {
	// Clear all ZOC first
	for (int row = 0; row < MAP_ROWS; row++) {
		for (int col = 0; col < MAP_COLS; col++) {
			game.map[row][col].zoc[0] = 0;
			game.map[row][col].zoc[1] = 0;
		}
	}

	// Set ZOC for all units
	for (auto &unit : game.units) {
		setUnitZOC(game, unit.get(), true);
	}
}

// Get all cells within a certain range (using hex distance)
std::vector<HexCoord> getCellsInRange(int row, int col, int range) {
	std::vector<HexCoord> result;

	for (int r = 0; r < MAP_ROWS; r++) {
		for (int c = 0; c < MAP_COLS; c++) {
			HexCoord target = {r, c};
			HexCoord center = {row, col};
			int dist = hexDistance(center, target);

			if (dist <= range) {
				result.push_back(target);
			}
		}
	}

	return result;
}

// Set or clear spotting range for a unit
void setUnitSpotRange(GameState &game, Unit *unit, bool on) {
	if (!unit)
		return;

	HexCoord pos = unit->position;
	int range = unit->spotRange;
	std::vector<HexCoord> cells = getCellsInRange(pos.row, pos.col, range);

	for (const auto &cell : cells) {
		GameHex &hex = game.map[cell.row][cell.col];
		hex.setSpotted(unit->side, on);
	}
}

// Initialize spotting for all units on the map
void initializeAllSpotting(GameState &game) {
	// Clear all spotting first
	for (int row = 0; row < MAP_ROWS; row++) {
		for (int col = 0; col < MAP_COLS; col++) {
			game.map[row][col].spotted[0] = 0;
			game.map[row][col].spotted[1] = 0;
		}
	}

	// Set spotting for all units
	for (auto &unit : game.units) {
		setUnitSpotRange(game, unit.get(), true);
	}
}

// BFS pathfinding to find path from start to goal
// Returns empty vector if no path exists
std::vector<HexCoord> findPath(GameState &game, Unit *unit, const HexCoord &start, const HexCoord &goal) {
	if (!unit)
		return {};
	if (start == goal)
		return {start};

	int movMethodIdx = static_cast<int>(unit->movMethod);
	int enemySide = 1 - unit->side;
	bool ignoreZOC = isAir(unit) || isRecon(unit); // Air and recon units ignore ZOC

	// Check if unit starts in enemy ZOC (Panzer General rule: if starting in ZOC, can move one hex)
	GameHex &startHex = game.map[start.row][start.col];
	bool startingInZOC = !ignoreZOC && startHex.isZOC(enemySide);

	// BFS with parent tracking
	struct PathNode {
		HexCoord coord;
		int movementUsed; // Total movement used to reach this node
		HexCoord parent;
		bool hasParent;
	};

	std::vector<PathNode> queue;
	std::vector<PathNode> visited;

	// Start node
	queue.push_back({start, 0, {-1, -1}, false});
	visited.push_back({start, 0, {-1, -1}, false});

	while (!queue.empty()) {
		PathNode current = queue.front();
		queue.erase(queue.begin()); // Remove first element

		// Check if we reached the goal
		if (current.coord == goal) {
			// Reconstruct path
			std::vector<HexCoord> path;
			HexCoord c = goal;

			while (true) {
				path.push_back(c);
				bool found = false;
				for (const auto &v : visited) {
					if (v.coord == c && v.hasParent) {
						c = v.parent;
						found = true;
						break;
					}
				}
				if (!found)
					break;
				if (c == start) {
					path.push_back(start);
					break;
				}
			}

			// Reverse path so it goes from start to goal
			std::reverse(path.begin(), path.end());
			return path;
		}

		// Explore neighbors
		std::vector<HexCoord> adjacent = getAdjacent(current.coord.row, current.coord.col);

		for (const auto &adj : adjacent) {
			// Get terrain cost
			GameHex &hex = game.map[adj.row][adj.col];
			int terrainIdx = getTerrainIndex(hex.terrain);
			int cost = MOV_TABLE_DRY[movMethodIdx][terrainIdx];

			// Skip impassable terrain
			if (cost >= 255)
				continue;

			int newMovementUsed = current.movementUsed + cost;

			// For cost 254, it stops movement
			if (cost == 254)
				newMovementUsed = 999; // Very high cost

			// ZOC: Panzer General rules
			// - If starting in ZOC: can only move to adjacent hexes (one hex movement)
			// - If not starting in ZOC: entering ZOC adds high penalty
			// - Air and recon units ignore ZOC
			if (!ignoreZOC && hex.isZOC(enemySide) && cost < 254) {
				if (startingInZOC) {
					// Starting in ZOC: only allow movement to adjacent hexes (distance 1 from start)
					int distFromStart = hexDistance(start, adj);
					if (distFromStart > 1) {
						continue; // Can't move more than 1 hex when starting in ZOC
					}
					newMovementUsed = current.movementUsed + cost + 100; // High penalty
				} else {
					// Not starting in ZOC: entering ZOC adds penalty
					newMovementUsed = current.movementUsed + cost + 100;
				}
			}

			// Check if we can afford this movement
			if (newMovementUsed > unit->movesLeft * 2)
				continue; // Allow some extra for pathfinding flexibility

			// Check if another unit occupies this hex (unless it's the goal)
			Unit *occupant = game.getUnitAt(adj);
			if (occupant && occupant->side != unit->side && !(adj == goal))
				continue;

			// Check if we've already visited this with lower cost
			bool alreadyVisited = false;
			for (const auto &v : visited) {
				if (v.coord == adj && v.movementUsed <= newMovementUsed) {
					alreadyVisited = true;
					break;
				}
			}

			if (!alreadyVisited) {
				PathNode newNode = {adj, newMovementUsed, current.coord, true};
				queue.push_back(newNode);
				visited.push_back(newNode);
			}
		}
	}

	// No path found
	return {};
}

void highlightMovementRange(GameState &game, Unit *unit) {
	Rendering::clearSelectionHighlights(game);
	if (!unit)
		return;

	int maxRange = unit->movesLeft;
	int movMethodIdx = static_cast<int>(unit->movMethod);
	int enemySide = 1 - unit->side;
	bool ignoreZOC = isAir(unit) || isRecon(unit); // Air and recon units ignore ZOC

	// Check if unit starts in enemy ZOC (Panzer General rule: if starting in ZOC, can move one hex)
	GameHex &startHex = game.map[unit->position.row][unit->position.col];
	bool startingInZOC = !ignoreZOC && startHex.isZOC(enemySide);

	// Track cells we can reach with their remaining movement
	std::vector<std::pair<HexCoord, int>> frontier;
	std::vector<std::pair<HexCoord, int>> visited;

	// Start with unit's position
	frontier.push_back({unit->position, maxRange});
	visited.push_back({unit->position, maxRange});

	while (!frontier.empty()) {
		auto current = frontier.back();
		frontier.pop_back();

		HexCoord pos = current.first;
		int remainingMoves = current.second;

		// Check all adjacent hexes
		std::vector<HexCoord> adjacent = getAdjacent(pos.row, pos.col);

		for (const auto &adj : adjacent) {
			// Get terrain cost
			GameHex &hex = game.map[adj.row][adj.col];
			int terrainIdx = getTerrainIndex(hex.terrain);
			int cost = MOV_TABLE_DRY[movMethodIdx][terrainIdx];

			// Skip impassable terrain
			if (cost >= 255)
				continue;

			// Check if we can enter this hex
			int newRemaining = remainingMoves - cost;

			// For cost 254, we can enter but it stops us (remaining becomes 0)
			if (cost == 254)
				newRemaining = 0;

			// ZOC: Panzer General rules
			// - If starting in ZOC: can only move to adjacent hexes (one hex movement)
			// - If not starting in ZOC: entering ZOC stops movement
			// - Air and recon units ignore ZOC
			if (!ignoreZOC && hex.isZOC(enemySide) && cost < 254) {
				if (startingInZOC) {
					// Starting in ZOC: only allow movement to adjacent hexes (distance 1 from start)
					int distFromStart = hexDistance(unit->position, adj);
					if (distFromStart > 1) {
						continue; // Can't move more than 1 hex when starting in ZOC
					}
					newRemaining = 0; // Can move one hex but must stop there
				} else {
					// Not starting in ZOC: entering ZOC stops movement
					newRemaining = 0;
				}
			}

			// Skip if we can't afford to enter
			if (newRemaining < 0)
				continue;

			// Check if another unit occupies this hex
			Unit *occupant = game.getUnitAt(adj);
			if (occupant && occupant->side != unit->side)
				continue;

			// Check if we've already visited with more movement
			bool shouldUpdate = true;
			bool alreadyVisited = false;
			for (auto &v : visited) {
				if (v.first == adj) {
					alreadyVisited = true;
					if (v.second >= newRemaining) {
						shouldUpdate = false;
					} else {
						v.second = newRemaining;
					}
					break;
				}
			}

			if (!alreadyVisited) {
				visited.push_back({adj, newRemaining});
			}

			if (shouldUpdate && newRemaining > 0) {
				frontier.push_back({adj, newRemaining});
			}
		}
	}

	// Highlight all reachable cells (including starting position)
	for (const auto &v : visited) {
		game.map[v.first.row][v.first.col].isMoveSel = true;
	}
}

void highlightAttackRange(GameState &game, Unit *unit) {
	if (!unit || unit->hasFired)
		return;

	int range = 1; // Default attack range
	if (unit->unitClass == UnitClass::ARTILLERY)
		range = 3;

	for (int row = 0; row < MAP_ROWS; row++) {
		for (int col = 0; col < MAP_COLS; col++) {
			HexCoord target = {row, col};
			int dist = hexDistance(unit->position, target);

			if (dist > 0 && dist <= range) {
				Unit *occupant = game.getUnitAt(target);
				if (occupant && occupant->side != unit->side) {
					game.map[row][col].isAttackSel = true;
				}
			}
		}
	}
}

void moveUnit(GameState &game, Unit *unit, const HexCoord &target) {
	if (!unit)
		return;

	// Calculate actual movement cost based on terrain
	int movMethodIdx = static_cast<int>(unit->movMethod);
	GameHex &targetHex = game.map[target.row][target.col];
	int terrainIdx = getTerrainIndex(targetHex.terrain);
	int cost = MOV_TABLE_DRY[movMethodIdx][terrainIdx];

	// Don't move if impassable
	if (cost >= 255) {
		addLogMessage(game, "Terrain is impassable");
		return;
	}

	// For difficult terrain (cost 254), we can enter but it uses all remaining moves
	if (cost == 254)
		cost = unit->movesLeft;

	// Only move if we have enough movement points
	if (cost <= unit->movesLeft) {
		// Clear ZOC and spotting from old position
		setUnitZOC(game, unit, false);
		setUnitSpotRange(game, unit, false);

		// Store old position for fuel calculation
		HexCoord oldPos = unit->position;

		// Move unit
		unit->position = target;
		unit->movesLeft = 0; // One move per turn - all movement used up
		unit->hasMoved = true;

		// Set ZOC and spotting at new position
		setUnitZOC(game, unit, true);
		setUnitSpotRange(game, unit, true);

		// Reduce fuel by hex distance (not terrain cost)
		int distance = hexDistance(oldPos, target);
		unit->fuel = std::max(0, unit->fuel - distance);

		// Log movement
		std::string unitName = unit->name + " (" + (unit->side == 0 ? "Axis" : "Allied") + ")";
		addLogMessage(game, unitName + " moves to (" + std::to_string(target.row) + "," + std::to_string(target.col) + ")");
	} else {
		addLogMessage(game, "Not enough movement points");
	}
}

// Calculate kills using PG2 formula
int calculateKills(int atkVal, int defVal, const Unit *attacker, const Unit *defender) {
	int kF = atkVal - defVal;

	// PG2 formula: compress high values
	if (kF > 4) {
		kF = 4 + (2 * kF - 8) / 5;
	}
	kF += 6;

	// Artillery/Bomber penalty (less effective at killing)
	if (attacker->unitClass == UnitClass::ARTILLERY) {
		kF -= 3;
	}

	// Clamp kill factor between 1 and 19
	kF = std::max(1, std::min(19, kF));

	// Calculate kills based on attacker strength
	// Formula: (5 * kF * strength + 50) / 100 = (kF * strength) / 20 + 0.5
	int kills = (5 * kF * attacker->strength + 50) / 100;

	return kills;
}

void performAttack(GameState &game, Unit *attacker, Unit *defender) {
	if (!attacker || !defender)
		return;

	if (attacker->hasFired) {
		addLogMessage(game, "Unit has already fired this turn");
		return;
	}

	// Log combat initiation
	std::string attackerName = attacker->name + " (" + (attacker->side == 0 ? "Axis" : "Allied") + ")";
	std::string defenderName = defender->name + " (" + (defender->side == 0 ? "Axis" : "Allied") + ")";
	addLogMessage(game, attackerName + " attacks " + defenderName);

	// Get distance and hex information
	int distance = hexDistance(attacker->position, defender->position);
	GameHex &atkHex = game.map[attacker->position.row][attacker->position.col];
	GameHex &defHex = game.map[defender->position.row][defender->position.col];

	// Determine attack/defense values based on target type
	int aav, adv, dav, ddv;

	// Attacker attack value (use hard attack for armored targets)
	if (isHardTarget(defender)) {
		aav = attacker->hardAttack;
	} else {
		aav = attacker->softAttack;
	}

	// Attacker defense value
	adv = attacker->groundDefense;

	// Defender attack value (for return fire)
	if (isHardTarget(attacker)) {
		dav = defender->hardAttack;
	} else {
		dav = defender->softAttack;
	}

	// Defender defense value
	ddv = defender->groundDefense;

	// 1. Apply experience modifiers (+1 per experience bar)
	int aExpBars = attacker->experience / 100;
	int dExpBars = defender->experience / 100;
	aav += aExpBars;
	adv += aExpBars;
	dav += dExpBars;
	ddv += dExpBars;

	// 2. Apply entrenchment (adds to defense only)
	adv += attacker->entrenchment;
	ddv += defender->entrenchment;

	// 3. Apply terrain modifiers
	// Cities give +4 defense
	if (defHex.terrain == TerrainType::CITY) {
		ddv += 4;
	}
	if (atkHex.terrain == TerrainType::CITY) {
		adv += 4;
	}

	// Water without road: -4 defense, attacker gets +4 attack
	if (defHex.terrain == TerrainType::WATER) {
		ddv -= 4;
		aav += 4;
	}
	if (atkHex.terrain == TerrainType::WATER) {
		adv -= 4;
		dav += 4;
	}

	// 4. Apply initiative bonus (who shoots first gets advantage)
	int initDiff = attacker->initiative - defender->initiative;
	if (initDiff >= 0) {
		// Attacker has initiative
		adv += 4;                     // Attacker defense bonus
		aav += std::min(4, initDiff); // Attack bonus (max +4)
	} else {
		// Defender has initiative
		ddv += 4;                      // Defender defense bonus
		dav += std::min(4, -initDiff); // Defender attack bonus (max +4)
	}

	// 5. Apply range defense modifier (for ranged combat)
	if (distance > 1) {
		adv += attacker->rangeDefMod;
		ddv += defender->rangeDefMod;
	}

	// 6. Apply accumulated hits (reduces defense)
	adv -= attacker->hits;
	ddv -= defender->hits;

	// Calculate kills
	int kills = calculateKills(aav, ddv, attacker, defender);

	// Defender can fire back if:
	// - At range 1 (close combat), OR
	// - Both are sea units (naval combat)
	bool defCanFire = (distance <= 1 || (isSea(attacker) && isSea(defender)));
	int losses = 0;

	if (defCanFire && defender->ammo > 0) {
		losses = calculateKills(dav, adv, defender, attacker);
	}

	// Apply damage
	defender->strength = std::max(0, defender->strength - kills);
	attacker->strength = std::max(0, attacker->strength - losses);

	// Log damage
	if (kills > 0) {
		addLogMessage(game, attackerName + " deals " + std::to_string(kills) + " damage to " + defenderName);
	}
	if (losses > 0 && defCanFire) {
		addLogMessage(game, defenderName + " returns fire, dealing " + std::to_string(losses) + " damage");
	}

	// Experience gain
	// Attacker gains based on defender's attack value and kills
	int bonusAD = std::max(1, dav + 6 - adv);
	int atkExpGain = (bonusAD * (defender->maxStrength / 10) + bonusAD) * kills;
	attacker->experience = std::min(500, attacker->experience + atkExpGain);

	// Defender gains 2 * losses
	if (defCanFire) {
		int defExpGain = 2 * losses;
		defender->experience = std::min(500, defender->experience + defExpGain);
	}

	// Mark as fired and consume ammo
	attacker->hasFired = true;
	attacker->ammo = std::max(0, attacker->ammo - 1);

	// Increment hits (reduces future defense)
	attacker->hits++;
	defender->hits++;

	// Reduce entrenchment on hit
	if (kills > 0 && defender->entrenchment > 0) {
		defender->entrenchment--;
	}
	if (losses > 0 && attacker->entrenchment > 0) {
		attacker->entrenchment--;
	}

	// Clear ZOC and spotting for units about to be destroyed, and log destruction
	for (auto &unit : game.units) {
		if (unit->strength <= 0) {
			setUnitZOC(game, unit.get(), false);
			setUnitSpotRange(game, unit.get(), false);

			// Log unit destruction
			std::string unitName = unit->name + " (" + (unit->side == 0 ? "Axis" : "Allied") + ")";
			addLogMessage(game, unitName + " destroyed!");
		}
	}

	// Remove destroyed units
	game.units.erase(std::remove_if(game.units.begin(), game.units.end(),
	                                [](const std::unique_ptr<Unit> &u) {
		                                return u->strength <= 0;
	                                }),
	                 game.units.end());
}

// Entrench a unit (called at end of turn if unit didn't move)
void entrenchUnit(GameState &game, Unit *unit) {
	if (!unit)
		return;

	GameHex &hex = game.map[unit->position.row][unit->position.col];
	int terrainEntrench = getTerrainEntrenchment(hex.terrain);
	int uc = static_cast<int>(unit->unitClass);

	if (unit->entrenchment >= terrainEntrench) {
		// Slow gain above terrain level (ticks system)
		int level = unit->entrenchment - terrainEntrench;
		int nextThreshold = 9 * level + 4;
		int expBars = unit->experience / 100;

		// Add ticks based on experience, terrain, and unit type
		unit->entrenchTicks += expBars + (terrainEntrench + 1) * UNIT_ENTRENCH_RATE[uc];

		// Check if we've gained a level
		while (unit->entrenchTicks >= nextThreshold && unit->entrenchment < terrainEntrench + 5) {
			unit->entrenchTicks -= nextThreshold;
			unit->entrenchment++;
			level++;
			nextThreshold = 9 * level + 4;
		}
	} else {
		// Instant gain to terrain level
		unit->entrenchment = terrainEntrench;
		unit->entrenchTicks = 0;
	}

	// Max entrenchment is 5
	unit->entrenchment = std::min(5, unit->entrenchment);
}

void endTurn(GameState &game) {
	// Process units ending their turn
	for (auto &unit : game.units) {
		if (unit->side == game.currentPlayer) {
			// Gain entrenchment if didn't move
			if (!unit->hasMoved) {
				entrenchUnit(game, unit.get());
			} else {
				// Lost entrenchment by moving
				unit->entrenchment = 0;
				unit->entrenchTicks = 0;
			}

			// Reset hits at end of turn
			unit->hits = 0;
		}
	}

	// Log turn end
	std::string playerName = game.currentPlayer == 0 ? "Axis" : "Allied";
	addLogMessage(game, playerName + " turn ended");

	// Switch player
	game.currentPlayer = 1 - game.currentPlayer;

	// If both players have moved, advance turn
	if (game.currentPlayer == 0) {
		game.currentTurn++;
		addLogMessage(game, "--- Turn " + std::to_string(game.currentTurn) + " ---");
	}

	// Log new player turn
	playerName = game.currentPlayer == 0 ? "Axis" : "Allied";
	addLogMessage(game, playerName + " turn begins");

	// Reset actions for units about to start their turn
	for (auto &unit : game.units) {
		if (unit->side == game.currentPlayer) {
			unit->hasMoved = false;
			unit->hasFired = false;
			unit->movesLeft = unit->movementPoints;
		}
	}

	// Clear selection and movement state
	game.selectedUnit = nullptr;
	game.movementSel.reset();
	Rendering::clearSelectionHighlights(game);
}

// Calculate exact facing angle from center hex to target point
float calculateFacingFromPoint(const HexCoord &center, const Point &targetPoint, Layout &layout) {
	OffsetCoord centerOffset = Rendering::gameCoordToOffset(center);
	::Hex centerCube = offset_to_cube(centerOffset);
	Point centerPixel = hex_to_pixel(layout, centerCube);

	// Calculate direction vector from center to target
	float dx = targetPoint.x - centerPixel.x;
	float dy = targetPoint.y - centerPixel.y;

	// Calculate exact angle using atan2
	// In screen coordinates: E=0°, S=90° (y-down), W=180°, N=270°
	float angle = atan2(dy, dx) * 180.0f / M_PI;

	// Normalize angle to 0-360
	if (angle < 0)
		angle += 360.0f;

	return angle;
}

} // namespace GameLogic

//==============================================================================
// SECTION 8: INPUT AND CAMERA FUNCTIONS
//==============================================================================

namespace Input {

// Calculate centered camera offset to center the hex map in the play area
void calculateCenteredCameraOffset(CameraState &camera, int screenWidth, int screenHeight) {
	// Play area: excludes top bar (40px) and right panel (250px) and bottom bar (30px)
	float playAreaWidth = screenWidth - 250;
	float playAreaHeight = screenHeight - 70; // 40px top + 30px bottom
	float playAreaCenterX = playAreaWidth / 2.0f;
	float playAreaCenterY = 40.0f + playAreaHeight / 2.0f;

	// Calculate the center of the hex map in world coordinates
	// The map center is approximately at hex (MAP_ROWS/2, MAP_COLS/2)
	HexCoord mapCenter = {MAP_ROWS / 2, MAP_COLS / 2};
	OffsetCoord offset = Rendering::gameCoordToOffset(mapCenter);
	::Hex cubeHex = offset_to_cube(offset);

	// Create a temporary layout to calculate pixel position
	Layout tempLayout = Rendering::createHexLayout(HEX_SIZE, 0, 0, camera.zoom);
	Point mapCenterPixel = hex_to_pixel(tempLayout, cubeHex);

	// Calculate offset so that map center appears at play area center
	camera.offsetX = playAreaCenterX - mapCenterPixel.x;
	camera.offsetY = playAreaCenterY - mapCenterPixel.y;
}

} // namespace Input

// Forward declarations for Config namespace (needed by Rendering::drawOptionsMenu)
namespace Config {
void saveConfig(const VideoSettings &settings);
void loadConfig(VideoSettings &settings);
void applyGuiScale(float scale);
void loadStyleTheme(const std::string &themeName);
int getStyleIndex(const std::string &styleName);
extern std::vector<std::string> AVAILABLE_STYLES;
extern std::string STYLE_LABELS_STRING;
} // namespace Config

//==============================================================================
// SECTION 6B: RENDERING FUNCTIONS - UI AND MENUS
//==============================================================================

namespace Rendering {

void drawCombatLog(GameState &game) {
	// Get font size from raygui theme (same as dropdowns)
	const int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	const int lineSpacing = fontSize + 4;
	const float scrollBarWidth = 15.0f;
	const float padding = 10.0f;
	const int titleHeight = 25;

	Rectangle bounds = game.combatLog.bounds;

	// Get colors from raygui dropdown theme
	Color backgroundColor = GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_NORMAL));
	Color borderColor = GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_NORMAL));
	Color textColor = GetColor(GuiGetStyle(DROPDOWNBOX, TEXT_COLOR_NORMAL));
	Color titleColor = textColor; // Use same color for title

	// Draw background
	DrawRectangleRec(bounds, backgroundColor);
	DrawRectangleLinesEx(bounds, 1, borderColor); // 1px border

	// Draw title
	DrawText("Combat Log", (int)(bounds.x + padding), (int)(bounds.y + 5), 16, titleColor);

	// Calculate text area (below title, with padding, leaving space for scrollbar)
	Rectangle textArea = {
	    bounds.x + padding,
	    bounds.y + titleHeight,
	    bounds.width - (2 * padding) - scrollBarWidth,
	    bounds.height - titleHeight - padding};

	// Check if mouse is hovering over the log
	Vector2 mousePos = GetMousePosition();
	game.combatLog.isHovering = CheckCollisionPointRec(mousePos, bounds);

	// Build display lines with word wrapping
	std::vector<std::string> displayLines;
	std::vector<Color> lineColors;

	for (const auto &msg : game.combatLog.messages) {
		// Format message with turn prefix and count
		std::string prefix = "[T" + std::to_string(msg.turn) + "] ";
		std::string fullMsg = prefix + msg.message;
		if (msg.count > 1) {
			fullMsg += " (x" + std::to_string(msg.count) + ")";
		}

		// Word wrap the message
		int maxWidth = (int)textArea.width;
		std::string currentLine;
		std::istringstream words(fullMsg);
		std::string word;

		while (words >> word) {
			std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
			int textWidth = MeasureText(testLine.c_str(), fontSize);

			if (textWidth > maxWidth && !currentLine.empty()) {
				// Current line is full, save it
				displayLines.push_back(currentLine);
				lineColors.push_back(textColor);
				currentLine = word;
			} else {
				currentLine = testLine;
			}
		}

		// Add remaining text
		if (!currentLine.empty()) {
			displayLines.push_back(currentLine);
			lineColors.push_back(textColor);
		}
	}

	// Calculate total content height
	float totalContentHeight = displayLines.size() * lineSpacing;
	float visibleHeight = textArea.height;

	// Update max scroll offset
	game.combatLog.maxScrollOffset = std::max(0.0f, totalContentHeight - visibleHeight);

	// Clamp scroll offset
	game.combatLog.scrollOffset = Clamp(game.combatLog.scrollOffset, 0.0f, game.combatLog.maxScrollOffset);

	// Enable scissor mode for clipping
	BeginScissorMode((int)textArea.x, (int)textArea.y, (int)textArea.width, (int)textArea.height);

	// Draw messages from top to bottom, applying scroll offset
	int yPos = (int)(textArea.y - game.combatLog.scrollOffset);

	for (size_t i = 0; i < displayLines.size(); i++) {
		if (yPos + lineSpacing >= textArea.y && yPos <= textArea.y + textArea.height) {
			DrawText(displayLines[i].c_str(), (int)textArea.x, yPos, fontSize, lineColors[i]);
		}
		yPos += lineSpacing;
	}

	EndScissorMode();

	// Draw scrollbar if needed
	if (game.combatLog.maxScrollOffset > 0) {
		Rectangle scrollBarBounds = {
		    bounds.x + bounds.width - scrollBarWidth - padding,
		    bounds.y + titleHeight,
		    scrollBarWidth,
		    bounds.height - titleHeight - padding};

		// Get scrollbar colors from raygui theme
		Color scrollBarBg = GetColor(GuiGetStyle(SCROLLBAR, BORDER_COLOR_NORMAL));
		Color scrollBarFg = GetColor(GuiGetStyle(SCROLLBAR, BASE_COLOR_NORMAL));
		Color scrollBarBorder = GetColor(GuiGetStyle(SCROLLBAR, BORDER_COLOR_FOCUSED));

		// Calculate scroll bar handle size and position
		float handleRatio = visibleHeight / totalContentHeight;
		float handleHeight = std::max(20.0f, scrollBarBounds.height * handleRatio);
		float scrollRatio = game.combatLog.scrollOffset / game.combatLog.maxScrollOffset;
		float handleY = scrollBarBounds.y + scrollRatio * (scrollBarBounds.height - handleHeight);

		// Draw scrollbar track
		DrawRectangleRec(scrollBarBounds, scrollBarBg);

		// Draw scrollbar handle
		Rectangle handleBounds = {
		    scrollBarBounds.x,
		    handleY,
		    scrollBarWidth,
		    handleHeight};
		DrawRectangleRec(handleBounds, scrollBarFg);
		DrawRectangleLinesEx(handleBounds, 1, scrollBarBorder);
	}
}

void drawUnitInfoBox(GameState &game) {
	if (!game.selectedUnit || game.showOptionsMenu)
		return;

	// Get font size and colors from raygui dropdown theme
	const int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	const float padding = 10.0f;
	Rectangle bounds = game.unitInfoBox.bounds;

	Color backgroundColor = GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_NORMAL));
	Color borderColor = GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_NORMAL));
	Color textColor = GetColor(GuiGetStyle(DROPDOWNBOX, TEXT_COLOR_NORMAL));

	// Check if mouse is hovering over the box
	Vector2 mousePos = GetMousePosition();
	game.unitInfoBox.isHovering = CheckCollisionPointRec(mousePos, bounds);

	// Draw background
	DrawRectangleRec(bounds, backgroundColor);
	DrawRectangleLinesEx(bounds, 1, borderColor); // 1px border

	Unit *unit = game.selectedUnit;
	int y = (int)bounds.y + (int)padding;
	int x = (int)bounds.x + (int)padding;
	const int titleSize = fontSize + 4;
	const int normalSize = fontSize;

	// Use raygui's font via DrawText (DrawText uses the default font, which raygui can set)
	DrawText(unit->name.c_str(), x, y, titleSize, textColor);
	y += titleSize + 10;

	std::string info = "Strength: " + std::to_string(unit->strength) + "/" + std::to_string(unit->maxStrength);
	DrawText(info.c_str(), x, y, normalSize, textColor);
	y += normalSize + 5;

	info = "Experience: " + std::string(unit->experience, '*');
	DrawText(info.c_str(), x, y, normalSize, textColor);
	y += normalSize + 5;

	info = "Moves: " + std::to_string(unit->movesLeft) + "/" + std::to_string(unit->movementPoints);
	DrawText(info.c_str(), x, y, normalSize, textColor);
	y += normalSize + 5;

	info = "Fuel: " + std::to_string(unit->fuel);
	DrawText(info.c_str(), x, y, normalSize, textColor);
	y += normalSize + 5;

	info = "Ammo: " + std::to_string(unit->ammo);
	DrawText(info.c_str(), x, y, normalSize, textColor);
	y += normalSize + 5;

	info = "Entrenchment: " + std::to_string(unit->entrenchment);
	DrawText(info.c_str(), x, y, normalSize, textColor);
	y += normalSize + 5;

	info = "Facing: " + GameLogic::getFacingName(unit->facing);
	DrawText(info.c_str(), x, y, normalSize, textColor);
	y += normalSize + 10;

	DrawText("Stats:", x, y, normalSize, textColor);
	y += normalSize + 5;
	info = "Hard Atk: " + std::to_string(unit->hardAttack);
	DrawText(info.c_str(), x, y, normalSize - 2, textColor);
	y += normalSize;
	info = "Soft Atk: " + std::to_string(unit->softAttack);
	DrawText(info.c_str(), x, y, normalSize - 2, textColor);
	y += normalSize;
	info = "Defense: " + std::to_string(unit->groundDefense);
	DrawText(info.c_str(), x, y, normalSize - 2, textColor);
}

void drawUI(GameState &game) {
	// Turn info panel (status bar)
	DrawRectangleRec(game.layout.statusBar, Color {40, 40, 40, 240});

	std::string turnText = "Turn: " + std::to_string(game.currentTurn) + "/" + std::to_string(game.maxTurns);
	DrawText(turnText.c_str(), 10, 10, 20, WHITE);

	std::string playerText = game.currentPlayer == 0 ? "Axis" : "Allied";
	playerText = "Current: " + playerText;
	DrawText(playerText.c_str(), 200, 10, 20,
	         game.currentPlayer == 0 ? RED : BLUE);

	// Zoom indicator
	char zoomText[32];
	snprintf(zoomText, sizeof(zoomText), "Zoom: %.0f%%", game.camera.zoom * 100);
	DrawText(zoomText, 400, 10, 20, WHITE);

	// Terrain hover display (shows terrain type, coordinates, and move cost)
	Vector2 mousePos = GetMousePosition();
	Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
	                                game.camera.offsetY, game.camera.zoom);
	Point mousePoint(mousePos.x, mousePos.y);
	FractionalHex fracHex = pixel_to_hex(layout, mousePoint);
	::Hex cubeHex = hex_round(fracHex);
	OffsetCoord offset = cube_to_offset(cubeHex);
	HexCoord hoveredHex = offsetToGameCoord(offset);

	if (hoveredHex.row >= 0 && hoveredHex.row < MAP_ROWS && hoveredHex.col >= 0 && hoveredHex.col < MAP_COLS) {
		GameHex &hex = game.map[hoveredHex.row][hoveredHex.col];
		std::string terrainName = GameLogic::getTerrainName(hex.terrain);

		// Get movement cost (use selected unit's movement method if available, otherwise use TRACKED as default)
		int moveCost = 255;
		if (game.selectedUnit) {
			moveCost = GameLogic::getMovementCost(game.selectedUnit->movMethod, hex.terrain);
		} else {
			moveCost = GameLogic::getMovementCost(MovMethod::TRACKED, hex.terrain);
		}

		std::string costStr;
		if (moveCost == 255) {
			costStr = "Impassable";
		} else if (moveCost == 254) {
			costStr = "Stops";
		} else {
			costStr = std::to_string(moveCost);
		}

		char hoverText[128];
		snprintf(hoverText, sizeof(hoverText), "[%s %d,%d Move Cost: %s]",
		         terrainName.c_str(), hoveredHex.row, hoveredHex.col, costStr.c_str());
		DrawText(hoverText, 580, 10, 20, Color {255, 255, 150, 255}); // Light yellow
	}

	// Reset UI button
	if (GuiButton(Rectangle {game.layout.statusBar.width - 240, 5, 120, 30},
	              "RESET UI")) {
		// Reset camera to center and 100% zoom
		game.camera.zoom = 1.0f;
		game.camera.zoomDirection = 0;
		Input::calculateCenteredCameraOffset(game.camera, SCREEN_WIDTH, SCREEN_HEIGHT);

		// Reset combat log and unit info box positions
		game.combatLog.resetPosition();
		game.unitInfoBox.resetPosition();
	}

	// Options button
	if (GuiButton(Rectangle {game.layout.statusBar.width - 110, 5, 100, 30},
	              "OPTIONS")) {
		game.showOptionsMenu = !game.showOptionsMenu;
	}

	// Draw combat log
	drawCombatLog(game);

	// Draw unit info box
	drawUnitInfoBox(game);
}

void drawOptionsMenu(GameState &game, bool &needsRestart) {
	int menuWidth = 600;
	int menuHeight = 650;
	int menuX = (SCREEN_WIDTH - menuWidth) / 2;
	int menuY = (SCREEN_HEIGHT - menuHeight) / 2;

	// Draw background overlay
	DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color {0, 0, 0, 180});

	// Get colors from current style
	Color backgroundColor = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
	Color borderColor = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED));
	Color titleColor = GetColor(GuiGetStyle(DEFAULT, LINE_COLOR));

	// Draw menu panel
	DrawRectangle(menuX, menuY, menuWidth, menuHeight, backgroundColor);
	DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, borderColor);

	// Title
	DrawText("VIDEO OPTIONS", menuX + 20, menuY + 15, 30, titleColor);

	int y = menuY + 70;
	int labelX = menuX + 30;
	int controlX = menuX + 250;
	int controlWidth = 300;

	// Get label and text colors from style
	Color labelColor = GetColor(GuiGetStyle(LABEL, TEXT_COLOR_NORMAL));
	Color valueColor = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED));

	// Store positions for dropdowns to draw them last
	int resolutionY = y;
	y += 50;
	int fullscreenY = y;
	y += 50;
	int vsyncY = y;
	y += 50;
	int fpsY = y;
	y += 50;
	int guiScaleY = y;
	y += 50;
	int styleThemeY = y;
	y += 50;

	// Draw labels and non-dropdown controls first
	// Resolution label
	DrawText("Resolution:", labelX, resolutionY, 20, labelColor);

	// Fullscreen
	DrawText("Fullscreen:", labelX, fullscreenY, 20, labelColor);
	GuiCheckBox(Rectangle {(float)controlX, (float)fullscreenY - 5, 30, 30}, "",
	            &game.settings.fullscreen);

	// VSync
	DrawText("VSync:", labelX, vsyncY, 20, labelColor);
	GuiCheckBox(Rectangle {(float)controlX, (float)vsyncY - 5, 30, 30}, "",
	            &game.settings.vsync);

	// FPS Target label
	DrawText("FPS Target:", labelX, fpsY, 20, labelColor);
	std::string currentFps =
	    game.settings.fpsIndex == 6
	        ? "Unlimited"
	        : std::to_string(FPS_VALUES[game.settings.fpsIndex]);
	DrawText(currentFps.c_str(), controlX + controlWidth + 15, fpsY, 20, valueColor);

	// GUI Scale label
	DrawText("GUI Scale:", labelX, guiScaleY, 20, labelColor);

	// Style Theme label
	DrawText("Style Theme:", labelX, styleThemeY, 20, labelColor);

	// MSAA
	DrawText("Anti-Aliasing (4x):", labelX, y, 20, labelColor);
	bool oldMsaa = game.settings.msaa;
	GuiCheckBox(Rectangle {(float)controlX, (float)y - 5, 30, 30}, "",
	            &game.settings.msaa);
	if (game.settings.msaa != oldMsaa)
		needsRestart = true;
	y += 50;

	// Hex Size Slider
	DrawText("Hex Size:", labelX, y, 20, labelColor);
	GuiSlider(Rectangle {(float)controlX, (float)y, (float)controlWidth, 20}, "20",
	          "80", &game.settings.hexSize, 20, 80);
	std::string hexSizeStr = std::to_string((int)game.settings.hexSize);
	DrawText(hexSizeStr.c_str(), controlX + controlWidth + 15, y, 20, valueColor);
	y += 50;

	// Pan Speed Slider
	DrawText("Camera Pan Speed:", labelX, y, 20, labelColor);
	GuiSlider(Rectangle {(float)controlX, (float)y, (float)controlWidth, 20}, "1",
	          "20", &game.settings.panSpeed, 1, 20);
	std::string panSpeedStr = std::to_string((int)game.settings.panSpeed);
	DrawText(panSpeedStr.c_str(), controlX + controlWidth + 15, y, 20, valueColor);
	y += 60;

	// Buttons
	int buttonY = menuY + menuHeight - 70;
	if (GuiButton(Rectangle {(float)menuX + 30, (float)buttonY, 150, 40},
	              "Apply")) {
		// Close any open dropdowns
		game.settings.resolutionDropdownEdit = false;
		game.settings.fpsDropdownEdit = false;
		game.settings.guiScaleDropdownEdit = false;
		game.settings.styleThemeDropdownEdit = false;

		// Apply settings
		Resolution res = RESOLUTIONS[game.settings.resolutionIndex];

		if (res.width != SCREEN_WIDTH || res.height != SCREEN_HEIGHT) {
			SetWindowSize(res.width, res.height);
			SCREEN_WIDTH = res.width;
			SCREEN_HEIGHT = res.height;
			game.layout.recalculate(res.width, res.height);
			game.combatLog.recalculateBounds(res.width, res.height);
			game.unitInfoBox.recalculateBounds(res.width, res.height);
		}

		if (game.settings.fullscreen != IsWindowFullscreen()) {
			ToggleFullscreen();
			game.layout.recalculate(GetScreenWidth(), GetScreenHeight());
			game.combatLog.recalculateBounds(GetScreenWidth(), GetScreenHeight());
			game.unitInfoBox.recalculateBounds(GetScreenWidth(), GetScreenHeight());
		}

		SetTargetFPS(FPS_VALUES[game.settings.fpsIndex]);

		// Apply hex size
		HEX_SIZE = game.settings.hexSize;

		// Apply style theme
		Config::loadStyleTheme(game.settings.styleTheme);

		// Apply GUI scale (after style is loaded)
		Config::applyGuiScale(GUI_SCALE_VALUES[game.settings.guiScaleIndex]);

		// Save config to file
		Config::saveConfig(game.settings);

		// Menu stays open after applying settings
	}

	if (GuiButton(Rectangle {(float)menuX + 220, (float)buttonY, 150, 40},
	              "Cancel")) {
		// Close any open dropdowns
		game.settings.resolutionDropdownEdit = false;
		game.settings.fpsDropdownEdit = false;
		game.settings.guiScaleDropdownEdit = false;
		game.settings.styleThemeDropdownEdit = false;
		game.showOptionsMenu = false;
	}

	if (GuiButton(Rectangle {(float)menuX + 410, (float)buttonY, 150, 40},
	              "Defaults")) {
		game.settings.resolutionIndex = 6; // 1920x1080
		game.settings.fullscreen = true;
		game.settings.vsync = false;
		game.settings.fpsIndex = 6; // Unlimited FPS
		game.settings.hexSize = 40.0f;
		game.settings.panSpeed = 5.0f;
		game.settings.msaa = false;
		game.settings.guiScaleIndex = 0; // 1.0
		game.settings.styleTheme = "dark";
	}

	// Draw dropdowns last so they appear on top
	// Draw from bottom to top (Style Theme, GUI Scale, FPS, Resolution) so top dropdowns overlap bottom ones
	std::string resLabels;
	for (int i = 0; i < RESOLUTION_COUNT; i++) {
		if (i > 0)
			resLabels += ";";
		resLabels += RESOLUTIONS[i].label;
	}

	// Style Theme dropdown (draw first - bottommost)
	// Get current style index
	int currentStyleIndex = Config::getStyleIndex(game.settings.styleTheme);
	if (GuiDropdownBox(
	        Rectangle {(float)controlX, (float)styleThemeY - 5, (float)controlWidth, 30},
	        Config::STYLE_LABELS_STRING.c_str(), &currentStyleIndex, game.settings.styleThemeDropdownEdit)) {
		game.settings.styleThemeDropdownEdit = !game.settings.styleThemeDropdownEdit;
	}
	// Update style theme name if index changed
	if (currentStyleIndex >= 0 && currentStyleIndex < (int)Config::AVAILABLE_STYLES.size()) {
		game.settings.styleTheme = Config::AVAILABLE_STYLES[currentStyleIndex];
	}

	// GUI Scale dropdown
	if (GuiDropdownBox(
	        Rectangle {(float)controlX, (float)guiScaleY - 5, (float)controlWidth, 30},
	        GUI_SCALE_LABELS, &game.settings.guiScaleIndex, game.settings.guiScaleDropdownEdit)) {
		game.settings.guiScaleDropdownEdit = !game.settings.guiScaleDropdownEdit;
	}

	// FPS Target dropdown
	if (GuiDropdownBox(
	        Rectangle {(float)controlX, (float)fpsY - 5, (float)controlWidth, 30},
	        FPS_LABELS, &game.settings.fpsIndex, game.settings.fpsDropdownEdit)) {
		game.settings.fpsDropdownEdit = !game.settings.fpsDropdownEdit;
	}

	// Resolution dropdown (draw last - topmost, overlaps all others)
	if (GuiDropdownBox(
	        Rectangle {(float)controlX, (float)resolutionY - 5, (float)controlWidth, 30},
	        resLabels.c_str(), &game.settings.resolutionIndex,
	        game.settings.resolutionDropdownEdit)) {
		game.settings.resolutionDropdownEdit =
		    !game.settings.resolutionDropdownEdit;
	}

	// Restart warning
	if (needsRestart) {
		Color warningColor = GetColor(GuiGetStyle(DEFAULT, LINE_COLOR));
		DrawText("Note: MSAA requires restart to take effect", menuX + 30,
		         menuY + menuHeight - 25, 14, warningColor);
	}
}

} // namespace Rendering

//==============================================================================
// SECTION 8B: INPUT FUNCTIONS - ZOOM AND PAN HANDLERS
//==============================================================================

namespace Input {

// Handle combat log scrolling (must be called before handleZoom)
void handleCombatLogScroll(GameState &game) {
	// Only scroll if hovering over the combat log
	if (!game.combatLog.isHovering)
		return;

	float wheelMove = GetMouseWheelMove();
	if (wheelMove != 0) {
		// Scroll speed: 3 lines per wheel notch
		const float scrollSpeed = 3.0f * 18.0f; // 18 is line spacing
		game.combatLog.scrollOffset -= wheelMove * scrollSpeed;

		// Clamp to valid range
		game.combatLog.scrollOffset = Clamp(game.combatLog.scrollOffset, 0.0f, game.combatLog.maxScrollOffset);
	}
}

// Handle mouse zoom with special behavior
void handleZoom(GameState &game) {
	// Don't zoom if hovering over combat log
	if (game.combatLog.isHovering)
		return;

	float wheelMove = GetMouseWheelMove();

	if (wheelMove != 0) {
		float oldZoom = game.camera.zoom;
		int direction = wheelMove > 0 ? 1 : -1; // 1 for zoom in, -1 for zoom out

		// Check if we're continuing in the same direction or changing direction
		bool changingDirection = (game.camera.zoomDirection != 0 && game.camera.zoomDirection != direction);

		// If we're at 100% and starting to zoom
		bool startingFromNeutral = (oldZoom == 1.0f && game.camera.zoomDirection == 0);

		// If changing direction from 100%, reset the direction counter
		if (oldZoom == 1.0f && changingDirection) {
			game.camera.zoomDirection = 0;
		}

		// Determine if we should actually zoom
		bool shouldZoom = false;

		if (startingFromNeutral) {
			// Starting from 100% - zoom immediately
			shouldZoom = true;
			game.camera.zoomDirection = direction;
		} else if (oldZoom == 1.0f && game.camera.zoomDirection == direction) {
			// Second input in same direction from 100% - zoom again
			shouldZoom = true;
		} else if (oldZoom != 1.0f && changingDirection) {
			// Changing direction while not at 100% - first input goes back to 100%
			shouldZoom = true;
			game.camera.zoomDirection = 0; // Reset direction
		} else if (oldZoom != 1.0f && game.camera.zoomDirection == direction) {
			// Continuing in same direction - zoom normally
			shouldZoom = true;
		} else if (oldZoom != 1.0f && game.camera.zoomDirection == 0) {
			// After returning to 100%, first input in any direction
			shouldZoom = true;
			game.camera.zoomDirection = direction;
		}

		if (shouldZoom) {
			float newZoom = oldZoom + (direction * 0.25f);

			// Clamp zoom between 0.5 (50%) and 2.0 (200%)
			newZoom = Clamp(newZoom, 0.5f, 2.0f);

			if (newZoom != oldZoom) {
				// Get mouse position for zoom center
				Vector2 mousePos = GetMousePosition();

				// Calculate world position at mouse before zoom
				Point worldPosOld((mousePos.x - game.camera.offsetX) / oldZoom,
				                  (mousePos.y - game.camera.offsetY) / oldZoom);

				// Apply zoom
				game.camera.zoom = newZoom;

				// Calculate world position at mouse after zoom
				Point worldPosNew((mousePos.x - game.camera.offsetX) / newZoom,
				                  (mousePos.y - game.camera.offsetY) / newZoom);

				// Adjust offset to keep world position under mouse constant
				game.camera.offsetX += (worldPosNew.x - worldPosOld.x) * newZoom;
				game.camera.offsetY += (worldPosNew.y - worldPosOld.y) * newZoom;

				// Update zoom direction if we're moving away from 100%
				if (newZoom != 1.0f && game.camera.zoomDirection != direction) {
					game.camera.zoomDirection = direction;
				} else if (newZoom == 1.0f) {
					// Reset direction when we reach 100%
					game.camera.zoomDirection = 0;
				}
			}
		}
	}
}

// Handle middle mouse button panning
void handlePan(GameState &game) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
		game.camera.isPanning = true;
		game.camera.panStartMouse = GetMousePosition();
		game.camera.panStartOffset = Vector2 {game.camera.offsetX, game.camera.offsetY};
	}

	if (IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE)) {
		game.camera.isPanning = false;
	}

	if (game.camera.isPanning) {
		Vector2 currentMouse = GetMousePosition();
		Vector2 delta = {
		    currentMouse.x - game.camera.panStartMouse.x,
		    currentMouse.y - game.camera.panStartMouse.y};

		game.camera.offsetX = game.camera.panStartOffset.x + delta.x;
		game.camera.offsetY = game.camera.panStartOffset.y + delta.y;
	}
}

// Handle combat log and unit info box dragging
void handleCombatLogDrag(GameState &game) {
	Vector2 mousePos = GetMousePosition();

	// Start dragging on left click
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		// Combat log dragging
		if (CheckCollisionPointRec(mousePos, game.combatLog.bounds)) {
			game.combatLog.isDragging = true;
			game.combatLog.dragOffset.x = mousePos.x - game.combatLog.bounds.x;
			game.combatLog.dragOffset.y = mousePos.y - game.combatLog.bounds.y;
		}

		// Unit info box dragging
		if (game.selectedUnit && CheckCollisionPointRec(mousePos, game.unitInfoBox.bounds)) {
			game.unitInfoBox.isDragging = true;
			game.unitInfoBox.dragOffset.x = mousePos.x - game.unitInfoBox.bounds.x;
			game.unitInfoBox.dragOffset.y = mousePos.y - game.unitInfoBox.bounds.y;
		}
	}

	// Stop dragging on release
	if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
		game.combatLog.isDragging = false;
		game.unitInfoBox.isDragging = false;
	}

	// Update position while dragging
	if (game.combatLog.isDragging) {
		game.combatLog.bounds.x = mousePos.x - game.combatLog.dragOffset.x;
		game.combatLog.bounds.y = mousePos.y - game.combatLog.dragOffset.y;
	}

	if (game.unitInfoBox.isDragging) {
		game.unitInfoBox.bounds.x = mousePos.x - game.unitInfoBox.dragOffset.x;
		game.unitInfoBox.bounds.y = mousePos.y - game.unitInfoBox.dragOffset.y;
	}
}

} // namespace Input

//==============================================================================
// SECTION 9: CONFIGURATION - SAVE/LOAD AND SETTINGS
//==============================================================================

namespace Config {

// Save config to file
void saveConfig(const VideoSettings &settings) {
	std::ofstream configFile("config.txt");
	if (!configFile.is_open()) {
		TraceLog(LOG_WARNING, "Failed to save config.txt");
		return;
	}

	configFile << "resolutionIndex=" << settings.resolutionIndex << "\n";
	configFile << "fullscreen=" << (settings.fullscreen ? 1 : 0) << "\n";
	configFile << "vsync=" << (settings.vsync ? 1 : 0) << "\n";
	configFile << "fpsIndex=" << settings.fpsIndex << "\n";
	configFile << "hexSize=" << settings.hexSize << "\n";
	configFile << "panSpeed=" << settings.panSpeed << "\n";
	configFile << "msaa=" << (settings.msaa ? 1 : 0) << "\n";
	configFile << "guiScaleIndex=" << settings.guiScaleIndex << "\n";
	configFile << "styleTheme=" << settings.styleTheme << "\n";

	configFile.close();
	TraceLog(LOG_INFO, "Config saved to config.txt");
}

// Load config from file
void loadConfig(VideoSettings &settings) {
	std::ifstream configFile("config.txt");
	if (!configFile.is_open()) {
		TraceLog(LOG_INFO, "No config.txt found, creating default config");
		saveConfig(settings);
		return;
	}

	std::string line;
	while (std::getline(configFile, line)) {
		// Skip empty lines and comments
		if (line.empty() || line[0] == '#')
			continue;

		size_t equalPos = line.find('=');
		if (equalPos == std::string::npos)
			continue;

		std::string key = line.substr(0, equalPos);
		std::string value = line.substr(equalPos + 1);

		try {
			if (key == "resolutionIndex") {
				int val = std::stoi(value);
				if (val >= 0 && val < RESOLUTION_COUNT) {
					settings.resolutionIndex = val;
				}
			} else if (key == "fullscreen") {
				settings.fullscreen = (std::stoi(value) != 0);
			} else if (key == "vsync") {
				settings.vsync = (std::stoi(value) != 0);
			} else if (key == "fpsIndex") {
				int val = std::stoi(value);
				if (val >= 0 && val <= 6) {
					settings.fpsIndex = val;
				}
			} else if (key == "hexSize") {
				float val = std::stof(value);
				if (val >= 20.0f && val <= 80.0f) {
					settings.hexSize = val;
				}
			} else if (key == "panSpeed") {
				float val = std::stof(value);
				if (val >= 1.0f && val <= 20.0f) {
					settings.panSpeed = val;
				}
			} else if (key == "msaa") {
				settings.msaa = (std::stoi(value) != 0);
			} else if (key == "guiScaleIndex") {
				int val = std::stoi(value);
				if (val >= 0 && val < GUI_SCALE_COUNT) {
					settings.guiScaleIndex = val;
				}
			} else if (key == "styleTheme") {
				settings.styleTheme = value;
			}
		} catch (const std::exception &e) {
			// Ignore malformed values
			TraceLog(LOG_WARNING, TextFormat("Failed to parse config value: %s", key.c_str()));
		}
	}

	configFile.close();
	TraceLog(LOG_INFO, "Config loaded from config.txt");
}

// Apply GUI scale to raygui (currently disabled - functionality removed)
void applyGuiScale(float scale) {
	// GUI scaling functionality has been removed as requested
	// The menu option remains for potential future use
	TraceLog(LOG_INFO, TextFormat("GUI scale setting: %.2f (scaling disabled)", scale));
}

// Load style theme
void loadStyleTheme(const std::string &themeName) {
	std::string stylePath = "resources/styles/" + themeName + "/style_" + themeName + ".rgs";

	// Check if file exists
	std::ifstream styleFile(stylePath);
	if (!styleFile.good()) {
		TraceLog(LOG_WARNING, TextFormat("Style file not found: %s", stylePath.c_str()));
		return;
	}
	styleFile.close();

	// Load the style
	GuiLoadStyle(stylePath.c_str());
	TraceLog(LOG_INFO, TextFormat("Style loaded: %s", themeName.c_str()));
}

} // namespace Config

//==============================================================================
// SECTION 10: MAIN LOOP
//==============================================================================

int main() {
	// Discover available styles first (before window init)
	Config::discoverStyles();

	// Create temporary settings to load config before window init
	VideoSettings tempSettings;
	Config::loadConfig(tempSettings);

	// Set config flags before window creation
	unsigned int flags = FLAG_WINDOW_RESIZABLE;
	if (tempSettings.vsync) {
		flags |= FLAG_VSYNC_HINT;
	}
	if (tempSettings.msaa) {
		flags |= FLAG_MSAA_4X_HINT;
	}
	SetConfigFlags(flags);

	// Apply resolution from config
	SCREEN_WIDTH = RESOLUTIONS[tempSettings.resolutionIndex].width;
	SCREEN_HEIGHT = RESOLUTIONS[tempSettings.resolutionIndex].height;
	HEX_SIZE = tempSettings.hexSize;

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
	           "Panzer General 2 Prototype - Raylib + RayGUI");

	// Disable ESC key to exit - we use it for menu control
	SetExitKey(KEY_NULL);

	// Apply fullscreen from config
	if (tempSettings.fullscreen && !IsWindowFullscreen()) {
		ToggleFullscreen();
	}

	// Apply FPS from config
	SetTargetFPS(FPS_VALUES[tempSettings.fpsIndex]);

	// Load style theme from config
	Config::loadStyleTheme(tempSettings.styleTheme);

	// Apply GUI scale from config
	Config::applyGuiScale(GUI_SCALE_VALUES[tempSettings.guiScaleIndex]);

	GameState game;
	// Apply loaded settings to game state
	game.settings = tempSettings;

	// Center the camera on the hex map
	Input::calculateCenteredCameraOffset(game.camera, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Add some initial units
	game.addUnit(UnitClass::INFANTRY, 0, 2, 2);
	game.addUnit(UnitClass::TANK, 0, 2, 3);
	game.addUnit(UnitClass::ARTILLERY, 0, 1, 2);

	game.addUnit(UnitClass::INFANTRY, 1, 8, 10);
	game.addUnit(UnitClass::TANK, 1, 9, 10);
	game.addUnit(UnitClass::RECON, 1, 8, 11);

	// Initialize Zone of Control and Spotting for all units
	GameLogic::initializeAllZOC(game);
	GameLogic::initializeAllSpotting(game);

	// Add initial combat log messages
	GameLogic::addLogMessage(game, "=== Battle Start ===");
	GameLogic::addLogMessage(game, "Axis turn begins");

	bool needsRestart = false;

	while (!WindowShouldClose()) {
		// Input handling (only when menu is closed)
		if (!game.showOptionsMenu) {
			// Handle combat log dragging (must be first for left click priority)
			Input::handleCombatLogDrag(game);

			// Handle combat log scrolling (must be before zoom)
			Input::handleCombatLogScroll(game);

			// Handle zoom
			Input::handleZoom(game);

			// Handle middle mouse panning
			Input::handlePan(game);

			if (IsKeyPressed(KEY_SPACE)) {
				GameLogic::endTurn(game);
			}

			if (IsKeyPressed(KEY_ESCAPE)) {
				game.showOptionsMenu = true;
			}

			// Right-click handling (undo or deselect)
			if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
				if (game.movementSel.isFacingSelection && game.selectedUnit) {
					// Phase 2: Right-click undoes the movement
					// Restore unit to old position and state
					GameLogic::setUnitZOC(game, game.selectedUnit, false);
					GameLogic::setUnitSpotRange(game, game.selectedUnit, false);

					game.selectedUnit->position = game.movementSel.oldPosition;
					game.selectedUnit->movesLeft = game.movementSel.oldMovesLeft;
					game.selectedUnit->hasMoved = game.movementSel.oldHasMoved;
					game.selectedUnit->fuel = game.movementSel.oldFuel;

					GameLogic::setUnitZOC(game, game.selectedUnit, true);
					GameLogic::setUnitSpotRange(game, game.selectedUnit, true);

					// Return to Phase 1 - re-highlight movement range
					game.movementSel.reset();
					Rendering::clearSelectionHighlights(game);
					GameLogic::highlightMovementRange(game, game.selectedUnit);
				} else if (game.selectedUnit) {
					// Phase 1: Right-click deselects unit
					game.selectedUnit = nullptr;
					game.movementSel.reset();
					Rendering::clearSelectionHighlights(game);
				}
			}

			// Update facing preview in Phase 2 (after movement, selecting facing)
			if (game.selectedUnit && game.movementSel.isFacingSelection) {
				Vector2 mousePos = GetMousePosition();
				Layout layout = Rendering::createHexLayout(HEX_SIZE, game.camera.offsetX,
				                                           game.camera.offsetY, game.camera.zoom);
				Point mousePoint(mousePos.x, mousePos.y);

				// Calculate facing from unit's current position to cursor
				game.movementSel.selectedFacing = GameLogic::calculateFacingFromPoint(
				    game.selectedUnit->position, mousePoint, layout);
			}

			// Left-click handling (only if not dragging combat log or unit info box)
			if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !game.combatLog.isDragging && !game.unitInfoBox.isDragging) {
				Vector2 mousePos = GetMousePosition();

				// Convert mouse position to hex coordinate
				Layout layout = Rendering::createHexLayout(HEX_SIZE, game.camera.offsetX,
				                                           game.camera.offsetY, game.camera.zoom);
				Point mousePoint(mousePos.x, mousePos.y);
				FractionalHex fracHex = pixel_to_hex(layout, mousePoint);
				::Hex cubeHex = hex_round(fracHex);
				OffsetCoord offset = cube_to_offset(cubeHex);
				HexCoord clickedHex = Rendering::offsetToGameCoord(offset);

				// Check if clicked on a hex that's within map bounds
				if (clickedHex.row >= 0 && clickedHex.row < MAP_ROWS && clickedHex.col >= 0 && clickedHex.col < MAP_COLS) {
					Unit *clickedUnit = game.getUnitAt(clickedHex);

					// Phase 2: Unit has moved, confirming facing direction
					if (game.selectedUnit && game.movementSel.isFacingSelection) {
						// Set the facing direction
						game.selectedUnit->facing = game.movementSel.selectedFacing;

						// Reset selection state
						game.movementSel.reset();
						Rendering::clearSelectionHighlights(game);

						// Show attack range after setting facing
						if (!game.selectedUnit->hasFired) {
							GameLogic::highlightAttackRange(game, game.selectedUnit);
						}
					}
					// Phase 1: Unit selected, handle movement or attack
					else if (game.selectedUnit && !game.movementSel.isFacingSelection) {
						// Click on movement hex - execute move immediately and enter Phase 2 (facing selection)
						if (game.map[clickedHex.row][clickedHex.col].isMoveSel && !game.selectedUnit->hasMoved) {
							std::vector<HexCoord> path = GameLogic::findPath(game, game.selectedUnit,
							                                                 game.selectedUnit->position,
							                                                 clickedHex);
							if (!path.empty()) {
								// Store old state for undo
								game.movementSel.oldPosition = game.selectedUnit->position;
								game.movementSel.oldMovesLeft = game.selectedUnit->movesLeft;
								game.movementSel.oldHasMoved = game.selectedUnit->hasMoved;
								game.movementSel.oldFuel = game.selectedUnit->fuel;

								// Execute the move immediately
								GameLogic::moveUnit(game, game.selectedUnit, clickedHex);

								// Clear movement highlights
								Rendering::clearSelectionHighlights(game);

								// Enter Phase 2 - facing selection
								game.movementSel.isFacingSelection = true;
								game.movementSel.selectedFacing = game.selectedUnit->facing; // Start with current facing
							}
						}
						// Click on attack hex - perform attack
						else if (game.map[clickedHex.row][clickedHex.col].isAttackSel) {
							if (clickedUnit) {
								GameLogic::performAttack(game, game.selectedUnit, clickedUnit);
								Rendering::clearSelectionHighlights(game);
								game.selectedUnit = nullptr;
								game.movementSel.reset();
							} else {
								GameLogic::addLogMessage(game, "No target at attack location");
							}
						}
						// Click on own unit - switch selection
						else if (clickedUnit && clickedUnit->side == game.currentPlayer) {
							game.selectedUnit = clickedUnit;
							game.movementSel.reset();
							Rendering::clearSelectionHighlights(game);
							if (!clickedUnit->hasMoved) {
								GameLogic::highlightMovementRange(game, game.selectedUnit);
							}
							GameLogic::highlightAttackRange(game, game.selectedUnit);
						}
						// Click on enemy unit but not in attack range
						else if (clickedUnit && clickedUnit->side != game.currentPlayer) {
							GameLogic::addLogMessage(game, "Target not in range");
						}
						// Click on invalid hex - keep selection but provide feedback
						else {
							if (!game.map[clickedHex.row][clickedHex.col].isMoveSel && !game.map[clickedHex.row][clickedHex.col].isAttackSel) {
								GameLogic::addLogMessage(game, "Cannot move or attack there");
							}
						}
					}
					// No unit selected - try to select a unit
					else if (!game.selectedUnit) {
						if (clickedUnit && clickedUnit->side == game.currentPlayer) {
							game.selectedUnit = clickedUnit;
							game.movementSel.reset();
							if (!clickedUnit->hasMoved) {
								GameLogic::highlightMovementRange(game, game.selectedUnit);
							}
							GameLogic::highlightAttackRange(game, game.selectedUnit);
						} else if (clickedUnit && clickedUnit->side != game.currentPlayer) {
							GameLogic::addLogMessage(game, "Cannot select enemy unit");
						}
					}
				}
			}

			// Keyboard zoom controls (R = zoom in, F = zoom out)
			if (IsKeyPressed(KEY_R)) {
				float oldZoom = game.camera.zoom;
				float newZoom = oldZoom + 0.25f;
				newZoom = Clamp(newZoom, 0.5f, 2.0f);

				if (newZoom != oldZoom) {
					// Get screen center for zoom
					Vector2 centerPos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};

					// Calculate world position at center before zoom
					Point worldPosOld((centerPos.x - game.camera.offsetX) / oldZoom,
					                  (centerPos.y - game.camera.offsetY) / oldZoom);

					// Apply zoom
					game.camera.zoom = newZoom;

					// Calculate world position at center after zoom
					Point worldPosNew((centerPos.x - game.camera.offsetX) / newZoom,
					                  (centerPos.y - game.camera.offsetY) / newZoom);

					// Adjust offset to keep world position under center constant
					game.camera.offsetX += (worldPosNew.x - worldPosOld.x) * newZoom;
					game.camera.offsetY += (worldPosNew.y - worldPosOld.y) * newZoom;

					// Update zoom direction
					if (newZoom != 1.0f) {
						game.camera.zoomDirection = 1; // Zooming in
					} else {
						game.camera.zoomDirection = 0;
					}
				}
			}

			if (IsKeyPressed(KEY_F)) {
				float oldZoom = game.camera.zoom;
				float newZoom = oldZoom - 0.25f;
				newZoom = Clamp(newZoom, 0.5f, 2.0f);

				if (newZoom != oldZoom) {
					// Get screen center for zoom
					Vector2 centerPos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};

					// Calculate world position at center before zoom
					Point worldPosOld((centerPos.x - game.camera.offsetX) / oldZoom,
					                  (centerPos.y - game.camera.offsetY) / oldZoom);

					// Apply zoom
					game.camera.zoom = newZoom;

					// Calculate world position at center after zoom
					Point worldPosNew((centerPos.x - game.camera.offsetX) / newZoom,
					                  (centerPos.y - game.camera.offsetY) / newZoom);

					// Adjust offset to keep world position under center constant
					game.camera.offsetX += (worldPosNew.x - worldPosOld.x) * newZoom;
					game.camera.offsetY += (worldPosNew.y - worldPosOld.y) * newZoom;

					// Update zoom direction
					if (newZoom != 1.0f) {
						game.camera.zoomDirection = -1; // Zooming out
					} else {
						game.camera.zoomDirection = 0;
					}
				}
			}

			// Camera panning with arrow keys and WASD (absolute directions)
			float panSpeed = game.settings.panSpeed;
			if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
				game.camera.offsetX -= panSpeed; // Pan left
			if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
				game.camera.offsetX += panSpeed; // Pan right
			if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
				game.camera.offsetY -= panSpeed; // Pan up
			if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
				game.camera.offsetY += panSpeed; // Pan down
		} else {
			// Close menu with ESC (close dropdowns first if open)
			if (IsKeyPressed(KEY_ESCAPE)) {
				if (game.settings.resolutionDropdownEdit || game.settings.fpsDropdownEdit || game.settings.guiScaleDropdownEdit || game.settings.styleThemeDropdownEdit) {
					// Close any open dropdowns first
					game.settings.resolutionDropdownEdit = false;
					game.settings.fpsDropdownEdit = false;
					game.settings.guiScaleDropdownEdit = false;
					game.settings.styleThemeDropdownEdit = false;
				} else {
					// Close the menu
					game.showOptionsMenu = false;
				}
			}
		}

		// Drawing
		BeginDrawing();
		ClearBackground(COLOR_BACKGROUND);

		Rendering::drawMap(game);
		Rendering::drawUI(game);

		// Draw options menu on top
		if (game.showOptionsMenu) {
			Rendering::drawOptionsMenu(game, needsRestart);
		}

		// Draw FPS in bottom right corner
		// FPS display removed per user request

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
