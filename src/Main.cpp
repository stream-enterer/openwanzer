//==============================================================================
// OPEN WANZER - Turn-based Tactical Mech Combat Game
//==============================================================================

#include "Raylib.hpp"
#include "Raymath.hpp"

// Suppress warnings from raygui.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#define RAYGUI_IMPLEMENTATION
#include "Raygui.hpp"
#pragma GCC diagnostic pop

#include "CherryStyle.hpp"
#include "Config.hpp"
#include "Constants.hpp"
#include "GameLogic.hpp"
#include "GameState.hpp"
#include "Hex.hpp"
#include "Input.hpp"
#include "MechBayUI.hpp"
#include "PaperdollUI.hpp"
#include "Rendering.hpp"
#include "UIPanels.hpp"

int main() {
	// Create temporary settings to load config before window init
	VideoSettings tempSettings;
	config::loadConfig(tempSettings);

	// Set config flags before window creation
	unsigned int flags = FLAG_WINDOW_RESIZABLE;
	if (tempSettings.vsync) {
		flags |= FLAG_VSYNC_HINT;
	}
	SetConfigFlags(flags);

	// Apply resolution from config
	SCREEN_WIDTH = RESOLUTIONS[tempSettings.resolutionIndex].width;
	SCREEN_HEIGHT = RESOLUTIONS[tempSettings.resolutionIndex].height;
	HEX_SIZE = tempSettings.hexSize;

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
	           "Open Wanzer - Tactical Mech Combat");

	// Disable ESC key to exit - we use it for menu control
	SetExitKey(KEY_NULL);

	// Apply fullscreen from config
	if (tempSettings.fullscreen && !IsWindowFullscreen()) {
		ToggleFullscreen();
	}

	// Apply FPS from config
	SetTargetFPS(FPS_VALUES[tempSettings.fpsIndex]);

	// Initialize Cherry UI style
	cherrystyle::InitializeCherryStyle();

	GameState game;
	// Apply loaded settings to game state
	game.settings = tempSettings;

	// Center the camera on the hex map
	input::calculateCenteredCameraOffset(game.camera, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Add some initial units (BattleTech mech weight classes)
	game.addUnit(UnitClass::LIGHT, 0, 2, 2);
	game.addUnit(UnitClass::MEDIUM, 0, 2, 3);
	game.addUnit(UnitClass::HEAVY, 0, 1, 2);

	game.addUnit(UnitClass::LIGHT, 1, 8, 10);
	game.addUnit(UnitClass::MEDIUM, 1, 9, 10);
	game.addUnit(UnitClass::ASSAULT, 1, 8, 11);

	// Set one mech per side to have 0 armor (but full structure) for testing
	// This tests the orange structure pattern display
	if (game.units.size() >= 1) {
		// First unit (side 0) - strip all armor
		Unit* unit0 = game.units[0].get();
		for (auto& loc : unit0->locations) {
			loc.second.currentArmor = 0;
		}
	}
	if (game.units.size() >= 4) {
		// Fourth unit (side 1) - strip all armor
		Unit* unit1 = game.units[3].get();
		for (auto& loc : unit1->locations) {
			loc.second.currentArmor = 0;
		}
	}

	// Initialize Spotting for all units
	gamelogic::initializeAllSpotting(game);

	// Initialize paperdoll panels
	uipanel::initializeTargetPanel(game);
	uipanel::initializePlayerPanel(game);

	// Add initial combat log messages
	gamelogic::addLogMessage(game, "=== Battle Start ===");
	gamelogic::addLogMessage(game, "Axis turn begins");

	bool needsRestart = false;

	while (!WindowShouldClose()) {
		// Input handling (only when menus are closed)
		if (!game.showOptionsMenu && !game.showMechbayScreen) {
			// Handle paperdoll panel dragging (must be before selection)
			paperdollui::handlePaperdollPanelDrag(game);

			// Handle combat log dragging (must be first for left click priority)
			input::handleCombatLogDrag(game);

			// Handle paperdoll tooltips
			paperdollui::handlePaperdollTooltips(game);

			// Handle combat log scrolling (must be before zoom)
			input::handleCombatLogScroll(game);

			// Handle zoom
			input::handleZoom(game);

			// Handle middle mouse panning
			input::handlePan(game);

			if (IsKeyPressed(KEY_SPACE)) {
				gamelogic::endTurn(game);
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
					rendering::clearSelectionHighlights(game);
					gamelogic::highlightMovementRange(game, game.selectedUnit);

					// Update attack lines for returned position
					gamelogic::updateAttackLines(game);
					game.showAttackLines = true;
				} else if (game.selectedUnit) {
					// Phase 1: deselect
					game.selectedUnit = nullptr;
					game.movementSel.reset();
					rendering::clearSelectionHighlights(game);
					game.showAttackLines = false;
					// Hide panels on deselect
					uipanel::hideTargetPanel(game);
					uipanel::hidePlayerPanel(game);
				}
			}

			// Update facing preview in Phase 2
			if (game.selectedUnit && game.movementSel.isFacingSelection) {
				Vector2 mousePos = GetMousePosition();
				Layout layout = rendering::createHexLayout(HEX_SIZE, game.camera.offsetX,
				                                           game.camera.offsetY, game.camera.zoom);
				Point mousePoint(mousePos.x, mousePos.y);

				game.movementSel.selectedFacing = gamelogic::calculateFacingFromPoint(
				    game.selectedUnit->position, mousePoint, layout);

				// Update attack lines based on preview facing
				gamelogic::updateAttackLines(game);
				game.showAttackLines = true;
			}

			// Left-click handling (skip if interacting with draggable UI elements)
			Vector2 clickMousePos = GetMousePosition();
			bool clickedPaperdoll = (game.targetPanel.isVisible && CheckCollisionPointRec(clickMousePos, game.targetPanel.bounds)) || (game.playerPanel.isVisible && CheckCollisionPointRec(clickMousePos, game.playerPanel.bounds));
			if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !game.combatLog.isDragging && !game.unitInfoBox.isDragging && !game.targetPanel.isDragging && !game.playerPanel.isDragging && !clickedPaperdoll) {
				Vector2 mousePos = GetMousePosition();
				Layout layout = rendering::createHexLayout(HEX_SIZE, game.camera.offsetX,
				                                           game.camera.offsetY, game.camera.zoom);
				Point mousePoint(mousePos.x, mousePos.y);
				FractionalHex fracHex = PixelToHex(layout, mousePoint);
				::Hex cubeHex = HexRound(fracHex);
				OffsetCoord offset = CubeToOffset(cubeHex);
				HexCoord clickedHex = rendering::offsetToGameCoord(offset);

				if (clickedHex.row >= 0 && clickedHex.row < MAP_ROWS && clickedHex.col >= 0 && clickedHex.col < MAP_COLS) {
					Unit* clickedUnit = game.getUnitAt(clickedHex);

					// Phase 2: confirming facing
					if (game.selectedUnit && game.movementSel.isFacingSelection) {
						game.selectedUnit->facing = game.movementSel.selectedFacing;

						// Now that movement is confirmed, update spotting (clear old, set new)
						gamelogic::setSpotRangeAtPosition(game, game.selectedUnit->side,
						                                  game.selectedUnit->spotRange,
						                                  game.movementSel.oldPosition, false);
						gamelogic::setUnitSpotRange(game, game.selectedUnit, true);

						game.movementSel.reset();
						rendering::clearSelectionHighlights(game);
						if (!game.selectedUnit->hasFired) {
							gamelogic::highlightAttackRange(game, game.selectedUnit);
						}

						// Keep attack lines visible after confirming facing
						gamelogic::updateAttackLines(game);
						game.showAttackLines = true;
					}
					// Phase 1: movement or attack
					else if (game.selectedUnit && !game.movementSel.isFacingSelection) {
						if (game.map[clickedHex.row][clickedHex.col].isMoveSel && !game.selectedUnit->hasMoved) {
							std::vector<HexCoord> path = gamelogic::findPath(game, game.selectedUnit,
							                                                 game.selectedUnit->position,
							                                                 clickedHex);
							if (!path.empty()) {
								game.movementSel.oldPosition = game.selectedUnit->position;
								game.movementSel.oldMovesLeft = game.selectedUnit->movesLeft;
								game.movementSel.oldHasMoved = game.selectedUnit->hasMoved;

								// Move without updating spotting (defer until facing confirmation)
								gamelogic::moveUnit(game, game.selectedUnit, clickedHex, false);
								rendering::clearSelectionHighlights(game);
								game.movementSel.isFacingSelection = true;
								game.movementSel.selectedFacing = game.selectedUnit->facing;
							}
						} else if (game.map[clickedHex.row][clickedHex.col].isAttackSel) {
							if (clickedUnit) {
								// Show target panel during attack
								Layout attackLayout = rendering::createHexLayout(HEX_SIZE, game.camera.offsetX,
								                                                 game.camera.offsetY, game.camera.zoom);
								OffsetCoord atkOffset = rendering::gameCoordToOffset(game.selectedUnit->position);
								OffsetCoord defOffset = rendering::gameCoordToOffset(clickedUnit->position);
								::Hex atkCube = OffsetToCube(atkOffset);
								::Hex defCube = OffsetToCube(defOffset);
								Point atkPixel = HexToPixel(attackLayout, atkCube);
								Point defPixel = HexToPixel(attackLayout, defCube);
								Vector2 atkPos = {(float)atkPixel.x, (float)atkPixel.y};
								Vector2 defPos = {(float)defPixel.x, (float)defPixel.y};
								combatarcs::AttackArc attackArc = combatarcs::getAttackArc(atkPos, defPos, clickedUnit->facing);
								uipanel::showTargetPanel(game, clickedUnit, attackArc);

								gamelogic::performAttack(game, game.selectedUnit, clickedUnit);
								rendering::clearSelectionHighlights(game);
								game.selectedUnit = nullptr;
								game.movementSel.reset();
								game.showAttackLines = false;
								// Keep target panel visible after attack (don't hide it)
								uipanel::hidePlayerPanel(game);
							}
						} else if (clickedUnit) {
							game.selectedUnit = clickedUnit;
							game.movementSel.reset();
							rendering::clearSelectionHighlights(game);

							// Only show movement/attack highlights and targeting lines for friendly units
							if (clickedUnit->side == game.currentPlayer) {
								if (!clickedUnit->hasMoved) {
									gamelogic::highlightMovementRange(game, game.selectedUnit);
								}
								gamelogic::highlightAttackRange(game, game.selectedUnit);

								// Update attack lines for selected unit
								gamelogic::updateAttackLines(game);
								game.showAttackLines = true;

								// Show player panel for friendly unit
								uipanel::showPlayerPanel(game, clickedUnit);
								uipanel::hideTargetPanel(game);
							} else {
								// Enemy unit selected - show target panel
								// Calculate attack arc from current selected unit to this enemy
								if (game.selectedUnit) {
									Layout layout = rendering::createHexLayout(HEX_SIZE, game.camera.offsetX,
									                                           game.camera.offsetY, game.camera.zoom);
									OffsetCoord attackerOffset = rendering::gameCoordToOffset(game.selectedUnit->position);
									OffsetCoord defenderOffset = rendering::gameCoordToOffset(clickedUnit->position);
									::Hex attackerCube = OffsetToCube(attackerOffset);
									::Hex defenderCube = OffsetToCube(defenderOffset);
									Point attackerPixel = HexToPixel(layout, attackerCube);
									Point defenderPixel = HexToPixel(layout, defenderCube);
									Vector2 attackerPos = {(float)attackerPixel.x, (float)attackerPixel.y};
									Vector2 defenderPos = {(float)defenderPixel.x, (float)defenderPixel.y};
									combatarcs::AttackArc arc = combatarcs::getAttackArc(attackerPos, defenderPos, clickedUnit->facing);
									uipanel::showTargetPanel(game, clickedUnit, arc);
								}
								game.showAttackLines = false;
								uipanel::hidePlayerPanel(game);
							}
						} else {
							// Clicked empty hex - hide panels
							uipanel::hideTargetPanel(game);
							uipanel::hidePlayerPanel(game);
						}
					} else if (!game.selectedUnit) {
						if (clickedUnit) {
							game.selectedUnit = clickedUnit;
							game.movementSel.reset();

							// Only show movement/attack highlights and targeting lines for friendly units
							if (clickedUnit->side == game.currentPlayer) {
								if (!clickedUnit->hasMoved) {
									gamelogic::highlightMovementRange(game, game.selectedUnit);
								}
								gamelogic::highlightAttackRange(game, game.selectedUnit);

								// Update attack lines for selected unit
								gamelogic::updateAttackLines(game);
								game.showAttackLines = true;

								// Show player panel for friendly unit
								uipanel::showPlayerPanel(game, clickedUnit);
								uipanel::hideTargetPanel(game);
							} else {
								// Enemy unit selected - show target panel
								// Default to FRONT arc when no attacker selected
								combatarcs::AttackArc arc = combatarcs::AttackArc::FRONT;
								uipanel::showTargetPanel(game, clickedUnit, arc);
								game.showAttackLines = false;
								uipanel::hidePlayerPanel(game);
							}
						} else {
							// Clicked empty hex - hide panels
							uipanel::hideTargetPanel(game);
							uipanel::hidePlayerPanel(game);
						}
					}
				}
			}

			// Keyboard zoom (only if not locked)
			if (!game.camera.zoomLocked && IsKeyPressed(KEY_R)) {
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

			if (!game.camera.zoomLocked && IsKeyPressed(KEY_F)) {
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

			// Camera panning (hardcoded speed)
			const float panSpeed = 1.0f;
			if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
				game.camera.offsetX += panSpeed;
			if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
				game.camera.offsetX -= panSpeed;
			if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
				game.camera.offsetY += panSpeed;
			if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
				game.camera.offsetY -= panSpeed;
		} else if (game.showMechbayScreen) {
			// MechBay screen input
			// Only close MechBay on ESC if filter is not focused
			// (filter handles ESC internally when focused)
			if (IsKeyPressed(KEY_ESCAPE) && !game.mechbayFilterFocused) {
				game.showMechbayScreen = false;
			}
		} else {
			// Options menu input
			if (IsKeyPressed(KEY_ESCAPE)) {
				if (game.settings.resolutionDropdownEdit || game.settings.fpsDropdownEdit) {
					game.settings.resolutionDropdownEdit = false;
					game.settings.fpsDropdownEdit = false;
				} else {
					game.showOptionsMenu = false;
				}
			}
		}

		// Update combat texts
		gamelogic::updateCombatTexts(game, GetFrameTime());

		// Update panel flash animations
		paperdollui::updatePanelFlashes(game);

		// Drawing
		BeginDrawing();
		ClearBackground(kColorBackground);

		rendering::drawMap(game);
		rendering::drawUI(game);

		// Draw combat texts (floating damage numbers)
		rendering::drawCombatTexts(game);

		// Draw paperdoll panels
		paperdollui::renderTargetPanel(game);
		paperdollui::renderPlayerPanel(game);

		// Draw modal screens (on top of everything)
		if (game.showMechbayScreen) {
			mechbayui::RenderMechBayScreen(game);
		}

		if (game.showOptionsMenu) {
			rendering::drawOptionsMenu(game, needsRestart);
		}

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
