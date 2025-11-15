#ifndef OPENWANZER_PAPERDOLL_UI_H
#define OPENWANZER_PAPERDOLL_UI_H

#include "GameState.h"
#include "Unit.h"
#include "raylib.h"

namespace PaperdollUI {

// Main panel rendering functions
void renderTargetPanel(const GameState& game);
void renderPlayerPanel(const GameState& game);

// Panel dragging and tooltip handlers (called from input.cpp)
void handlePaperdollPanelDrag(GameState& game);
void handlePaperdollTooltips(GameState& game);

} // namespace PaperdollUI

#endif // OPENWANZER_PAPERDOLL_UI_H
