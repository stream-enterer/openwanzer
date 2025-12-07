#include <cmath>
#include "CherryStyle.hpp"
#include "CombatArcs.hpp"
#include "Constants.hpp"
#include "Hex.hpp"
#include "Rendering.hpp"

namespace rendering {

void drawTargetArcRing(GameState& game, Unit* unit) {
	if (!unit)
		return;

	Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
	                                game.camera.offsetY, game.camera.zoom);

	OffsetCoord unitOffset = gameCoordToOffset(unit->position);
	::Hex unitCube = OffsetToCube(unitOffset);
	Point center = HexToPixel(layout, unitCube);

	float radius = HEX_SIZE * game.camera.zoom * 0.7f;
	float facing = unit->facing;
	float gap = 2.0f; // Small gap between arc segments

	// Draw 4 arc segments
	combatarcs::AttackArc arcs[] = {
	    combatarcs::AttackArc::FRONT,
	    combatarcs::AttackArc::RIGHT_SIDE,
	    combatarcs::AttackArc::LEFT_SIDE,
	    combatarcs::AttackArc::REAR};

	// Arc angles with gaps: Front ±30°, Right 30-150°, Left -30 to -150°, Rear ±150-180°
	float arcRanges[][2] = {
	    {facing - 30.0f + gap, facing + 30.0f - gap},  // Front
	    {facing + 30.0f + gap, facing + 150.0f - gap}, // Right
	    {facing - 150.0f + gap, facing - 30.0f - gap}, // Left
	    {facing + 150.0f + gap, facing + 180.0f}       // Rear part 1 (handle wrap)
	};

	for (int i = 0; i < 4; i++) {
		Color color = combatarcs::getArcSegmentColor(arcs[i], i == 0);

		float startAngle = arcRanges[i][0];
		float endAngle = arcRanges[i][1];

		// Handle rear arc wrapping
		if (i == 3) {
			// Draw rear as two segments with gaps
			DrawRing(Vector2 {(float)center.x, (float)center.y},
			         radius - 5.0f, radius,
			         facing + 150.0f + gap, facing + 210.0f - gap, 32, color);
			DrawRing(Vector2 {(float)center.x, (float)center.y},
			         radius - 5.0f, radius,
			         facing - 210.0f + gap, facing - 150.0f - gap, 32, color);
		} else {
			DrawRing(Vector2 {(float)center.x, (float)center.y},
			         radius - 5.0f, radius,
			         startAngle, endAngle, 32, color);
		}
	}

	// Draw tiny red arrow for front facing
	float arrowAngle = facing * (PI / 180.0f);
	float arrowLength = 12.0f * game.camera.zoom;

	Vector2 arrowTip = {
	    (float)(center.x + cosf(arrowAngle) * (radius + arrowLength)),
	    (float)(center.y + sinf(arrowAngle) * (radius + arrowLength))};

	Vector2 arrowLeft = {
	    (float)(center.x + cosf(arrowAngle - 0.5f) * radius),
	    (float)(center.y + sinf(arrowAngle - 0.5f) * radius)};

	Vector2 arrowRight = {
	    (float)(center.x + cosf(arrowAngle + 0.5f) * radius),
	    (float)(center.y + sinf(arrowAngle + 0.5f) * radius)};

	DrawTriangle(arrowTip, arrowLeft, arrowRight, RED);
}

