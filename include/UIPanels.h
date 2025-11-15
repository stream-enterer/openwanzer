#ifndef OPENWANZER_UIPANELS_H
#define OPENWANZER_UIPANELS_H

#include "GameState.h"
#include "CombatArcs.h"

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

#endif // OPENWANZER_UIPANELS_H
