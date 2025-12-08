#ifndef OPENWANZER_GAME_STATE_HPP
#define OPENWANZER_GAME_STATE_HPP

#include "CombatArcs.hpp"
#include "GameHex.hpp"
#include "HexCoord.hpp"
#include "MechLoadout.hpp"
#include "Raylib.hpp"
#include "Unit.hpp"

#include <memory>
#include <string>
#include <vector>

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
	bool zoomLocked; // When true, mouse wheel and keyboard zoom are disabled
	Vector2 panStartMouse;
	Vector2 panStartOffset;

	CameraState()
	    : offsetX(0.0f), offsetY(0.0f), zoom(1.0f), zoomDirection(0), isPanning(false), zoomLocked(false), panStartMouse {0, 0}, panStartOffset {0, 0} {
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
	bool resolutionDropdownEdit;
	bool fpsDropdownEdit;

	// Combat text animation timing (in seconds)
	// Defaults based on 30fps YouTube video: 5 frames = 0.17s, 10 frames = 0.33s
	float combatTextFadeInTime; // Duration of fade-in phase
	float combatTextFloatTime;  // Duration of float-up + fade-out phase
	float combatTextFloatSpeed; // Pixels per second during float (30fps * 1px/frame = 30px/s)

	VideoSettings()
	    : resolutionIndex(6), fullscreen(true), vsync(false), fpsIndex(6), hexSize(65.0f), resolutionDropdownEdit(false), fpsDropdownEdit(false), combatTextFadeInTime(0.17f), combatTextFloatTime(0.33f), combatTextFloatSpeed(30.0f) {
	}
};

// Game Layout Structure
struct GameLayout {
	Rectangle statusBar; // Top status bar
	Rectangle unitPanel; // Right unit info panel
	Rectangle helpBar;   // Bottom help text area
	Rectangle playArea;  // Map rendering area

	void recalculate(int w, int h);

	GameLayout();
};

// Resolution options
struct Resolution {
	int width;
	int height;
	const char *label;
};

extern const Resolution RESOLUTIONS[];
extern const int RESOLUTION_COUNT;

extern const int FPS_VALUES[];
extern const char *FPS_LABELS;

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

	CombatLog();

	void recalculateBounds(int screenWidth, int screenHeight);

	void resetPosition();
};

// Unit info box structure (upper right panel for selected unit)
struct UnitInfoBox {
	Rectangle bounds;        // Display area
	bool isHovering;         // Is mouse hovering over box?
	bool isDragging;         // Is being dragged?
	Vector2 dragOffset;      // Offset from top-left corner to mouse when drag started
	Rectangle initialBounds; // Initial position for reset

	UnitInfoBox();

	void recalculateBounds(int screenWidth, int screenHeight);

	void resetPosition();
};

// Movement selection state (for two-phase selection system)
// Phase 1: Select unit, show movement range
// Phase 2: Unit has moved, selecting facing direction
struct MovementSelection {
	bool isFacingSelection; // Phase 2: unit has moved, now selecting facing
	HexCoord oldPosition;   // Position before move (for undo)
	int oldMovesLeft;       // Movement points before move (for undo)
	bool oldHasMoved;       // hasMoved state before move (for undo)
	float selectedFacing;   // The facing direction being previewed (0-360 degrees)

	MovementSelection()
	    : isFacingSelection(false), oldPosition {-1, -1}, oldMovesLeft(0), oldHasMoved(false), selectedFacing(0.0f) {
	}

	void reset();
};

// Attack line structure for visualizing firing lines
struct AttackLine {
	HexCoord from;
	HexCoord to;
	combatarcs::AttackArc arc;
	bool outOfRange;

	AttackLine(HexCoord f, HexCoord t, combatarcs::AttackArc a, bool oor = false)
	    : from(f), to(t), arc(a), outOfRange(oor) {
	}
};

// Paperdoll Panel structures for simplified 5-box cross layout
// Layout (top-down view):
//         [FRONT]
//     [LEFT][CENTER][RIGHT]
//         [REAR]
struct PaperdollPanel {
	Rectangle bounds; // Panel position and size
	bool isVisible;
	bool isDragging;
	Vector2 dragOffset;
	Vector2 defaultPosition; // For reset functionality

	// 5-box cross paperdoll regions (for hover detection)
	Rectangle boxFront;
	Rectangle boxLeft;
	Rectangle boxCenter;
	Rectangle boxRight;
	Rectangle boxRear;

	// Current tooltip state
	ArmorLocation hoveredLocation;
	bool showTooltip;
	Vector2 tooltipPos;

	// Flash overlay state (for hit animation)
	ArmorLocation flashLocation;
	int flashFrame; // 0-9: frames 0-4 darken, 5-9 lighten; -1 = no flash
	static constexpr int FLASH_TOTAL_FRAMES = 10;

