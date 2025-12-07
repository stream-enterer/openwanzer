#ifndef OPENWANZER_PAPERDOLL_UI_HPP
#define OPENWANZER_PAPERDOLL_UI_HPP

#include "ArmorLocation.hpp"
#include "GameState.hpp"
#include "Raylib.hpp"
#include "Unit.hpp"

namespace paperdollui {

// Main panel rendering functions
void renderTargetPanel(const GameState& game);
void renderPlayerPanel(const GameState& game);

// Panel dragging and tooltip handlers (called from input.cpp)
void handlePaperdollPanelDrag(GameState& game);
void handlePaperdollTooltips(GameState& game);

// Flash overlay for hit animation
void triggerHitFlash(GameState& game, Unit* unit, ArmorLocation location);
void updatePanelFlashes(GameState& game);

} // namespace paperdollui

#endif // OPENWANZER_PAPERDOLL_UI_HPP
