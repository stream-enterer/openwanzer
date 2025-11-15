#ifndef OPENWANZER_UI_PANELS_HPP
#define OPENWANZER_UI_PANELS_HPP

#include "CombatArcs.hpp"
#include "GameState.hpp"

namespace uipanel {

// Panel initialization
void initializeTargetPanel(GameState& game);
void initializePlayerPanel(GameState& game);

// Panel visibility control
void showTargetPanel(GameState& game, Unit* target, combatarcs::AttackArc arc);
void showPlayerPanel(GameState& game, Unit* player);
void hideTargetPanel(GameState& game);
void hidePlayerPanel(GameState& game);

// Panel position management
void resetPanelPositions(GameState& game);
void calculatePaperdollRegions(PaperdollPanel& panel);

} // namespace uipanel

#endif // OPENWANZER_UI_PANELS_HPP
