#include "Rendering.h"
#include "Constants.h"
#include "GameLogic.h"
#include "Input.h"
#include "Config.h"
#include "UIPanels.h"
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"
#include "hex.h"
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

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
  Color titleColor = textColor;  // Use same color for title

  // Draw background
  DrawRectangleRec(bounds, backgroundColor);
  DrawRectangleLinesEx(bounds, 1, borderColor);  // 1px border

  // Draw title
  DrawText("Combat Log", (int)(bounds.x + padding), (int)(bounds.y + 5), 16, titleColor);

  // Calculate text area (below title, with padding, leaving space for scrollbar)
  Rectangle textArea = {
    bounds.x + padding,
    bounds.y + titleHeight,
    bounds.width - (2 * padding) - scrollBarWidth,
    bounds.height - titleHeight - padding
  };

  // Check if mouse is hovering over the log
  Vector2 mousePos = GetMousePosition();
  game.combatLog.isHovering = CheckCollisionPointRec(mousePos, bounds);

  // Build display lines with word wrapping
  std::vector<std::string> displayLines;
  std::vector<Color> lineColors;

  for (const auto& msg : game.combatLog.messages) {
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
      bounds.height - titleHeight - padding
    };

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
      handleHeight
    };
    DrawRectangleRec(handleBounds, scrollBarFg);
    DrawRectangleLinesEx(handleBounds, 1, scrollBarBorder);
  }
}

void drawUnitInfoBox(GameState &game) {
  if (!game.selectedUnit || game.showOptionsMenu) return;

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
  DrawRectangleLinesEx(bounds, 1, borderColor);  // 1px border

  Unit *unit = game.selectedUnit;
  int y = (int)bounds.y + (int)padding;
  int x = (int)bounds.x + (int)padding;
  const int titleSize = fontSize + 4;
  const int normalSize = fontSize;

  // Use raygui's font via DrawText (DrawText uses the default font, which raygui can set)
  DrawText(unit->name.c_str(), x, y, titleSize, textColor);
  y += titleSize + 10;

  std::string info = "Health: " + std::to_string(unit->getOverallHealthPercent()) + "%";
  DrawText(info.c_str(), x, y, normalSize, textColor);
  y += normalSize + 5;

  const LocationStatus& ct = unit->locations.at(ArmorLocation::CENTER_TORSO);
  info = "CT Armor: " + std::to_string(ct.currentArmor) + "/" +
         std::to_string(ct.maxArmor);
  DrawText(info.c_str(), x, y, normalSize - 2, textColor);
  y += normalSize;

  info = "CT Structure: " + std::to_string(ct.currentStructure) + "/" +
         std::to_string(ct.maxStructure);
  DrawText(info.c_str(), x, y, normalSize - 2, textColor);
  y += normalSize + 5;

  info = "Moves: " + std::to_string(unit->movesLeft) + "/" +
         std::to_string(unit->movementPoints);
  DrawText(info.c_str(), x, y, normalSize, textColor);
  y += normalSize + 5;

  info = "Facing: " + GameLogic::getFacingName(unit->facing);
  DrawText(info.c_str(), x, y, normalSize, textColor);
  y += normalSize + 10;

}