	PaperdollPanel()
	    : isVisible(false), isDragging(false), hoveredLocation(ArmorLocation::NONE), showTooltip(false), flashLocation(ArmorLocation::NONE), flashFrame(-1) {
		bounds = {0, 0, 0, 0};
		dragOffset = {0, 0};
		defaultPosition = {0, 0};
		boxFront = boxLeft = boxCenter = boxRight = boxRear = {0, 0, 0, 0};
		tooltipPos = {0, 0};
	}

	void startFlash(ArmorLocation location) {
		flashLocation = location;
		flashFrame = 0;
	}

	void updateFlash() {
		if (flashFrame >= 0) {
			flashFrame++;
			if (flashFrame >= FLASH_TOTAL_FRAMES) {
				flashFrame = -1;
				flashLocation = ArmorLocation::NONE;
			}
		}
	}

	// Returns alpha for black overlay (0 = no overlay, 255 = full black)
	unsigned char getFlashAlpha() const {
		if (flashFrame < 0)
			return 0;

		// Frames 0-4: darken (0 to 255)
		// Frames 5-9: lighten (255 to 0)
		if (flashFrame < 5) {
			return (unsigned char)((flashFrame + 1) * 255 / 5);
		} else {
			return (unsigned char)(255 - ((flashFrame - 4) * 255 / 5));
		}
	}
};

struct TargetPanel : public PaperdollPanel {
	Unit *targetUnit;
	combatarcs::AttackArc currentArc; // For red line indicators

	TargetPanel()
	    : targetUnit(nullptr), currentArc(combatarcs::AttackArc::FRONT) {
	}
};

struct PlayerPanel : public PaperdollPanel {
	Unit *playerUnit;

	PlayerPanel()
	    : playerUnit(nullptr) {
	}
};

// Combat text for floating damage numbers
struct CombatText {
	HexCoord spawnHex; // Hex where text was spawned (for camera-relative positioning)
	Vector2 hexOffset; // Random offset relative to hex center (calculated once at spawn)
	Vector2 position;  // Current screen position (recalculated each frame)
	std::string text;
	Color color;
	float currentTime; // Current time elapsed in animation (seconds)
	float floatOffset; // Current vertical float offset (pixels, in world space)
	bool isStructure;  // Orange for structure, white for armor/miss

	// Animation timing (in seconds) - set from config
	float fadeInTime; // Duration of fade-in phase
	float floatTime;  // Duration of float-up + fade-out phase
	float floatSpeed; // Pixels per second during float

	CombatText(HexCoord hex, Vector2 offset, const std::string &txt, bool structure, float fadeIn = 0.17f, float floatDur = 0.33f, float speed = 30.0f)
	    : spawnHex(hex), hexOffset(offset), position {0, 0}, text(txt), currentTime(0.0f), floatOffset(0.0f), isStructure(structure), fadeInTime(fadeIn), floatTime(floatDur), floatSpeed(speed) {
		color = structure ? ORANGE : WHITE;
	}

	float getTotalTime() const {
		return fadeInTime + floatTime;
	}

	bool isFinished() const {
		return currentTime >= getTotalTime();
	}

	void update(float deltaTime) {
		currentTime += deltaTime;

		// Update float offset during float phase
		if (currentTime > fadeInTime) {
			float floatElapsed = currentTime - fadeInTime;
			floatOffset = floatElapsed * floatSpeed;
		}
	}

	unsigned char getAlpha() const {
		if (currentTime < fadeInTime) {
			// Fade in: 0 to 255 over fadeInTime
			float progress = currentTime / fadeInTime;
			return (unsigned char)(progress * 255.0f);
		} else {
			// Fade out: 255 to 0 over floatTime
			float floatElapsed = currentTime - fadeInTime;
			float progress = floatElapsed / floatTime;
			progress = (progress > 1.0f) ? 1.0f : progress;
			return (unsigned char)((1.0f - progress) * 255.0f);
		}
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
	bool showMechbayScreen;
	bool mechbayFilterFocused; // Is the MechBay inventory filter focused?
	VideoSettings settings;
	GameLayout layout;
	CameraState camera;
	CombatLog combatLog;
	UnitInfoBox unitInfoBox;
	MovementSelection movementSel;       // Two-phase selection state
	std::vector<AttackLine> attackLines; // Active attack lines to display
	bool showAttackLines;                // Whether to show attack lines
	TargetPanel targetPanel;             // HBS-style target mech panel
	PlayerPanel playerPanel;             // HBS-style player mech panel
	std::vector<CombatText> combatTexts; // Floating damage numbers

	// MechBay loadout management
	std::unique_ptr<mechloadout::MechLoadout> mechLoadout;

	GameState();

	void initializeMap();
	void initializeMechBay(); // Initialize MechBay with mock data

	Unit *getUnitAt(const HexCoord &coord);

	void addUnit(UnitClass uClass, int side, int row, int col);
};

#endif // OPENWANZER_GAME_STATE_HPP