void drawAttackerFiringCone(GameState& game) {
	if (!game.selectedUnit || !game.movementSel.isFacingSelection)
		return;

	Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
	                                game.camera.offsetY, game.camera.zoom);

	OffsetCoord unitOffset = gameCoordToOffset(game.selectedUnit->position);
	::Hex unitCube = OffsetToCube(unitOffset);
	Point center = HexToPixel(layout, unitCube);

	float facing = game.movementSel.selectedFacing;
	float coneRadius = HEX_SIZE * game.camera.zoom * 8.0f; // Large radius

	// Draw 120° cone (±60° from facing)
	Color coneColor = Color {255, 255, 0, 40}; // Semi-transparent yellow

	DrawCircleSector(Vector2 {(float)center.x, (float)center.y},
	                 coneRadius,
	                 facing - 60.0f,
	                 facing + 60.0f,
	                 64,
	                 coneColor);

	// Draw cone boundary lines
	float leftAngle = (facing - 60.0f) * (PI / 180.0f);
	float rightAngle = (facing + 60.0f) * (PI / 180.0f);

	Vector2 leftEdge = {
	    (float)(center.x + cosf(leftAngle) * coneRadius),
	    (float)(center.y + sinf(leftAngle) * coneRadius)};

	Vector2 rightEdge = {
	    (float)(center.x + cosf(rightAngle) * coneRadius),
	    (float)(center.y + sinf(rightAngle) * coneRadius)};

	DrawLineEx(Vector2 {(float)center.x, (float)center.y}, leftEdge,
	           2.0f, YELLOW);
	DrawLineEx(Vector2 {(float)center.x, (float)center.y}, rightEdge,
	           2.0f, YELLOW);
}

// Draw a dotted/dashed line between two points
void drawDottedLine(Vector2 from, Vector2 to, float thickness, Color color, float dashLength, float gapLength) {
	float dx = to.x - from.x;
	float dy = to.y - from.y;
	float length = sqrtf(dx * dx + dy * dy);

	if (length < 1.0f)
		return;

	// Normalize direction
	float nx = dx / length;
	float ny = dy / length;

	float currentPos = 0.0f;
	bool drawing = true;

	while (currentPos < length) {
		float segmentLength = drawing ? dashLength : gapLength;
		float endPos = currentPos + segmentLength;
		if (endPos > length)
			endPos = length;

		if (drawing) {
			Vector2 segStart = {from.x + nx * currentPos, from.y + ny * currentPos};
			Vector2 segEnd = {from.x + nx * endPos, from.y + ny * endPos};
			DrawLineEx(segStart, segEnd, thickness, color);
		}

		currentPos = endPos;
		drawing = !drawing;
	}
}

void drawAttackLines(GameState& game) {
	if (!game.showAttackLines || game.attackLines.empty())
		return;

	Layout layout = createHexLayout(HEX_SIZE, game.camera.offsetX,
	                                game.camera.offsetY, game.camera.zoom);

	for (const auto& line : game.attackLines) {
		OffsetCoord fromOffset = gameCoordToOffset(line.from);
		OffsetCoord toOffset = gameCoordToOffset(line.to);

		::Hex fromCube = OffsetToCube(fromOffset);
		::Hex toCube = OffsetToCube(toOffset);

		Point fromPoint = HexToPixel(layout, fromCube);
		Point toPoint = HexToPixel(layout, toCube);

		Vector2 fromVec = {(float)fromPoint.x, (float)fromPoint.y};
		Vector2 toVec = {(float)toPoint.x, (float)toPoint.y};

		if (line.outOfRange) {
			// Draw gray dotted line for out-of-range targets
			drawDottedLine(fromVec, toVec, 3.0f, GRAY, 8.0f, 6.0f);
		} else {
			// Draw solid colored line for in-range targets
			Color lineColor = combatarcs::getLineColor(line.arc);
			DrawLineEx(fromVec, toVec, 3.0f, lineColor);
		}
	}
}

void drawCombatTexts(GameState& game) {
	if (game.combatTexts.empty())
		return;

	float spacing = (float)cherrystyle::kFontSpacing;
	const int fontSize = cherrystyle::kFontSize;

	for (const auto& ct : game.combatTexts) {
		Color textColor = ct.color;
		textColor.a = ct.getAlpha();

		DrawTextEx(cherrystyle::CHERRY_FONT, ct.text.c_str(),
		           ct.position, (float)fontSize, spacing, textColor);
	}
}

} // namespace rendering
