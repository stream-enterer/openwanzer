#include "CherryStyle.hpp"
#include "Config.hpp"
#include "Constants.hpp"
#include "GameLogic.hpp"
#include "Hex.hpp"
#include "Input.hpp"
#include "Raygui.hpp"
#include "Raylib.hpp"
#include "Raymath.hpp"
#include "Rendering.hpp"
#include "UIPanels.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace rendering {

void drawCombatLog(GameState &game) {
	// Use fixed font size from Cherry style
	const int fontSize = cherrystyle::kFontSize;
	const int lineSpacing = cherrystyle::kTextLineSpacing;
	const float scrollBarWidth = 15.0f;
	const float padding = 10.0f;
	const int titleHeight = 30;

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
	float spacing = (float)cherrystyle::kFontSpacing;
	DrawTextEx(cherrystyle::CHERRY_FONT, "Combat Log", Vector2 {bounds.x + padding, bounds.y + 8}, (float)fontSize, spacing, titleColor);

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
			int textWidth = (int)MeasureTextEx(cherrystyle::CHERRY_FONT, testLine.c_str(), (float)fontSize, spacing).x;

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
			DrawTextEx(cherrystyle::CHERRY_FONT, displayLines[i].c_str(), Vector2 {textArea.x, (float)yPos}, (float)fontSize, spacing, lineColors[i]);
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

	// Use fixed font size from Cherry style
	const int fontSize = cherrystyle::kFontSize;
	const float padding = 10.0f;
	const int lineSpacing = 18;
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
	float spacing = (float)cherrystyle::kFontSpacing;

	// Draw unit name
	DrawTextEx(cherrystyle::CHERRY_FONT, unit->name.c_str(), Vector2 {(float)x, (float)y}, (float)fontSize, spacing, textColor);
	y += fontSize + 12;

	std::string info = "Health: " + std::to_string(unit->getOverallHealthPercent()) + "%";
	DrawTextEx(cherrystyle::CHERRY_FONT, info.c_str(), Vector2 {(float)x, (float)y}, (float)fontSize, spacing, textColor);
	y += lineSpacing;

	const LocationStatus &ct = unit->locations.at(ArmorLocation::CENTER_TORSO);
	info = "CT Armor: " + std::to_string(ct.currentArmor) + "/" + std::to_string(ct.maxArmor);
	DrawTextEx(cherrystyle::CHERRY_FONT, info.c_str(), Vector2 {(float)x, (float)y}, (float)fontSize, spacing, textColor);
	y += lineSpacing;

	info = "CT Structure: " + std::to_string(ct.currentStructure) + "/" + std::to_string(ct.maxStructure);
	DrawTextEx(cherrystyle::CHERRY_FONT, info.c_str(), Vector2 {(float)x, (float)y}, (float)fontSize, spacing, textColor);
	y += lineSpacing + 4;

	info = "Moves: " + std::to_string(unit->movesLeft) + "/" + std::to_string(unit->movementPoints);
	DrawTextEx(cherrystyle::CHERRY_FONT, info.c_str(), Vector2 {(float)x, (float)y}, (float)fontSize, spacing, textColor);
	y += lineSpacing;

	info = "Facing: " + gamelogic::getFacingName(unit->facing);
	DrawTextEx(cherrystyle::CHERRY_FONT, info.c_str(), Vector2 {(float)x, (float)y}, (float)fontSize, spacing, textColor);
}

