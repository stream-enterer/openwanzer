#include "UIPanels.hpp"
#include "Constants.hpp"

namespace uipanel {

// Panel dimensions
const int TARGET_PANEL_WIDTH = 720;
const int TARGET_PANEL_HEIGHT = 200;
const int PLAYER_PANEL_WIDTH = 320;
const int PLAYER_PANEL_HEIGHT = 200;

// Internal layout constants
const int PAPERDOLL_PADDING = 10;
const int PAPERDOLL_SECTION_MARGIN = 2;
const int FRONT_PAPERDOLL_WIDTH = 120;
const int FRONT_PAPERDOLL_HEIGHT = 140;
const int REAR_PAPERDOLL_WIDTH = 100;
const int REAR_PAPERDOLL_HEIGHT = 140;

void initializeTargetPanel(GameState& game) {
    int targetPanelX = (SCREEN_WIDTH - TARGET_PANEL_WIDTH) / 2;
    int targetPanelY = 20;

    game.targetPanel.bounds = Rectangle{
        (float)targetPanelX,
        (float)targetPanelY,
        (float)TARGET_PANEL_WIDTH,
        (float)TARGET_PANEL_HEIGHT
    };
    game.targetPanel.defaultPosition = Vector2{
        (float)targetPanelX,
        (float)targetPanelY
    };
    game.targetPanel.isVisible = false;
}

void initializePlayerPanel(GameState& game) {
    int playerPanelX = 20;
    int playerPanelY = SCREEN_HEIGHT - PLAYER_PANEL_HEIGHT - 20;

    game.playerPanel.bounds = Rectangle{
        (float)playerPanelX,
        (float)playerPanelY,
        (float)PLAYER_PANEL_WIDTH,
        (float)PLAYER_PANEL_HEIGHT
    };
    game.playerPanel.defaultPosition = Vector2{
        (float)playerPanelX,
        (float)playerPanelY
    };
    game.playerPanel.isVisible = false;
}

void showTargetPanel(GameState& game, Unit* target, combatarcs::AttackArc arc) {
    game.targetPanel.targetUnit = target;
    game.targetPanel.currentArc = arc;
    game.targetPanel.isVisible = true;
    calculatePaperdollRegions(game.targetPanel);
}

void showPlayerPanel(GameState& game, Unit* player) {
    game.playerPanel.playerUnit = player;
    game.playerPanel.isVisible = true;
    calculatePaperdollRegions(game.playerPanel);
}

void hideTargetPanel(GameState& game) {
    game.targetPanel.isVisible = false;
    game.targetPanel.targetUnit = nullptr;
}

void hidePlayerPanel(GameState& game) {
    game.playerPanel.isVisible = false;
    game.playerPanel.playerUnit = nullptr;
}

void resetPanelPositions(GameState& game) {
    game.targetPanel.bounds.x = game.targetPanel.defaultPosition.x;
    game.targetPanel.bounds.y = game.targetPanel.defaultPosition.y;
    game.playerPanel.bounds.x = game.playerPanel.defaultPosition.x;
    game.playerPanel.bounds.y = game.playerPanel.defaultPosition.y;
}

void calculatePaperdollRegions(PaperdollPanel& panel) {
    // Calculate precise Rectangle bounds for each body part
    // based on panel.bounds position + internal layout

    float startX = panel.bounds.x + PAPERDOLL_PADDING;
    float startY = panel.bounds.y + 60;  // After header section

    // Front paperdoll layout (each section is 25x30)
    float sectionWidth = 25.0f;
    float sectionHeight = 30.0f;

    // Row 1: HEAD
    panel.frontHead = Rectangle{startX + sectionWidth * 2, startY,
                                 sectionWidth, sectionHeight};

    // Row 2: LA, LT, CT, RT, RA
    panel.frontLA = Rectangle{startX, startY + sectionHeight,
                               sectionWidth, sectionHeight};
    panel.frontLT = Rectangle{startX + sectionWidth, startY + sectionHeight,
                               sectionWidth, sectionHeight};
    panel.frontCT = Rectangle{startX + sectionWidth * 2, startY + sectionHeight,
                               sectionWidth, sectionHeight};
    panel.frontRT = Rectangle{startX + sectionWidth * 3, startY + sectionHeight,
                               sectionWidth, sectionHeight};
    panel.frontRA = Rectangle{startX + sectionWidth * 4, startY + sectionHeight,
                               sectionWidth, sectionHeight};

    // Row 3: LL, RL
    panel.frontLL = Rectangle{startX + sectionWidth, startY + sectionHeight * 2,
                               sectionWidth, sectionHeight};
    panel.frontRL = Rectangle{startX + sectionWidth * 3, startY + sectionHeight * 2,
                               sectionWidth, sectionHeight};

    // Rear paperdoll (to the right of front)
    float rearStartX = startX + FRONT_PAPERDOLL_WIDTH + 20;

    // Rear only has torso sections (arms are blackened)
    panel.rearLA = Rectangle{rearStartX, startY + sectionHeight,
                              sectionWidth * 0.8f, sectionHeight};
    panel.rearLT = Rectangle{rearStartX + sectionWidth * 0.8f, startY + sectionHeight,
                              sectionWidth, sectionHeight};
    panel.rearCT = Rectangle{rearStartX + sectionWidth * 1.8f, startY + sectionHeight,
                              sectionWidth, sectionHeight};
    panel.rearRT = Rectangle{rearStartX + sectionWidth * 2.8f, startY + sectionHeight,
                              sectionWidth, sectionHeight};
    panel.rearRA = Rectangle{rearStartX + sectionWidth * 3.8f, startY + sectionHeight,
                              sectionWidth * 0.8f, sectionHeight};
}

} // namespace uipanel
