#include "game_logic.h"
#include "combat_arcs.h"
#include "../rendering/rendering.h"
#include "../core/constants.h"

namespace GameLogic {

void updateAttackLines(GameState& game) {
    if (!game.selectedUnit) return;

    game.attackLines.clear();

    // If in facing selection mode, use preview facing
    // Otherwise use confirmed facing
    float facing = game.movementSel.isFacingSelection
                   ? game.movementSel.selectedFacing
                   : game.selectedUnit->facing;

    Layout layout = Rendering::createHexLayout(HEX_SIZE, 0, 0, 1.0f);

    OffsetCoord attackerOffset = Rendering::gameCoordToOffset(game.selectedUnit->position);
    ::Hex attackerCube = offset_to_cube(attackerOffset);
    Point attackerPos = hex_to_pixel(layout, attackerCube);

    for (const auto& unit : game.units) {
        // Skip self and friendlies
        if (unit.get() == game.selectedUnit ||
            unit->side == game.selectedUnit->side) continue;

        OffsetCoord targetOffset = Rendering::gameCoordToOffset(unit->position);
        ::Hex targetCube = offset_to_cube(targetOffset);
        Point targetPos = hex_to_pixel(layout, targetCube);

        Vector2 atkPos = {(float)attackerPos.x, (float)attackerPos.y};
        Vector2 tgtPos = {(float)targetPos.x, (float)targetPos.y};

        // Check if target is in firing arc
        if (!CombatArcs::isInFiringArc(atkPos, facing, tgtPos)) continue;

        // Calculate which arc of target we're hitting
        CombatArcs::AttackArc arc = CombatArcs::getAttackArc(
            atkPos, tgtPos, unit->facing
        );

        game.attackLines.emplace_back(
            game.selectedUnit->position,
            unit->position,
            arc
        );
    }
}

} // namespace GameLogic
