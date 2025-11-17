#include <cmath>
#include "GameState.hpp"
#include "PolyhedronRenderer.hpp"
#include "Rendering.hpp"
#include "rl/raygui.h"

namespace paperdollui {

void renderTargetPanel(const GameState &game) {
	// Need to cast away const for trackball updates
	GameState &mutableGame = const_cast<GameState &>(game);

	if (!mutableGame.targetPanel.isVisible || !mutableGame.targetPanel.targetUnit)
		return;

	// Draw panel background
	DrawRectangleRec(mutableGame.targetPanel.bounds, Color {40, 40, 40, 250});
	DrawRectangleLinesEx(mutableGame.targetPanel.bounds, 2.0f, Color {80, 80, 80, 255});

	// Draw header
	int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	const char *title = TextFormat("TARGET: %s", mutableGame.targetPanel.targetUnit->name.c_str());
	DrawText(title, mutableGame.targetPanel.bounds.x + 10, mutableGame.targetPanel.bounds.y + 10, fontSize, WHITE);

	// Update trackball camera
	mutableGame.targetPanel.polyView->trackball.Update(mutableGame.targetPanel.polyViewport);

	// Handle lock toggle
	if (GuiButton(mutableGame.targetPanel.lockToggleBounds,
	              mutableGame.targetPanel.polyView->lockedToGridView ? "Unlock" : "Lock to Grid")) {
		mutableGame.targetPanel.polyView->lockedToGridView = !mutableGame.targetPanel.polyView->lockedToGridView;

		if (mutableGame.targetPanel.polyView->lockedToGridView) {
			// Match grid view camera angle
			// Set camera to top-down view matching unit facing
			float facing = mutableGame.targetPanel.targetUnit->facing * DEG2RAD;
			float distance = 5.0f;
			float height = 4.0f;

			mutableGame.targetPanel.polyView->trackball.camera.position =
			    Vector3 {distance * sinf(facing), height, distance * cosf(facing)};
			mutableGame.targetPanel.polyView->trackball.camera.target = Vector3 {0, 0, 0};
			mutableGame.targetPanel.polyView->trackball.camera.up = Vector3 {0, 1, 0};
			mutableGame.targetPanel.polyView->trackball.angularVelocity = {0, 0};
		}
	}

	// Render 3D view
	mutableGame.targetPanel.polyView->Render(mutableGame.targetPanel.targetUnit->polyhedron, mutableGame.targetPanel.polyViewport, true);

	// Draw 3D view texture
	DrawTexturePro(mutableGame.targetPanel.polyView->renderTarget.texture,
	               Rectangle {0, 0, (float)mutableGame.targetPanel.polyView->renderTarget.texture.width,
	                          -(float)mutableGame.targetPanel.polyView->renderTarget.texture.height}, // Flip Y
	               mutableGame.targetPanel.polyViewport, Vector2 {0, 0}, 0.0f, WHITE);

	// Draw 3D viewport border
	DrawRectangleLinesEx(mutableGame.targetPanel.polyViewport, 1.0f, Color {60, 60, 60, 255});

	// Render net view
	mutableGame.targetPanel.netView->Render(mutableGame.targetPanel.targetUnit->polyhedron, mutableGame.targetPanel.netViewport);

	// Draw net view texture
	DrawTexturePro(mutableGame.targetPanel.netView->renderTarget.texture,
	               Rectangle {0, 0, (float)mutableGame.targetPanel.netView->renderTarget.texture.width,
	                          -(float)mutableGame.targetPanel.netView->renderTarget.texture.height},
	               mutableGame.targetPanel.netViewport, Vector2 {0, 0}, 0.0f, WHITE);

	// Draw net viewport border
	DrawRectangleLinesEx(mutableGame.targetPanel.netViewport, 1.0f, Color {60, 60, 60, 255});

	// Handle net view hover
	Vector2 mousePos = GetMousePosition();
	if (CheckCollisionPointRec(mousePos, mutableGame.targetPanel.netViewport)) {
		mutableGame.targetPanel.netView->hoveredFaceIndex =
		    mutableGame.targetPanel.netView->DetectHoveredFace(mutableGame.targetPanel.targetUnit->polyhedron, mousePos,
		                                                       mutableGame.targetPanel.netViewport);
	} else {
		mutableGame.targetPanel.netView->hoveredFaceIndex = -1;
	}
}

void renderPlayerPanel(const GameState &game) {
	// Need to cast away const for trackball updates
	GameState &mutableGame = const_cast<GameState &>(game);

	if (!mutableGame.playerPanel.isVisible || !mutableGame.playerPanel.playerUnit)
		return;

	// Draw panel background
	DrawRectangleRec(mutableGame.playerPanel.bounds, Color {40, 40, 40, 250});
	DrawRectangleLinesEx(mutableGame.playerPanel.bounds, 2.0f, Color {80, 80, 80, 255});

	// Draw header
	int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	const char *title = TextFormat("PLAYER: %s", mutableGame.playerPanel.playerUnit->name.c_str());
	DrawText(title, mutableGame.playerPanel.bounds.x + 10, mutableGame.playerPanel.bounds.y + 10, fontSize, WHITE);

	// Update trackball camera
	mutableGame.playerPanel.polyView->trackball.Update(mutableGame.playerPanel.polyViewport);

	// Handle lock toggle
	if (GuiButton(mutableGame.playerPanel.lockToggleBounds,
	              mutableGame.playerPanel.polyView->lockedToGridView ? "Unlock" : "Lock to Grid")) {
		mutableGame.playerPanel.polyView->lockedToGridView = !mutableGame.playerPanel.polyView->lockedToGridView;

		if (mutableGame.playerPanel.polyView->lockedToGridView) {
			// Match grid view camera angle
			float facing = mutableGame.playerPanel.playerUnit->facing * DEG2RAD;
			float distance = 5.0f;
			float height = 4.0f;

			mutableGame.playerPanel.polyView->trackball.camera.position =
			    Vector3 {distance * sinf(facing), height, distance * cosf(facing)};
			mutableGame.playerPanel.polyView->trackball.camera.target = Vector3 {0, 0, 0};
			mutableGame.playerPanel.polyView->trackball.camera.up = Vector3 {0, 1, 0};
			mutableGame.playerPanel.polyView->trackball.angularVelocity = {0, 0};
		}
	}

	// Render 3D view
	mutableGame.playerPanel.polyView->Render(mutableGame.playerPanel.playerUnit->polyhedron, mutableGame.playerPanel.polyViewport, true);

	// Draw 3D view texture
	DrawTexturePro(mutableGame.playerPanel.polyView->renderTarget.texture,
	               Rectangle {0, 0, (float)mutableGame.playerPanel.polyView->renderTarget.texture.width,
	                          -(float)mutableGame.playerPanel.polyView->renderTarget.texture.height}, // Flip Y
	               mutableGame.playerPanel.polyViewport, Vector2 {0, 0}, 0.0f, WHITE);

	// Draw 3D viewport border
	DrawRectangleLinesEx(mutableGame.playerPanel.polyViewport, 1.0f, Color {60, 60, 60, 255});

	// Render net view
	mutableGame.playerPanel.netView->Render(mutableGame.playerPanel.playerUnit->polyhedron, mutableGame.playerPanel.netViewport);

	// Draw net view texture
	DrawTexturePro(mutableGame.playerPanel.netView->renderTarget.texture,
	               Rectangle {0, 0, (float)mutableGame.playerPanel.netView->renderTarget.texture.width,
	                          -(float)mutableGame.playerPanel.netView->renderTarget.texture.height},
	               mutableGame.playerPanel.netViewport, Vector2 {0, 0}, 0.0f, WHITE);

	// Draw net viewport border
	DrawRectangleLinesEx(mutableGame.playerPanel.netViewport, 1.0f, Color {60, 60, 60, 255});

	// Handle net view hover
	Vector2 mousePos = GetMousePosition();
	if (CheckCollisionPointRec(mousePos, mutableGame.playerPanel.netViewport)) {
		mutableGame.playerPanel.netView->hoveredFaceIndex = mutableGame.playerPanel.netView->DetectHoveredFace(
		    mutableGame.playerPanel.playerUnit->polyhedron, mousePos, mutableGame.playerPanel.netViewport);
	} else {
		mutableGame.playerPanel.netView->hoveredFaceIndex = -1;
	}
}

} // namespace paperdollui