void drawUI(GameState &game) {
	// Turn info panel (status bar)
	DrawRectangleRec(game.layout.statusBar, Color {40, 40, 40, 240});

	const int fontSize = cherrystyle::kFontSize;
	float spacing = (float)cherrystyle::kFontSpacing;

	std::string turnText = "Turn: " + std::to_string(game.currentTurn) + "/" + std::to_string(game.maxTurns);
	DrawTextEx(cherrystyle::CHERRY_FONT, turnText.c_str(), Vector2 {10, 12}, (float)fontSize, spacing, WHITE);

	std::string playerText = game.currentPlayer == 0 ? "Axis" : "Allied";
	playerText = "Current: " + playerText;
	DrawTextEx(cherrystyle::CHERRY_FONT, playerText.c_str(), Vector2 {200, 12}, (float)fontSize, spacing,
	           game.currentPlayer == 0 ? RED : BLUE);

	// Zoom indicator
	char zoomText[32];
	snprintf(zoomText, sizeof(zoomText), "Zoom: %.0f%%", game.camera.zoom * 100);
	DrawTextEx(cherrystyle::CHERRY_FONT, zoomText, Vector2 {400, 12}, (float)fontSize, spacing, WHITE);

	// Terrain hover display (shows terrain type, coordinates, and move cost)
	Vector2 mousePos = GetMousePosition();
	Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
	                                game.camera.offsetY, game.camera.zoom);
	Point mousePoint(mousePos.x, mousePos.y);
	FractionalHex fracHex = PixelToHex(layout, mousePoint);
	::Hex cubeHex = HexRound(fracHex);
	OffsetCoord offset = CubeToOffset(cubeHex);
	HexCoord hoveredHex = offsetToGameCoord(offset);

	if (hoveredHex.row >= 0 && hoveredHex.row < MAP_ROWS && hoveredHex.col >= 0 && hoveredHex.col < MAP_COLS) {
		GameHex &hex = game.map[hoveredHex.row][hoveredHex.col];
		std::string terrainName = gamelogic::getTerrainName(hex.terrain);

		// Get movement cost (use selected unit's movement method if available, otherwise use TRACKED as default)
		int moveCost = 255;
		if (game.selectedUnit) {
			moveCost = gamelogic::getMovementCost(game.selectedUnit->movMethod, hex.terrain);
		} else {
			moveCost = gamelogic::getMovementCost(MovMethod::TRACKED, hex.terrain);
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
		DrawTextEx(cherrystyle::CHERRY_FONT, hoverText, Vector2 {580, 12}, (float)fontSize, spacing, Color {255, 255, 150, 255}); // Light yellow
	}

	// Reset UI button (moved further left to make room)
	if (GuiButton(Rectangle {game.layout.statusBar.width - 370, 5, 120, 30},
	              "RESET UI")) {
		// Reset camera to center and 100% zoom
		game.camera.zoom = 1.0f;
		game.camera.zoomDirection = 0;
		input::calculateCenteredCameraOffset(game.camera, SCREEN_WIDTH, SCREEN_HEIGHT);

		// Reset combat log and unit info box positions
		game.combatLog.resetPosition();
		game.unitInfoBox.resetPosition();

		// Reset paperdoll panel positions
		uipanel::resetPanelPositions(game);
	}

	// Mechbay button
	if (GuiButton(Rectangle {game.layout.statusBar.width - 240, 5, 120, 30},
	              "MECHBAY")) {
		game.showMechbayScreen = !game.showMechbayScreen;
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

void drawOptionsMenu(GameState &game, [[maybe_unused]] bool &needsRestart) {
	int menuWidth = 600;
	int menuHeight = 450; // Reduced height since we removed options
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
	DrawText("VIDEO OPTIONS", menuX + 20, menuY + 15, 20, titleColor);

	int y = menuY + 60;
	int labelX = menuX + 30;
	int controlX = menuX + 250;
	int controlWidth = 300;

	// Get label and text colors from style
	Color labelColor = GetColor(GuiGetStyle(LABEL, TEXT_COLOR_NORMAL));
	Color valueColor = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED));

	// Store positions for dropdowns to draw them last
	int resolutionY = y;
	y += 40;
	int fullscreenY = y;
	y += 40;
	int vsyncY = y;
	y += 40;
	int fpsY = y;
	y += 40;

	// Draw labels and non-dropdown controls first
	// Resolution label
	DrawText("Resolution:", labelX, resolutionY, 15, labelColor);

	// Fullscreen
	DrawText("Fullscreen:", labelX, fullscreenY, 15, labelColor);
	GuiCheckBox(Rectangle {(float)controlX, (float)fullscreenY - 5, 25, 25}, "",
	            &game.settings.fullscreen);

	// VSync
	DrawText("VSync:", labelX, vsyncY, 15, labelColor);
	GuiCheckBox(Rectangle {(float)controlX, (float)vsyncY - 5, 25, 25}, "",
	            &game.settings.vsync);

	// FPS Target label
	DrawText("FPS Target:", labelX, fpsY, 15, labelColor);
	std::string currentFps =
	    game.settings.fpsIndex == 6
	        ? "Unlimited"
	        : std::to_string(FPS_VALUES[game.settings.fpsIndex]);
	DrawText(currentFps.c_str(), controlX + controlWidth + 15, fpsY, 15, valueColor);

	// Hex Size Slider
	DrawText("Hex Size:", labelX, y, 15, labelColor);
	GuiSlider(Rectangle {(float)controlX, (float)y, (float)controlWidth, 20}, "20",
	          "80", &game.settings.hexSize, 20, 80);
	std::string hexSizeStr = std::to_string((int)game.settings.hexSize);
	DrawText(hexSizeStr.c_str(), controlX + controlWidth + 15, y, 15, valueColor);
	y += 40;

	// Pan Speed Slider
	DrawText("Camera Pan Speed:", labelX, y, 15, labelColor);
	GuiSlider(Rectangle {(float)controlX, (float)y, (float)controlWidth, 20}, "1",
	          "20", &game.settings.panSpeed, 1, 20);
	std::string panSpeedStr = std::to_string((int)game.settings.panSpeed);
	DrawText(panSpeedStr.c_str(), controlX + controlWidth + 15, y, 15, valueColor);

	// Buttons
	int buttonY = menuY + menuHeight - 60;
	if (GuiButton(Rectangle {(float)menuX + 30, (float)buttonY, 150, 35},
	              "Apply")) {
		// Close any open dropdowns
		game.settings.resolutionDropdownEdit = false;
		game.settings.fpsDropdownEdit = false;

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

		// Save config to file
		config::saveConfig(game.settings);

		// Menu stays open after applying settings
	}

	if (GuiButton(Rectangle {(float)menuX + 220, (float)buttonY, 150, 35},
	              "Cancel")) {
		// Close any open dropdowns
		game.settings.resolutionDropdownEdit = false;
		game.settings.fpsDropdownEdit = false;
		game.showOptionsMenu = false;
	}

	if (GuiButton(Rectangle {(float)menuX + 410, (float)buttonY, 150, 35},
	              "Defaults")) {
		game.settings.resolutionIndex = 6; // 1920x1080
		game.settings.fullscreen = true;
		game.settings.vsync = false;
		game.settings.fpsIndex = 6; // Unlimited FPS
		game.settings.hexSize = 40.0f;
		game.settings.panSpeed = 5.0f;
	}

	// Draw dropdowns last so they appear on top
	std::string resLabels;
	for (int i = 0; i < RESOLUTION_COUNT; i++) {
		if (i > 0)
			resLabels += ";";
		resLabels += RESOLUTIONS[i].label;
	}

	// FPS Target dropdown
	if (GuiDropdownBox(
	        Rectangle {(float)controlX, (float)fpsY - 5, (float)controlWidth, 25},
	        FPS_LABELS, &game.settings.fpsIndex, game.settings.fpsDropdownEdit)) {
		game.settings.fpsDropdownEdit = !game.settings.fpsDropdownEdit;
	}

	// Resolution dropdown (draw last - topmost, overlaps all others)
	if (GuiDropdownBox(
	        Rectangle {(float)controlX, (float)resolutionY - 5, (float)controlWidth, 25},
	        resLabels.c_str(), &game.settings.resolutionIndex,
	        game.settings.resolutionDropdownEdit)) {
		game.settings.resolutionDropdownEdit =
		    !game.settings.resolutionDropdownEdit;
	}
}

} // namespace rendering
