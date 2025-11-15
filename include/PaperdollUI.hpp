#ifndef OPENWANZER_PAPERDOLL_UI_HPP
#define OPENWANZER_PAPERDOLL_UI_HPP

#include "GameState.hpp"
#include "Unit.hpp"
#include "rl/raylib.h"

namespace paperdollui {

// Main panel rendering functions
void renderTargetPanel(const GameState& game);
void renderPlayerPanel(const GameState& game);

// Panel dragging and tooltip handlers (called from input.cpp)
void handlePaperdollPanelDrag(GameState& game);
void handlePaperdollTooltips(GameState& game);

} // namespace paperdollui

#endif // OPENWANZER_PAPERDOLL_UI_HPP