void drawUI(GameState &game) {
  // Turn info panel (status bar)
  DrawRectangleRec(game.layout.statusBar, Color{40, 40, 40, 240});

  std::string turnText = "Turn: " + std::to_string(game.currentTurn) + "/" +
                         std::to_string(game.maxTurns);
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

  if (hoveredHex.row >= 0 && hoveredHex.row < MAP_ROWS &&
      hoveredHex.col >= 0 && hoveredHex.col < MAP_COLS) {
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
    DrawText(hoverText, 580, 10, 20, Color{255, 255, 150, 255}); // Light yellow
  }

  // Reset UI button
  if (GuiButton(Rectangle{game.layout.statusBar.width - 240, 5, 120, 30},
                "RESET UI")) {
    // Reset camera to center and 100% zoom
    game.camera.zoom = 1.0f;
    game.camera.zoomDirection = 0;
    Input::calculateCenteredCameraOffset(game.camera, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Reset combat log and unit info box positions
    game.combatLog.resetPosition();
    game.unitInfoBox.resetPosition();

    // Reset paperdoll panel positions
    UIPanel::resetPanelPositions(game);
  }

  // Options button
  if (GuiButton(Rectangle{game.layout.statusBar.width - 110, 5, 100, 30},
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
  DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 180});

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
  GuiCheckBox(Rectangle{(float)controlX, (float)fullscreenY - 5, 30, 30}, "",
              &game.settings.fullscreen);

  // VSync
  DrawText("VSync:", labelX, vsyncY, 20, labelColor);
  GuiCheckBox(Rectangle{(float)controlX, (float)vsyncY - 5, 30, 30}, "",
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
  GuiCheckBox(Rectangle{(float)controlX, (float)y - 5, 30, 30}, "",
              &game.settings.msaa);
  if (game.settings.msaa != oldMsaa)
    needsRestart = true;
  y += 50;

  // Hex Size Slider
  DrawText("Hex Size:", labelX, y, 20, labelColor);
  GuiSlider(Rectangle{(float)controlX, (float)y, (float)controlWidth, 20}, "20",
            "80", &game.settings.hexSize, 20, 80);
  std::string hexSizeStr = std::to_string((int)game.settings.hexSize);
  DrawText(hexSizeStr.c_str(), controlX + controlWidth + 15, y, 20, valueColor);
  y += 50;

  // Pan Speed Slider
  DrawText("Camera Pan Speed:", labelX, y, 20, labelColor);
  GuiSlider(Rectangle{(float)controlX, (float)y, (float)controlWidth, 20}, "1",
            "20", &game.settings.panSpeed, 1, 20);
  std::string panSpeedStr = std::to_string((int)game.settings.panSpeed);
  DrawText(panSpeedStr.c_str(), controlX + controlWidth + 15, y, 20, valueColor);
  y += 60;

  // Buttons
  int buttonY = menuY + menuHeight - 70;
  if (GuiButton(Rectangle{(float)menuX + 30, (float)buttonY, 150, 40},
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

  if (GuiButton(Rectangle{(float)menuX + 220, (float)buttonY, 150, 40},
                "Cancel")) {
    // Close any open dropdowns
    game.settings.resolutionDropdownEdit = false;
    game.settings.fpsDropdownEdit = false;
    game.settings.guiScaleDropdownEdit = false;
    game.settings.styleThemeDropdownEdit = false;
    game.showOptionsMenu = false;
  }

  if (GuiButton(Rectangle{(float)menuX + 410, (float)buttonY, 150, 40},
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
          Rectangle{(float)controlX, (float)styleThemeY - 5, (float)controlWidth, 30},
          Config::STYLE_LABELS_STRING.c_str(), &currentStyleIndex, game.settings.styleThemeDropdownEdit)) {
    game.settings.styleThemeDropdownEdit = !game.settings.styleThemeDropdownEdit;
  }
  // Update style theme name if index changed
  if (currentStyleIndex >= 0 && currentStyleIndex < (int)Config::AVAILABLE_STYLES.size()) {
    game.settings.styleTheme = Config::AVAILABLE_STYLES[currentStyleIndex];
  }

  // GUI Scale dropdown
  if (GuiDropdownBox(
          Rectangle{(float)controlX, (float)guiScaleY - 5, (float)controlWidth, 30},
          GUI_SCALE_LABELS, &game.settings.guiScaleIndex, game.settings.guiScaleDropdownEdit)) {
    game.settings.guiScaleDropdownEdit = !game.settings.guiScaleDropdownEdit;
  }

  // FPS Target dropdown
  if (GuiDropdownBox(
          Rectangle{(float)controlX, (float)fpsY - 5, (float)controlWidth, 30},
          FPS_LABELS, &game.settings.fpsIndex, game.settings.fpsDropdownEdit)) {
    game.settings.fpsDropdownEdit = !game.settings.fpsDropdownEdit;
  }

  // Resolution dropdown (draw last - topmost, overlaps all others)
  if (GuiDropdownBox(
          Rectangle{(float)controlX, (float)resolutionY - 5, (float)controlWidth, 30},
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
