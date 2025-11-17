#include "PolyhedronRenderer.hpp"
#include <algorithm>
#include <cmath>
#include "Rendering.hpp"
#include "Hex.hpp"
#include "rl/raygui.h"

// Convert hex coordinate to 3D world position
Vector3 HexToWorld3D(HexCoord coord, const Layout &layout) {
	OffsetCoord offset = gameCoordToOffset(coord);
	::Hex cubeHex = OffsetToCube(offset);
	Point center = HexToPixel(layout, cubeHex);
	return Vector3 {(float)center.x, 0, (float)center.y};
}

// TrackballCamera implementation
void TrackballCamera::Update(Rectangle viewport) {
	if (isDragging) {
		Vector2 mousePos = GetMousePosition();
		if (CheckCollisionPointRec(mousePos, viewport)) {
			Vector2 delta = Vector2Subtract(mousePos, previousMousePos);
			Rotate(delta);
		}
		previousMousePos = mousePos;

		if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
			isDragging = false;
		}
	} else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		Vector2 mousePos = GetMousePosition();
		if (CheckCollisionPointRec(mousePos, viewport)) {
			isDragging = true;
			previousMousePos = mousePos;
			angularVelocity = {0, 0};
		}
	}

	// Apply momentum and friction
	if (!isDragging && (fabs(angularVelocity.x) > 0.001f || fabs(angularVelocity.y) > 0.001f)) {
		// Rotate camera based on velocity
		float dt = GetFrameTime();

		// Orbit camera around target
		Vector3 right = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(camera.target, camera.position), camera.up));
		Vector3 up = camera.up;

		// Rotate around up axis (Y rotation)
		Matrix rotY = MatrixRotate(up, angularVelocity.x * dt);
		// Rotate around right axis (X rotation)
		Matrix rotX = MatrixRotate(right, angularVelocity.y * dt);

		Vector3 offset = Vector3Subtract(camera.position, camera.target);
		offset = Vector3Transform(offset, MatrixMultiply(rotX, rotY));
		camera.position = Vector3Add(camera.target, offset);

		// Apply friction
		angularVelocity.x *= friction;
		angularVelocity.y *= friction;
	}
}

void TrackballCamera::Rotate(Vector2 delta) {
	float sensitivity = 0.003f; // Radians per pixel

	// Update angular velocity (for momentum)
	float dt = GetFrameTime();
	if (dt > 0) {
		angularVelocity.x = -delta.x * sensitivity / dt;
		angularVelocity.y = delta.y * sensitivity / dt;
	}

	// Immediate rotation (while dragging)
	Vector3 right = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(camera.target, camera.position), camera.up));

	Matrix rotY = MatrixRotate(camera.up, -delta.x * sensitivity);
	Matrix rotX = MatrixRotate(right, delta.y * sensitivity);

	Vector3 offset = Vector3Subtract(camera.position, camera.target);
	offset = Vector3Transform(offset, MatrixMultiply(rotX, rotY));
	camera.position = Vector3Add(camera.target, offset);
}

// PolyhedronView implementation
void PolyhedronView::Initialize(int width, int height) {
	renderTarget = LoadRenderTexture(width, height);
}

void PolyhedronView::Cleanup() {
	UnloadRenderTexture(renderTarget);
}

void PolyhedronView::Render(const PolyhedronData &poly, Rectangle viewport, bool drawNumbers) {
	// Render to texture for viewport
	BeginTextureMode(renderTarget);
	ClearBackground(Color {40, 40, 40, 255}); // Match panel background

	BeginMode3D(trackball.camera);

	// Draw polyhedron
	DrawModel(poly.model, Vector3 {0, 0, 0}, 1.0f, WHITE);

	// Draw face borders (1px dark grey lines)
	for (const auto &face : poly.faces) {
		for (size_t i = 0; i < face.vertices.size(); i++) {
			size_t next = (i + 1) % face.vertices.size();
			DrawLine3D(face.vertices[i], face.vertices[next], Color {60, 60, 60, 255});
		}
	}

	EndMode3D();

	// Draw face numbers (in 2D screen space after 3D)
	if (drawNumbers) {
		DrawFaceNumbers(poly);
	}

	EndTextureMode();
}

