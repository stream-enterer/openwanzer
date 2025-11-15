#include "GameLogic.hpp"
#include "CombatArcs.hpp"
#include "Rendering.hpp"
#include "Constants.hpp"

namespace gamelogic {

void updateAttackLines(GameState& game) {
    if (!game.selectedUnit) return;

    game.attackLines.clear();

    // Only show targeting lines for friendly units that haven't fired yet
    if (game.selectedUnit->side != game.currentPlayer || game.selectedUnit->hasFired) {
        return;
    }

    // If in facing selection mode, use preview facing
    // Otherwise use confirmed facing
    float facing = game.movementSel.isFacingSelection
                   ? game.movementSel.selectedFacing
                   : game.selectedUnit->facing;

    Layout layout = rendering::createHexLayout(HEX_SIZE, 0, 0, 1.0f);

    OffsetCoord attackerOffset = rendering::gameCoordToOffset(game.selectedUnit->position);
    ::Hex attackerCube = OffsetToCube(attackerOffset);
    Point attackerPos = HexToPixel(layout, attackerCube);

    for (const auto& unit : game.units) {
        // Skip self and friendlies
        if (unit.get() == game.selectedUnit ||
            unit->side == game.selectedUnit->side) continue;

        // Only show targeting lines to units that are in LOS (spotted by current player)
        GameHex& targetHex = game.map[unit->position.row][unit->position.col];
        if (!targetHex.isSpotted(game.currentPlayer)) continue;

        OffsetCoord targetOffset = rendering::gameCoordToOffset(unit->position);
        ::Hex targetCube = OffsetToCube(targetOffset);
        Point targetPos = HexToPixel(layout, targetCube);

        Vector2 atkPos = {(float)attackerPos.x, (float)attackerPos.y};
        Vector2 tgtPos = {(float)targetPos.x, (float)targetPos.y};

        // Check if target is in firing arc
        if (!combatarcs::isInFiringArc(atkPos, facing, tgtPos)) continue;

        // Calculate which arc of target we're hitting
        combatarcs::AttackArc arc = combatarcs::getAttackArc(
            atkPos, tgtPos, unit->facing
        );

        game.attackLines.emplace_back(
            game.selectedUnit->position,
            unit->position,
            arc
        );
    }
}

} // namespace gamelogic
