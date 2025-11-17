#include "PolyhedronRenderer.hpp"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <queue>
#include "Hex.hpp"
#include "Rendering.hpp"
#include "rl/raygui.h"

// Convert hex coordinate to 3D world position
Vector3 HexToWorld3D(HexCoord coord, const Layout &layout) {
	OffsetCoord offset = rendering::gameCoordToOffset(coord);
	::Hex cubeHex = OffsetToCube(offset);
	Point center = HexToPixel(layout, cubeHex);
	return Vector3 {(float)center.x, 0, (float)center.y};
}

// TrackballCamera implementation
void TrackballCamera::Update(Rectangle viewport, bool panelIsDragging) {
	// Don't activate trackball rotation if the panel itself is being dragged
	if (panelIsDragging) {
		isDragging = false;
		return;
	}

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

// ============================================================================
// NET VIEW HELPER FUNCTIONS
// ============================================================================

// Check if two faces share an edge (2+ vertices within epsilon distance)
bool FacesShareEdge(const PolyFace &f1, const PolyFace &f2) {
	int sharedVerts = 0;
	const float epsilon = 0.001f;

	for (const auto &v1 : f1.vertices) {
		for (const auto &v2 : f2.vertices) {
			if (Vector3Distance(v1, v2) < epsilon) {
				sharedVerts++;
				if (sharedVerts >= 2)
					return true;
			}
		}
	}
	return false;
}

// Find shared edge between two faces
Edge3D FindSharedEdge(const PolyFace &f1, const PolyFace &f2) {
	const float epsilon = 0.001f;
	std::vector<Vector3> sharedVerts;

	for (const auto &v1 : f1.vertices) {
		for (const auto &v2 : f2.vertices) {
			if (Vector3Distance(v1, v2) < epsilon) {
				bool alreadyAdded = false;
				for (const auto &sv : sharedVerts) {
					if (Vector3Distance(sv, v1) < epsilon) {
						alreadyAdded = true;
						break;
					}
				}
				if (!alreadyAdded) {
					sharedVerts.push_back(v1);
				}
			}
		}
	}

	if (sharedVerts.size() >= 2) {
		return Edge3D {sharedVerts[0], sharedVerts[1]};
	}
	return Edge3D {Vector3 {0, 0, 0}, Vector3 {0, 0, 0}};
}

// Project 3D face to 2D plane
FacePosition2D ProjectFaceToPlane(const PolyFace &face) {
	FacePosition2D result;

	// Use face normal to determine projection plane
	Vector3 normal = Vector3Normalize(face.normal);

	// Find two perpendicular vectors in the face plane
	Vector3 tangent1, tangent2;
	if (fabs(normal.x) < 0.9f) {
		tangent1 = Vector3Normalize(Vector3CrossProduct(normal, Vector3 {1, 0, 0}));
	} else {
		tangent1 = Vector3Normalize(Vector3CrossProduct(normal, Vector3 {0, 1, 0}));
	}
	tangent2 = Vector3CrossProduct(normal, tangent1);

	// Project each vertex onto the 2D plane
	for (const auto &v3d : face.vertices) {
		Vector3 relative = Vector3Subtract(v3d, face.center);
		Vector2 v2d = {Vector3DotProduct(relative, tangent1), Vector3DotProduct(relative, tangent2)};
		result.vertices.push_back(v2d);
	}

	return result;
}

// Rotate 2D point around origin
Vector2 RotatePoint(Vector2 point, float angleRad) {
	float cos_a = cosf(angleRad);
	float sin_a = sinf(angleRad);

	return Vector2 {point.x * cos_a - point.y * sin_a, point.x * sin_a + point.y * cos_a};
}

// Find vertex in 2D face that matches 3D vertex
Vector2 FindVertex2D(const FacePosition2D &face2D, const PolyFace &face3D, Vector3 vertex3D) {
	const float epsilon = 0.001f;

	for (size_t i = 0; i < face3D.vertices.size(); i++) {
		if (Vector3Distance(face3D.vertices[i], vertex3D) < epsilon) {
			return face2D.vertices[i];
		}
	}
	return Vector2 {0, 0};
}

// Calculate angle to align two edges
float CalculateAlignmentAngle(Vector2 edgeStart1, Vector2 edgeEnd1, Vector2 edgeStart2, Vector2 edgeEnd2) {
	Vector2 dir1 = Vector2Normalize(Vector2Subtract(edgeEnd1, edgeStart1));
	Vector2 dir2 = Vector2Normalize(Vector2Subtract(edgeEnd2, edgeStart2));

	float angle1 = atan2f(dir1.y, dir1.x);
	float angle2 = atan2f(dir2.y, dir2.x);

	// Flip direction (we want edges to align but point opposite ways)
	return angle2 - angle1 + PI;
}

// Generate contiguous net using BFS
NetLayout GenerateContiguousNet(const PolyhedronData &poly) {
	NetLayout layout;
	layout.positions.resize(poly.faces.size());

	// Step 1: Build adjacency graph
	std::vector<std::vector<int>> adjacency(poly.faces.size());
	for (size_t i = 0; i < poly.faces.size(); i++) {
		for (size_t j = i + 1; j < poly.faces.size(); j++) {
			if (FacesShareEdge(poly.faces[i], poly.faces[j])) {
				adjacency[i].push_back(j);
				adjacency[j].push_back(i);
			}
		}
	}

	// Step 2: Unfold using breadth-first traversal
	std::vector<bool> placed(poly.faces.size(), false);
	std::queue<int> toPlace;

	// Start with face 0 at origin
	layout.positions[0] = ProjectFaceToPlane(poly.faces[0]);
	placed[0] = true;
	toPlace.push(0);

	while (!toPlace.empty()) {
		int currentFaceIdx = toPlace.front();
		toPlace.pop();

		const PolyFace &currentFace = poly.faces[currentFaceIdx];
		const FacePosition2D &currentFace2D = layout.positions[currentFaceIdx];

		// Place all adjacent unplaced faces
		for (int adjFaceIdx : adjacency[currentFaceIdx]) {
			if (placed[adjFaceIdx])
				continue;

			const PolyFace &adjFace = poly.faces[adjFaceIdx];

			// Find shared edge
			Edge3D sharedEdge = FindSharedEdge(currentFace, adjFace);

			// Project adjacent face to 2D
			FacePosition2D adjFace2D = ProjectFaceToPlane(adjFace);

			// Find shared edge vertices in both 2D projections
			Vector2 currentEdgeStart = FindVertex2D(currentFace2D, currentFace, sharedEdge.v1);
			Vector2 currentEdgeEnd = FindVertex2D(currentFace2D, currentFace, sharedEdge.v2);
			Vector2 adjEdgeStart = FindVertex2D(adjFace2D, adjFace, sharedEdge.v1);
			Vector2 adjEdgeEnd = FindVertex2D(adjFace2D, adjFace, sharedEdge.v2);

			// Calculate rotation to align edges
			float angle = CalculateAlignmentAngle(adjEdgeStart, adjEdgeEnd, currentEdgeStart, currentEdgeEnd);

			// Rotate adjacent face
			for (auto &vertex : adjFace2D.vertices) {
				vertex = RotatePoint(vertex, angle);
			}

			// Calculate translation
			Vector2 rotatedAdjStart = RotatePoint(adjEdgeStart, angle);
			Vector2 translation = Vector2Subtract(currentEdgeStart, rotatedAdjStart);

			// Translate adjacent face
			for (auto &vertex : adjFace2D.vertices) {
				vertex = Vector2Add(vertex, translation);
			}

			layout.positions[adjFaceIdx] = adjFace2D;
			placed[adjFaceIdx] = true;
			toPlace.push(adjFaceIdx);
		}
	}

	// Step 3: Calculate bounding box
	float minX = FLT_MAX, minY = FLT_MAX;
	float maxX = -FLT_MAX, maxY = -FLT_MAX;

	for (const auto &facePos : layout.positions) {
		for (const auto &vertex : facePos.vertices) {
			minX = fmin(minX, vertex.x);
			minY = fmin(minY, vertex.y);
			maxX = fmax(maxX, vertex.x);
			maxY = fmax(maxY, vertex.y);
		}
	}

	layout.boundingBox = Rectangle {minX, minY, maxX - minX, maxY - minY};

	return layout;
}

// Optimize net orientation to maximize viewport usage
NetLayoutWithRotation OptimizeNetOrientation(const NetLayout &net, Rectangle viewport) {
	float bestScale = 0.0f;
	float bestRotation = 0.0f;

	float padding = 20.0f;
	float availableWidth = viewport.width - 2 * padding;
	float availableHeight = viewport.height - 2 * padding;

	Vector2 netCenter = Vector2 {net.boundingBox.x + net.boundingBox.width / 2, net.boundingBox.y + net.boundingBox.height / 2};

	// Try 8 rotation angles
	for (int i = 0; i < 8; i++) {
		float angle = i * 45.0f;
		float angleRad = angle * DEG2RAD;

		// Calculate bounding box for this rotation
		float rotMinX = FLT_MAX, rotMinY = FLT_MAX;
		float rotMaxX = -FLT_MAX, rotMaxY = -FLT_MAX;

		for (const auto &facePos : net.positions) {
			for (auto vertex : facePos.vertices) {
				// Translate to origin
				vertex = Vector2Subtract(vertex, netCenter);
				// Rotate
				vertex = RotatePoint(vertex, angleRad);

				rotMinX = fmin(rotMinX, vertex.x);
				rotMinY = fmin(rotMinY, vertex.y);
				rotMaxX = fmax(rotMaxX, vertex.x);
				rotMaxY = fmax(rotMaxY, vertex.y);
			}
		}

		float rotWidth = rotMaxX - rotMinX;
		float rotHeight = rotMaxY - rotMinY;

		// Calculate scale that fits in viewport
		float scaleX = availableWidth / rotWidth;
		float scaleY = availableHeight / rotHeight;
		float scale = fmin(scaleX, scaleY);

		// Keep rotation with largest scale
		if (scale > bestScale) {
			bestScale = scale;
			bestRotation = angle;
		}
	}

	return NetLayoutWithRotation {net, bestRotation, bestScale};
}

// Point-in-polygon test
bool PointInPolygon(Vector2 point, const std::vector<Vector2> &polygon) {
	int intersections = 0;

	for (size_t i = 0; i < polygon.size(); i++) {
		size_t next = (i + 1) % polygon.size();
		Vector2 v1 = polygon[i];
		Vector2 v2 = polygon[next];

		if ((v1.y > point.y) != (v2.y > point.y)) {
			float xIntersection = (v2.x - v1.x) * (point.y - v1.y) / (v2.y - v1.y) + v1.x;
			if (point.x < xIntersection) {
				intersections++;
			}
		}
	}

	return (intersections % 2) == 1;
}

// ============================================================================
// NETVIEW IMPLEMENTATION
// ============================================================================

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

	// Generate contiguous net
	NetLayout net = GenerateContiguousNet(poly);

	// Find optimal rotation and scale
	NetLayoutWithRotation optimized = OptimizeNetOrientation(net, viewport);

	Vector2 netCenter = Vector2 {net.boundingBox.x + net.boundingBox.width / 2, net.boundingBox.y + net.boundingBox.height / 2};

	Vector2 viewportCenter = Vector2 {viewport.width / 2, viewport.height / 2};

	float angleRad = optimized.rotation * DEG2RAD;

	// Draw each face with optimal rotation and scale
	for (size_t i = 0; i < poly.faces.size(); i++) {
		std::vector<Vector2> screenVerts;

		for (auto vertex : net.positions[i].vertices) {
			// Step 1: Translate to origin (relative to net center)
			vertex = Vector2Subtract(vertex, netCenter);

			// Step 2: Rotate to optimal angle
			vertex = RotatePoint(vertex, angleRad);

			// Step 3: Scale to fill viewport
			vertex = Vector2Scale(vertex, optimized.scaleFactor);

			// Step 4: Translate to viewport center
			vertex = Vector2Add(vertex, viewportCenter);

			screenVerts.push_back(vertex);
		}

		// Fill face with color
		Color faceColor = GetFaceColor(poly.faces[i]);
		faceColor.a = 255; // Opaque for net view

		// Triangulate and draw (fan triangulation)
		if (screenVerts.size() >= 3) {
			for (size_t t = 1; t < screenVerts.size() - 1; t++) {
				DrawTriangle(screenVerts[0], screenVerts[t], screenVerts[t + 1], faceColor);
			}
		}

		// Draw borders
		for (size_t j = 0; j < screenVerts.size(); j++) {
			size_t next = (j + 1) % screenVerts.size();
			DrawLineEx(screenVerts[j], screenVerts[next], 1.0f, Color {60, 60, 60, 255});
		}
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
		DrawRectangle(mousePos.x + 5, mousePos.y + 5, textWidth + 10, 22, Color {0, 0, 0, 220});
		DrawText(text, mousePos.x + 10, mousePos.y + 10, fontSize, WHITE);
	}

	EndTextureMode();
}

