//==============================================================================
// PANZER GENERAL 2 PROTOTYPE - MULTI-FILE ARCHITECTURE
//==============================================================================

#include "raylib.h"
#include "raymath.h"

// Suppress warnings from raygui.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#pragma GCC diagnostic pop

#include "hex.h"

// Include all module headers
#include "core/constants.h"
#include "core/game_state.h"
#include "rendering/rendering.h"
#include "rendering/paperdoll_ui.h"
#include "game_logic/game_logic.h"
#include "input/input.h"
#include "config/config.h"
#include "ui/ui_panels.h"

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

  // Add some initial units (BattleTech mech weight classes)
  game.addUnit(UnitClass::LIGHT, 0, 2, 2);
  game.addUnit(UnitClass::MEDIUM, 0, 2, 3);
  game.addUnit(UnitClass::HEAVY, 0, 1, 2);

  game.addUnit(UnitClass::LIGHT, 1, 8, 10);
  game.addUnit(UnitClass::MEDIUM, 1, 9, 10);
  game.addUnit(UnitClass::ASSAULT, 1, 8, 11);

  // Initialize Spotting for all units
  GameLogic::initializeAllSpotting(game);

  // Initialize paperdoll panels
  UIPanel::initializeTargetPanel(game);
  UIPanel::initializePlayerPanel(game);

  // Add initial combat log messages
  GameLogic::addLogMessage(game, "=== Battle Start ===");
  GameLogic::addLogMessage(game, "Axis turn begins");

  bool needsRestart = false;

  while (!WindowShouldClose()) {
    // Input handling (only when menu is closed)
    if (!game.showOptionsMenu) {
      // Handle paperdoll panel dragging (must be before selection)
      PaperdollUI::handlePaperdollPanelDrag(game);

      // Handle combat log dragging (must be first for left click priority)
      Input::handleCombatLogDrag(game);

      // Handle paperdoll tooltips
      PaperdollUI::handlePaperdollTooltips(game);

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
          // Note: spotting was never updated during tentative move, so no need to clear it

          game.selectedUnit->position = game.movementSel.oldPosition;
          game.selectedUnit->movesLeft = game.movementSel.oldMovesLeft;
          game.selectedUnit->hasMoved = game.movementSel.oldHasMoved;

          // Note: spotting is still at old position from before the tentative move

          // Return to Phase 1
          game.movementSel.reset();
          Rendering::clearSelectionHighlights(game);
          GameLogic::highlightMovementRange(game, game.selectedUnit);

          // Update attack lines for returned position
          GameLogic::updateAttackLines(game);
          game.showAttackLines = true;
        } else if (game.selectedUnit) {
          // Phase 1: deselect
          game.selectedUnit = nullptr;
          game.movementSel.reset();
          Rendering::clearSelectionHighlights(game);
          game.showAttackLines = false;
          // Hide panels on deselect
          UIPanel::hideTargetPanel(game);
          UIPanel::hidePlayerPanel(game);
        }
      }

      // Update facing preview in Phase 2
      if (game.selectedUnit && game.movementSel.isFacingSelection) {
        Vector2 mousePos = GetMousePosition();
        Layout layout = Rendering::createHexLayout(HEX_SIZE, game.camera.offsetX,
                                       game.camera.offsetY, game.camera.zoom);
        Point mousePoint(mousePos.x, mousePos.y);

        game.movementSel.selectedFacing = GameLogic::calculateFacingFromPoint(
            game.selectedUnit->position, mousePoint, layout);

        // Update attack lines based on preview facing
        GameLogic::updateAttackLines(game);
        game.showAttackLines = true;
      }

      // Left-click handling
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !game.combatLog.isDragging && !game.unitInfoBox.isDragging) {
        Vector2 mousePos = GetMousePosition();
        Layout layout = Rendering::createHexLayout(HEX_SIZE, game.camera.offsetX,
                                       game.camera.offsetY, game.camera.zoom);
        Point mousePoint(mousePos.x, mousePos.y);
        FractionalHex fracHex = pixel_to_hex(layout, mousePoint);
        ::Hex cubeHex = hex_round(fracHex);
        OffsetCoord offset = cube_to_offset(cubeHex);
        HexCoord clickedHex = Rendering::offsetToGameCoord(offset);

        if (clickedHex.row >= 0 && clickedHex.row < MAP_ROWS &&
            clickedHex.col >= 0 && clickedHex.col < MAP_COLS) {

          Unit *clickedUnit = game.getUnitAt(clickedHex);

          // Phase 2: confirming facing
          if (game.selectedUnit && game.movementSel.isFacingSelection) {
            game.selectedUnit->facing = game.movementSel.selectedFacing;

            // Now that movement is confirmed, update spotting (clear old, set new)
            GameLogic::setSpotRangeAtPosition(game, game.selectedUnit->side,
                                               game.selectedUnit->spotRange,
                                               game.movementSel.oldPosition, false);
            GameLogic::setUnitSpotRange(game, game.selectedUnit, true);

            game.movementSel.reset();
            Rendering::clearSelectionHighlights(game);
            if (!game.selectedUnit->hasFired) {
              GameLogic::highlightAttackRange(game, game.selectedUnit);
            }

            // Keep attack lines visible after confirming facing
            GameLogic::updateAttackLines(game);
            game.showAttackLines = true;
          }
          // Phase 1: movement or attack
          else if (game.selectedUnit && !game.movementSel.isFacingSelection) {
            if (game.map[clickedHex.row][clickedHex.col].isMoveSel && !game.selectedUnit->hasMoved) {
              std::vector<HexCoord> path = GameLogic::findPath(game, game.selectedUnit,
                                                               game.selectedUnit->position,
                                                               clickedHex);
              if (!path.empty()) {
                game.movementSel.oldPosition = game.selectedUnit->position;
                game.movementSel.oldMovesLeft = game.selectedUnit->movesLeft;
                game.movementSel.oldHasMoved = game.selectedUnit->hasMoved;

                // Move without updating spotting (defer until facing confirmation)
                GameLogic::moveUnit(game, game.selectedUnit, clickedHex, false);
                Rendering::clearSelectionHighlights(game);
                game.movementSel.isFacingSelection = true;
                game.movementSel.selectedFacing = game.selectedUnit->facing;
              }
            } else if (game.map[clickedHex.row][clickedHex.col].isAttackSel) {
              if (clickedUnit) {
                GameLogic::performAttack(game, game.selectedUnit, clickedUnit);
                Rendering::clearSelectionHighlights(game);
                game.selectedUnit = nullptr;
                game.movementSel.reset();
                game.showAttackLines = false;
                // Hide panels after attack
                UIPanel::hideTargetPanel(game);
                UIPanel::hidePlayerPanel(game);
              }
            } else if (clickedUnit) {
              game.selectedUnit = clickedUnit;
              game.movementSel.reset();
              Rendering::clearSelectionHighlights(game);

              // Only show movement/attack highlights and targeting lines for friendly units
              if (clickedUnit->side == game.currentPlayer) {
                if (!clickedUnit->hasMoved) {
                  GameLogic::highlightMovementRange(game, game.selectedUnit);
                }
                GameLogic::highlightAttackRange(game, game.selectedUnit);

                // Update attack lines for selected unit
                GameLogic::updateAttackLines(game);
                game.showAttackLines = true;

                // Show player panel for friendly unit
                UIPanel::showPlayerPanel(game, clickedUnit);
                UIPanel::hideTargetPanel(game);
              } else {
                // Enemy unit selected - show target panel
                // Calculate attack arc from current selected unit to this enemy
                if (game.selectedUnit) {
                  Layout layout = Rendering::createHexLayout(HEX_SIZE, game.camera.offsetX,
                                                             game.camera.offsetY, game.camera.zoom);
                  OffsetCoord attackerOffset = Rendering::gameCoordToOffset(game.selectedUnit->position);
                  OffsetCoord defenderOffset = Rendering::gameCoordToOffset(clickedUnit->position);
                  ::Hex attackerCube = offset_to_cube(attackerOffset);
                  ::Hex defenderCube = offset_to_cube(defenderOffset);
                  Point attackerPixel = hex_to_pixel(layout, attackerCube);
                  Point defenderPixel = hex_to_pixel(layout, defenderCube);
                  Vector2 attackerPos = {(float)attackerPixel.x, (float)attackerPixel.y};
                  Vector2 defenderPos = {(float)defenderPixel.x, (float)defenderPixel.y};
                  CombatArcs::AttackArc arc = CombatArcs::getAttackArc(attackerPos, defenderPos, clickedUnit->facing);
                  UIPanel::showTargetPanel(game, clickedUnit, arc);
                }
                game.showAttackLines = false;
                UIPanel::hidePlayerPanel(game);
              }
            } else {
              // Clicked empty hex - hide panels
              UIPanel::hideTargetPanel(game);
              UIPanel::hidePlayerPanel(game);
            }
          } else if (!game.selectedUnit) {
            if (clickedUnit) {
              game.selectedUnit = clickedUnit;
              game.movementSel.reset();

              // Only show movement/attack highlights and targeting lines for friendly units
              if (clickedUnit->side == game.currentPlayer) {
                if (!clickedUnit->hasMoved) {
                  GameLogic::highlightMovementRange(game, game.selectedUnit);
                }
                GameLogic::highlightAttackRange(game, game.selectedUnit);

                // Update attack lines for selected unit
                GameLogic::updateAttackLines(game);
                game.showAttackLines = true;

                // Show player panel for friendly unit
                UIPanel::showPlayerPanel(game, clickedUnit);
                UIPanel::hideTargetPanel(game);
              } else {
                // Enemy unit selected - show target panel
                Layout layout = Rendering::createHexLayout(HEX_SIZE, game.camera.offsetX,
                                                           game.camera.offsetY, game.camera.zoom);
                OffsetCoord defenderOffset = Rendering::gameCoordToOffset(clickedUnit->position);
                ::Hex defenderCube = offset_to_cube(defenderOffset);
                Point defenderPixel = hex_to_pixel(layout, defenderCube);
                Vector2 defenderPos = {(float)defenderPixel.x, (float)defenderPixel.y};
                // Default to FRONT arc when no attacker selected
                CombatArcs::AttackArc arc = CombatArcs::AttackArc::FRONT;
                UIPanel::showTargetPanel(game, clickedUnit, arc);
                game.showAttackLines = false;
                UIPanel::hidePlayerPanel(game);
              }
            } else {
              // Clicked empty hex - hide panels
              UIPanel::hideTargetPanel(game);
              UIPanel::hidePlayerPanel(game);
            }
          }
        }
      }

      // Keyboard zoom
      if (IsKeyPressed(KEY_R)) {
        float oldZoom = game.camera.zoom;
        float newZoom = Clamp(oldZoom + 0.25f, 0.5f, 2.0f);
        if (newZoom != oldZoom) {
          Vector2 centerPos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
          Point worldPosOld((centerPos.x - game.camera.offsetX) / oldZoom,
                           (centerPos.y - game.camera.offsetY) / oldZoom);
          game.camera.zoom = newZoom;
          Point worldPosNew((centerPos.x - game.camera.offsetX) / newZoom,
                           (centerPos.y - game.camera.offsetY) / newZoom);
          game.camera.offsetX += (worldPosNew.x - worldPosOld.x) * newZoom;
          game.camera.offsetY += (worldPosNew.y - worldPosOld.y) * newZoom;
          game.camera.zoomDirection = (newZoom != 1.0f) ? 1 : 0;
        }
      }

      if (IsKeyPressed(KEY_F)) {
        float oldZoom = game.camera.zoom;
        float newZoom = Clamp(oldZoom - 0.25f, 0.5f, 2.0f);
        if (newZoom != oldZoom) {
          Vector2 centerPos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
          Point worldPosOld((centerPos.x - game.camera.offsetX) / oldZoom,
                           (centerPos.y - game.camera.offsetY) / oldZoom);
          game.camera.zoom = newZoom;
          Point worldPosNew((centerPos.x - game.camera.offsetX) / newZoom,
                           (centerPos.y - game.camera.offsetY) / newZoom);
          game.camera.offsetX += (worldPosNew.x - worldPosOld.x) * newZoom;
          game.camera.offsetY += (worldPosNew.y - worldPosOld.y) * newZoom;
          game.camera.zoomDirection = (newZoom != 1.0f) ? -1 : 0;
        }
      }

      // Camera panning
      float panSpeed = game.settings.panSpeed;
      if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) game.camera.offsetX -= panSpeed;
      if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) game.camera.offsetX += panSpeed;
      if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) game.camera.offsetY -= panSpeed;
      if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) game.camera.offsetY += panSpeed;
    } else {
      // Options menu input
      if (IsKeyPressed(KEY_ESCAPE)) {
        if (game.settings.resolutionDropdownEdit ||
            game.settings.fpsDropdownEdit ||
            game.settings.guiScaleDropdownEdit ||
            game.settings.styleThemeDropdownEdit) {
          game.settings.resolutionDropdownEdit = false;
          game.settings.fpsDropdownEdit = false;
          game.settings.guiScaleDropdownEdit = false;
          game.settings.styleThemeDropdownEdit = false;
        } else {
          game.showOptionsMenu = false;
        }
      }
    }

    // Drawing
    BeginDrawing();
    ClearBackground(COLOR_BACKGROUND);

    Rendering::drawMap(game);
    Rendering::drawUI(game);

    // Draw paperdoll panels
    PaperdollUI::renderTargetPanel(game);
    PaperdollUI::renderPlayerPanel(game);

    if (game.showOptionsMenu) {
      Rendering::drawOptionsMenu(game, needsRestart);
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
