#ifndef OPENWANZER_UI_PANELS_H
#define OPENWANZER_UI_PANELS_H

#include "../core/game_state.h"
#include "../game_logic/combat_arcs.h"

namespace UIPanel {

// Panel initialization
void initializeTargetPanel(GameState& game);
void initializePlayerPanel(GameState& game);

// Panel visibility control
void showTargetPanel(GameState& game, Unit* target, CombatArcs::AttackArc arc);
void showPlayerPanel(GameState& game, Unit* player);
void hideTargetPanel(GameState& game);
void hidePlayerPanel(GameState& game);

// Panel position management
void resetPanelPositions(GameState& game);
void calculatePaperdollRegions(PaperdollPanel& panel);

} // namespace UIPanel

#endif // OPENWANZER_UI_PANELS_H