int NetView::DetectHoveredFace(const PolyhedronData &poly, Vector2 mousePos, Rectangle viewport) {
	// Convert mouse position to render texture coordinates
	Vector2 localPos = {mousePos.x - viewport.x, viewport.height - (mousePos.y - viewport.y)};

	// Generate contiguous net with same transformations as Render()
	NetLayout net = GenerateContiguousNet(poly);
	NetLayoutWithRotation optimized = OptimizeNetOrientation(net, viewport);

	Vector2 netCenter = Vector2 {net.boundingBox.x + net.boundingBox.width / 2, net.boundingBox.y + net.boundingBox.height / 2};
	Vector2 viewportCenter = Vector2 {viewport.width / 2, viewport.height / 2};
	float angleRad = optimized.rotation * DEG2RAD;

	// Reverse transform: viewport → net space
	// Reverse step 4: translate from viewport center
	Vector2 relativePos = Vector2Subtract(localPos, viewportCenter);
	// Reverse step 3: unscale
	relativePos = Vector2Scale(relativePos, 1.0f / optimized.scaleFactor);
	// Reverse step 2: unrotate
	relativePos = RotatePoint(relativePos, -angleRad);
	// Reverse step 1: translate back from net center origin
	relativePos = Vector2Add(relativePos, netCenter);

	// Check which face is under mouse in net space
	for (size_t i = 0; i < net.positions.size(); i++) {
		if (PointInPolygon(relativePos, net.positions[i].vertices)) {
			return i;
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
