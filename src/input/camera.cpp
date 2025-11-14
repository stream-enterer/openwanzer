#include "input.h"
#include "../rendering/rendering.h"
#include "../core/constants.h"
#include "raylib.h"
#include "raymath.h"
#include "hex.h"
#include <cmath>

namespace Input {

// Calculate centered camera offset to center the hex map in the play area
void calculateCenteredCameraOffset(CameraState& camera, int screenWidth, int screenHeight) {
  // Play area: excludes top bar (40px) and right panel (250px) and bottom bar (30px)
  float playAreaWidth = screenWidth - 250;
  float playAreaHeight = screenHeight - 70;  // 40px top + 30px bottom
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

// Handle combat log scrolling (must be called before handleZoom)
void handleCombatLogScroll(GameState &game) {
  // Only scroll if hovering over the combat log
  if (!game.combatLog.isHovering) return;

  float wheelMove = GetMouseWheelMove();
  if (wheelMove != 0) {
    // Scroll speed: 3 lines per wheel notch
    const float scrollSpeed = 3.0f * 18.0f;  // 18 is line spacing
    game.combatLog.scrollOffset -= wheelMove * scrollSpeed;

    // Clamp to valid range
    game.combatLog.scrollOffset = Clamp(game.combatLog.scrollOffset, 0.0f, game.combatLog.maxScrollOffset);
  }
}

// Handle mouse zoom with special behavior
void handleZoom(GameState &game) {
  // Don't zoom if hovering over combat log
  if (game.combatLog.isHovering) return;

  float wheelMove = GetMouseWheelMove();

  if (wheelMove != 0) {
    float oldZoom = game.camera.zoom;
    int direction = wheelMove > 0 ? 1 : -1;  // 1 for zoom in, -1 for zoom out

    // Check if we're continuing in the same direction or changing direction
    bool changingDirection = (game.camera.zoomDirection != 0 &&
                             game.camera.zoomDirection != direction);

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
      game.camera.zoomDirection = 0;  // Reset direction
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
    game.camera.panStartOffset = Vector2{game.camera.offsetX, game.camera.offsetY};
  }

  if (IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE)) {
    game.camera.isPanning = false;
  }

  if (game.camera.isPanning) {
    Vector2 currentMouse = GetMousePosition();
    Vector2 delta = {
      currentMouse.x - game.camera.panStartMouse.x,
      currentMouse.y - game.camera.panStartMouse.y
    };

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
