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

	VideoSettings()
	    : resolutionIndex(6), fullscreen(true), vsync(false), fpsIndex(6), hexSize(65.0f), resolutionDropdownEdit(false), fpsDropdownEdit(false) {
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

// Paperdoll Panel structures for HBS-style mech UI
struct PaperdollPanel {
	Rectangle bounds; // Panel position and size
	bool isVisible;
	bool isDragging;
	Vector2 dragOffset;
	Vector2 defaultPosition; // For reset functionality

	// Paperdoll regions (for hover detection)
	Rectangle frontHead;
	Rectangle frontCT, frontLT, frontRT;
	Rectangle frontLA, frontRA;
	Rectangle frontLL, frontRL;

	Rectangle rearCT, rearLT, rearRT;
	Rectangle rearLA, rearRA; // Blackened, non-interactive

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
		frontHead = frontCT = frontLT = frontRT = {0, 0, 0, 0};
		frontLA = frontRA = frontLL = frontRL = {0, 0, 0, 0};
		rearCT = rearLT = rearRT = rearLA = rearRA = {0, 0, 0, 0};
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
	Vector2 position;     // Current screen position
	Vector2 basePosition; // Starting position (for animation)
	std::string text;
	Color color;
	int currentFrame; // Current frame in animation
	bool isStructure; // Orange for structure, white for armor/miss

	// Animation phases:
	// Frames 0-4: Fade in (5 frames)
	// Frames 5-14: Float up + fade out (10 frames)
	static constexpr int FADE_IN_FRAMES = 5;
	static constexpr int FLOAT_FRAMES = 10;
	static constexpr int TOTAL_FRAMES = FADE_IN_FRAMES + FLOAT_FRAMES;

	CombatText(Vector2 pos, const std::string &txt, bool structure)
	    : position(pos), basePosition(pos), text(txt), currentFrame(0), isStructure(structure) {
		color = structure ? ORANGE : WHITE;
	}

	bool isFinished() const {
		return currentFrame >= TOTAL_FRAMES;
	}

	void update() {
		currentFrame++;

		// Float up during float phase (1 pixel per frame)
		if (currentFrame > FADE_IN_FRAMES) {
			position.y = basePosition.y - (currentFrame - FADE_IN_FRAMES);
		}
	}

	unsigned char getAlpha() const {
		if (currentFrame < FADE_IN_FRAMES) {
			// Fade in: 0 to 255 over 5 frames
			return (unsigned char)((currentFrame + 1) * 255 / FADE_IN_FRAMES);
		} else {
			// Fade out: 255 to 0 over 10 frames
			int floatFrame = currentFrame - FADE_IN_FRAMES;
			return (unsigned char)(255 - (floatFrame * 255 / FLOAT_FRAMES));
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
