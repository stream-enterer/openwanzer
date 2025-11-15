#ifndef OPENWANZER_INPUT_HPP
#define OPENWANZER_INPUT_HPP

#include "GameState.hpp"

namespace Input {

// Camera controls
void calculateCenteredCameraOffset(CameraState& camera, int screenWidth, int screenHeight);
void handleZoom(GameState& game);
void handlePan(GameState& game);

// UI interaction
void handleCombatLogScroll(GameState& game);
void handleCombatLogDrag(GameState& game);

} // namespace Input

#endif // OPENWANZER_INPUT_HPP