void PolyhedronView::DrawFaceNumbers(const PolyhedronData &poly) {
	// Project face centers to screen space and draw HP numbers
	for (size_t i = 0; i < poly.faces.size(); i++) {
		const PolyFace &face = poly.faces[i];

		// Check if face is visible (facing camera)
		Vector3 toCamera = Vector3Normalize(Vector3Subtract(trackball.camera.position, face.center));
		float dot = Vector3DotProduct(face.normal, toCamera);

		if (dot > 0.1f) { // Face is visible
			Vector2 screenPos = GetWorldToScreen(face.center, trackball.camera);

			// Draw HP number
			const char *text;
			if (face.isDestroyed) {
				text = "0";
			} else {
				text = TextFormat("%d", face.currentHP);
			}

			int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
			int textWidth = MeasureText(text, fontSize);

			// Draw with outline for readability
			DrawText(text, screenPos.x - textWidth / 2 + 1, screenPos.y - fontSize / 2 + 1, fontSize, BLACK);
			DrawText(text, screenPos.x - textWidth / 2, screenPos.y - fontSize / 2, fontSize, WHITE);
		}
	}
}

// NetView implementation
void NetView::Initialize(int width, int height) {
	renderTarget = LoadRenderTexture(width, height);
	hoveredFaceIndex = -1;
}

void NetView::Cleanup() {
	UnloadRenderTexture(renderTarget);
}

Vector2 NetView::GetFacePosition(const PolyhedronData &poly, int faceIndex) {
	// Simple grid layout for now
	int cols = sqrt(poly.faces.size()) + 1;
	int row = faceIndex / cols;
	int col = faceIndex % cols;

	float spacing = 80.0f;
	return Vector2 {col * spacing + 40, row * spacing + 40};
}

void NetView::Render(const PolyhedronData &poly, Rectangle viewport) {
	BeginTextureMode(renderTarget);
	ClearBackground(Color {40, 40, 40, 255});

	// Calculate scale to fit viewport
	float scale = 15.0f; // Base scale

	// Draw flattened net
	for (size_t i = 0; i < poly.faces.size(); i++) {
		const PolyFace &face = poly.faces[i];
		Vector2 netPos = GetFacePosition(poly, i);

		// Draw face polygon (projected to 2D)
		std::vector<Vector2> screenVerts;
		for (const auto &v3d : face.vertices) {
			// Simple XZ projection
			Vector2 v2d = {v3d.x, v3d.z};
			v2d = Vector2Scale(v2d, scale);
			v2d = Vector2Add(v2d, netPos);
			screenVerts.push_back(v2d);
		}

		// Fill face with color
		Color faceColor = GetFaceColor(face);
		faceColor.a = 255; // Opaque for net view

		// Triangulate and draw
		if (screenVerts.size() >= 3) {
			for (size_t t = 1; t < screenVerts.size() - 1; t++) {
				DrawTriangle(screenVerts[0], screenVerts[t], screenVerts[t + 1], faceColor);
			}
		}

		// Draw borders
		for (size_t j = 0; j < screenVerts.size(); j++) {
			size_t next = (j + 1) % screenVerts.size();
			DrawLineEx(screenVerts[j], screenVerts[next], 2.0f, Color {60, 60, 60, 255});
		}

		// Draw face number
		const char *text = TextFormat("%d", (int)i);
		int fontSize = 10;
		int textWidth = MeasureText(text, fontSize);
		DrawText(text, netPos.x - textWidth / 2, netPos.y - fontSize / 2, fontSize, WHITE);
	}

	// Draw hover tooltip
	if (hoveredFaceIndex >= 0 && hoveredFaceIndex < (int)poly.faces.size()) {
		const PolyFace &face = poly.faces[hoveredFaceIndex];
		Vector2 mousePos = GetMousePosition();

		const char *text = face.isStructure ? TextFormat("Structure: %d", face.currentHP)
		                                    : TextFormat("Armor: %d", face.currentHP);

		// Draw tooltip box
		int fontSize = 12;
		int textWidth = MeasureText(text, fontSize);
		DrawRectangle(mousePos.x, mousePos.y, textWidth + 10, 20, Color {0, 0, 0, 220});
		DrawText(text, mousePos.x + 5, mousePos.y + 5, fontSize, WHITE);
	}

	EndTextureMode();
}

int NetView::DetectHoveredFace(const PolyhedronData &poly, Vector2 mousePos, Rectangle viewport) {
	// Convert mouse position to render texture coordinates
	Vector2 localPos = {mousePos.x - viewport.x, viewport.height - (mousePos.y - viewport.y)};

	float scale = 15.0f;

	for (size_t i = 0; i < poly.faces.size(); i++) {
		const PolyFace &face = poly.faces[i];
		Vector2 netPos = GetFacePosition(poly, i);

		// Project face vertices to screen
		std::vector<Vector2> screenVerts;
		for (const auto &v3d : face.vertices) {
			Vector2 v2d = {v3d.x, v3d.z};
			v2d = Vector2Scale(v2d, scale);
			v2d = Vector2Add(v2d, netPos);
			screenVerts.push_back(v2d);
		}

		// Simple point-in-polygon test (using center distance for now)
		if (screenVerts.size() >= 3) {
			float dist = Vector2Distance(localPos, netPos);
			if (dist < 30.0f) { // Rough approximation
				return i;
			}
		}
	}

	return -1;
}

// Debug visualization
void RenderDebugFaceNormals(const PolyhedronData &poly, Vector3 worldPos) {
	for (const auto &face : poly.faces) {
		Vector3 start = Vector3Add(face.center, worldPos);
		Vector3 end = Vector3Add(start, Vector3Scale(face.normal, 2.0f));
		DrawLine3D(start, end, YELLOW);
		DrawSphere(end, 0.1f, YELLOW);
	}
}

void HighlightVisibleFaces(const PolyhedronData &attacker, const PolyhedronData &defender, Vector3 attackerPos,
                           Vector3 defenderPos) {
	// For each face of defender, check if visible from attacker
	for (const auto &defFace : defender.faces) {
		Vector3 defFaceWorld = Vector3Add(defFace.center, defenderPos);

		// Check if any attacker face can see this defender face
		bool visible = false;
		for (const auto &atkFace : attacker.faces) {
			Vector3 atkFaceWorld = Vector3Add(atkFace.center, attackerPos);

			Vector3 atkToDefDir = Vector3Normalize(Vector3Subtract(defFaceWorld, atkFaceWorld));
			Vector3 defToAtkDir = Vector3Normalize(Vector3Subtract(atkFaceWorld, defFaceWorld));

			float atkDot = Vector3DotProduct(atkFace.normal, atkToDefDir);
			float defDot = Vector3DotProduct(defFace.normal, defToAtkDir);

			if (atkDot > 0.1f && defDot > 0.1f) {
				visible = true;
				break;
			}
		}

		// Draw highlighted face in green if visible
		if (visible) {
			for (size_t i = 1; i < defFace.vertices.size() - 1; i++) {
				Vector3 v0 = Vector3Add(defFace.vertices[0], defenderPos);
				Vector3 v1 = Vector3Add(defFace.vertices[i], defenderPos);
				Vector3 v2 = Vector3Add(defFace.vertices[i + 1], defenderPos);
				DrawTriangle3D(v0, v1, v2, Color {0, 255, 0, 100});
			}
		}
	}
}
