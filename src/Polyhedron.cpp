#include "Polyhedron.hpp"
#include <algorithm>
#include <cmath>
#include "rl/rlgl.h"

// Helper function to create a face with HP
PolyFace CreateFace(const std::vector<Vector3> &vertices, int baseHP) {
	PolyFace face;
	face.vertices = vertices;
	face.normal = CalculateFaceNormal(vertices);
	face.center = CalculateFaceCenter(vertices);
	face.currentHP = baseHP;
	face.maxHP = baseHP;
	face.isStructure = false;
	face.isDestroyed = false;
	return face;
}

// Calculate face normal (assumes counterclockwise winding)
Vector3 CalculateFaceNormal(const std::vector<Vector3> &vertices) {
	if (vertices.size() < 3)
		return Vector3 {0, 1, 0};

	Vector3 v1 = Vector3Subtract(vertices[1], vertices[0]);
	Vector3 v2 = Vector3Subtract(vertices[2], vertices[0]);
	Vector3 normal = Vector3CrossProduct(v1, v2);
	return Vector3Normalize(normal);
}

// Calculate face center (centroid)
Vector3 CalculateFaceCenter(const std::vector<Vector3> &vertices) {
	Vector3 sum = {0, 0, 0};
	for (const auto &v : vertices) {
		sum = Vector3Add(sum, v);
	}
	return Vector3Scale(sum, 1.0f / vertices.size());
}

// Get color based on face HP state
Color GetFaceColor(const PolyFace &face) {
	if (face.isDestroyed) {
		return BLACK;
	}

	if (face.isStructure) {
		return Color {255, 165, 0, 243}; // ORANGE with alpha
	}

	// Armor color based on percentage
	float percentage = (float)face.currentHP / face.maxHP;
	if (percentage >= 0.80f)
		return Color {255, 255, 255, 243};
	if (percentage >= 0.60f)
		return Color {211, 211, 211, 243}; // LIGHTGRAY
	if (percentage >= 0.40f)
		return Color {130, 130, 130, 243}; // GRAY
	if (percentage >= 0.20f)
		return Color {80, 80, 80, 243}; // DARKGRAY
	return Color {50, 50, 50, 243};
}

// Add triangle to mesh
void AddTriangleToMesh(Mesh &mesh, int &idx, Vector3 v0, Vector3 v1, Vector3 v2, Vector3 normal, Color color) {
	// Vertex 0
	mesh.vertices[idx * 3 + 0] = v0.x;
	mesh.vertices[idx * 3 + 1] = v0.y;
	mesh.vertices[idx * 3 + 2] = v0.z;
	mesh.normals[idx * 3 + 0] = normal.x;
	mesh.normals[idx * 3 + 1] = normal.y;
	mesh.normals[idx * 3 + 2] = normal.z;
	mesh.colors[idx * 4 + 0] = color.r;
	mesh.colors[idx * 4 + 1] = color.g;
	mesh.colors[idx * 4 + 2] = color.b;
	mesh.colors[idx * 4 + 3] = color.a;

	// Vertex 1
	mesh.vertices[(idx + 1) * 3 + 0] = v1.x;
	mesh.vertices[(idx + 1) * 3 + 1] = v1.y;
	mesh.vertices[(idx + 1) * 3 + 2] = v1.z;
	mesh.normals[(idx + 1) * 3 + 0] = normal.x;
	mesh.normals[(idx + 1) * 3 + 1] = normal.y;
	mesh.normals[(idx + 1) * 3 + 2] = normal.z;
	mesh.colors[(idx + 1) * 4 + 0] = color.r;
	mesh.colors[(idx + 1) * 4 + 1] = color.g;
	mesh.colors[(idx + 1) * 4 + 2] = color.b;
	mesh.colors[(idx + 1) * 4 + 3] = color.a;

	// Vertex 2
	mesh.vertices[(idx + 2) * 3 + 0] = v2.x;
	mesh.vertices[(idx + 2) * 3 + 1] = v2.y;
	mesh.vertices[(idx + 2) * 3 + 2] = v2.z;
	mesh.normals[(idx + 2) * 3 + 0] = normal.x;
	mesh.normals[(idx + 2) * 3 + 1] = normal.y;
	mesh.normals[(idx + 2) * 3 + 2] = normal.z;
	mesh.colors[(idx + 2) * 4 + 0] = color.r;
	mesh.colors[(idx + 2) * 4 + 1] = color.g;
	mesh.colors[(idx + 2) * 4 + 2] = color.b;
	mesh.colors[(idx + 2) * 4 + 3] = color.a;
}

// Vertex generation functions
std::vector<Vector3> GetCubeVertices(float size) {
	float h = size / 2.0f;
	return {
	    {-h, -h, -h}, // 0
	    {h, -h, -h},  // 1
	    {h, h, -h},   // 2
	    {-h, h, -h},  // 3
	    {-h, -h, h},  // 4
	    {h, -h, h},   // 5
	    {h, h, h},    // 6
	    {-h, h, h}    // 7
	};
}

std::vector<Vector3> GetPyramidVertices(float size) {
	float h = size / 2.0f;
	float height = size * 1.2f;
	return {
	    {-h, 0, -h},   // 0 - base
	    {h, 0, -h},    // 1
	    {h, 0, h},     // 2
	    {-h, 0, h},    // 3
	    {0, height, 0} // 4 - apex
	};
}

std::vector<Vector3> GetOctahedronVertices(float size) {
	return {
	    {0, size, 0},  // 0 - top
	    {size, 0, 0},  // 1
	    {0, 0, size},  // 2
	    {-size, 0, 0}, // 3
	    {0, 0, -size}, // 4
	    {0, -size, 0}  // 5 - bottom
	};
}

std::vector<Vector3> GetDodecahedronVertices(float size) {
	const float phi = (1.0f + sqrtf(5.0f)) / 2.0f; // Golden ratio
	const float a = size / 2.0f;
	const float b = a / phi;
	const float c = a * phi;

	return {
	    {a, a, a}, {a, a, -a}, {a, -a, a}, {a, -a, -a}, {-a, a, a}, {-a, a, -a}, {-a, -a, a}, {-a, -a, -a}, {0, b, c}, {0, b, -c}, {0, -b, c}, {0, -b, -c}, {b, c, 0}, {b, -c, 0}, {-b, c, 0}, {-b, -c, 0}, {c, 0, b}, {c, 0, -b}, {-c, 0, b}, {-c, 0, -b}};
}

std::vector<Vector3> GetTriangularPrismVertices(float size) {
	float h = size / 2.0f;
	float r = size * 0.577f; // Radius of circumscribed circle
	return {
	    {0, h, r},                    // 0 - top front
	    {-r * 0.866f, h, -r * 0.5f},  // 1 - top left
	    {r * 0.866f, h, -r * 0.5f},   // 2 - top right
	    {0, -h, r},                   // 3 - bottom front
	    {-r * 0.866f, -h, -r * 0.5f}, // 4 - bottom left
	    {r * 0.866f, -h, -r * 0.5f}   // 5 - bottom right
	};
}

std::vector<Vector3> GetPentagonalPrismVertices(float size) {
	float h = size / 2.0f;
	float r = size * 0.5f;
	std::vector<Vector3> vertices;

	// Top pentagon
	for (int i = 0; i < 5; i++) {
		float angle = i * (2.0f * PI / 5.0f) - PI / 2.0f;
		vertices.push_back({r * cosf(angle), h, r * sinf(angle)});
	}

	// Bottom pentagon
	for (int i = 0; i < 5; i++) {
		float angle = i * (2.0f * PI / 5.0f) - PI / 2.0f;
		vertices.push_back({r * cosf(angle), -h, r * sinf(angle)});
	}

	return vertices;
}

std::vector<Vector3> GetIcosahedronVertices(float size) {
	const float phi = (1.0f + sqrtf(5.0f)) / 2.0f;
	const float a = size / 2.0f;
	const float b = a * phi;

	return {
	    {0, a, b}, {0, a, -b}, {0, -a, b}, {0, -a, -b}, {a, b, 0}, {a, -b, 0}, {-a, b, 0}, {-a, -b, 0}, {b, 0, a}, {b, 0, -a}, {-b, 0, a}, {-b, 0, -a}};
}

std::vector<Vector3> GetTruncatedPyramidVertices(float size) {
	float h = size / 2.0f;
	float height = size * 1.0f;
	float topScale = 0.5f; // Top is half the size of bottom

	return {
	    {-h, 0, -h},                            // 0 - base
	    {h, 0, -h},                             // 1
	    {h, 0, h},                              // 2
	    {-h, 0, h},                             // 3
	    {-h * topScale, height, -h * topScale}, // 4 - top
	    {h * topScale, height, -h * topScale},  // 5
	    {h * topScale, height, h * topScale},   // 6
	    {-h * topScale, height, h * topScale}   // 7
	};
}

// Factory function
PolyhedronData CreatePolyhedron(PolyhedronShape shape, int weightClass) {
	PolyhedronData poly;
	poly.shape = shape;
	poly.scale = Vector3 {1.0f, 1.0f, 1.0f};

	// Base HP per face (scale by weight class)
	int baseHP = 50;
	switch (weightClass) {
		case 0:
			baseHP = 30;
			break; // LIGHT
		case 1:
			baseHP = 50;
			break; // MEDIUM
		case 2:
			baseHP = 80;
			break; // HEAVY
		case 3:
			baseHP = 120;
			break; // ASSAULT
	}

	float size = 1.0f;
	std::vector<Vector3> vertices;

	switch (shape) {
		case PolyhedronShape::CUBE: {
			vertices = GetCubeVertices(size);
			// 6 faces (counterclockwise winding)
			poly.faces.push_back(CreateFace({vertices[0], vertices[1], vertices[2], vertices[3]}, baseHP)); // Front
			poly.faces.push_back(CreateFace({vertices[5], vertices[4], vertices[7], vertices[6]}, baseHP)); // Back
			poly.faces.push_back(CreateFace({vertices[4], vertices[0], vertices[3], vertices[7]}, baseHP)); // Left
			poly.faces.push_back(CreateFace({vertices[1], vertices[5], vertices[6], vertices[2]}, baseHP)); // Right
			poly.faces.push_back(CreateFace({vertices[3], vertices[2], vertices[6], vertices[7]}, baseHP)); // Top
			poly.faces.push_back(CreateFace({vertices[4], vertices[5], vertices[1], vertices[0]}, baseHP)); // Bottom
			break;
		}

		case PolyhedronShape::PYRAMID: {
			vertices = GetPyramidVertices(size);
			// 5 faces
			poly.faces.push_back(CreateFace({vertices[0], vertices[1], vertices[2], vertices[3]}, baseHP)); // Base
			poly.faces.push_back(CreateFace({vertices[0], vertices[4], vertices[1]}, baseHP));              // Side 1
			poly.faces.push_back(CreateFace({vertices[1], vertices[4], vertices[2]}, baseHP));              // Side 2
			poly.faces.push_back(CreateFace({vertices[2], vertices[4], vertices[3]}, baseHP));              // Side 3
			poly.faces.push_back(CreateFace({vertices[3], vertices[4], vertices[0]}, baseHP));              // Side 4
			break;
		}

		case PolyhedronShape::OCTAHEDRON: {
			vertices = GetOctahedronVertices(size);
			// 8 faces
			poly.faces.push_back(CreateFace({vertices[0], vertices[1], vertices[2]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[0], vertices[2], vertices[3]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[0], vertices[3], vertices[4]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[0], vertices[4], vertices[1]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[5], vertices[2], vertices[1]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[5], vertices[3], vertices[2]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[5], vertices[4], vertices[3]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[5], vertices[1], vertices[4]}, baseHP));
			break;
		}

		case PolyhedronShape::DODECAHEDRON: {
			vertices = GetDodecahedronVertices(size);
			// 12 pentagonal faces (simplified - using triangulation)
			poly.faces.push_back(CreateFace({vertices[0], vertices[8], vertices[10], vertices[2], vertices[16]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[0], vertices[16], vertices[17], vertices[1], vertices[12]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[0], vertices[12], vertices[14], vertices[4], vertices[8]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[1], vertices[17], vertices[3], vertices[11], vertices[9]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[1], vertices[9], vertices[5], vertices[14], vertices[12]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[2], vertices[10], vertices[6], vertices[18], vertices[4]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[2], vertices[4], vertices[14], vertices[5], vertices[13]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[2], vertices[13], vertices[15], vertices[7], vertices[16]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[3], vertices[17], vertices[16], vertices[7], vertices[19]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[3], vertices[19], vertices[5], vertices[9], vertices[11]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[6], vertices[10], vertices[8], vertices[4], vertices[18]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[7], vertices[15], vertices[13], vertices[5], vertices[19]}, baseHP));
			break;
		}

		case PolyhedronShape::TRIANGULAR_PRISM: {
			vertices = GetTriangularPrismVertices(size);
			// 5 faces (2 triangular, 3 rectangular)
			poly.faces.push_back(CreateFace({vertices[0], vertices[1], vertices[2]}, baseHP));              // Top
			poly.faces.push_back(CreateFace({vertices[3], vertices[5], vertices[4]}, baseHP));              // Bottom
			poly.faces.push_back(CreateFace({vertices[0], vertices[3], vertices[4], vertices[1]}, baseHP)); // Side 1
			poly.faces.push_back(CreateFace({vertices[1], vertices[4], vertices[5], vertices[2]}, baseHP)); // Side 2
			poly.faces.push_back(CreateFace({vertices[2], vertices[5], vertices[3], vertices[0]}, baseHP)); // Side 3
			break;
		}

		case PolyhedronShape::PENTAGONAL_PRISM: {
			vertices = GetPentagonalPrismVertices(size);
			// 7 faces (2 pentagonal, 5 rectangular)
			poly.faces.push_back(CreateFace({vertices[0], vertices[1], vertices[2], vertices[3], vertices[4]}, baseHP)); // Top
			poly.faces.push_back(CreateFace({vertices[5], vertices[9], vertices[8], vertices[7], vertices[6]}, baseHP)); // Bottom
			for (int i = 0; i < 5; i++) {
				int next = (i + 1) % 5;
				poly.faces.push_back(CreateFace(
				    {vertices[i], vertices[next], vertices[next + 5], vertices[i + 5]}, baseHP)); // Sides
			}
			break;
		}

		case PolyhedronShape::ICOSAHEDRON: {
			vertices = GetIcosahedronVertices(size);
			// 20 triangular faces
			poly.faces.push_back(CreateFace({vertices[0], vertices[8], vertices[4]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[0], vertices[4], vertices[6]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[0], vertices[6], vertices[10]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[0], vertices[10], vertices[2]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[0], vertices[2], vertices[8]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[1], vertices[9], vertices[5]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[1], vertices[5], vertices[7]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[1], vertices[7], vertices[11]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[1], vertices[11], vertices[3]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[1], vertices[3], vertices[9]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[2], vertices[10], vertices[7]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[2], vertices[7], vertices[5]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[2], vertices[5], vertices[8]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[3], vertices[11], vertices[6]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[3], vertices[6], vertices[4]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[3], vertices[4], vertices[9]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[4], vertices[8], vertices[5]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[4], vertices[5], vertices[9]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[6], vertices[11], vertices[7]}, baseHP));
			poly.faces.push_back(CreateFace({vertices[6], vertices[7], vertices[10]}, baseHP));
			break;
		}

		case PolyhedronShape::TRUNCATED_PYRAMID: {
			vertices = GetTruncatedPyramidVertices(size);
			// 5 faces (2 squares, 4 trapezoids)
			poly.faces.push_back(CreateFace({vertices[0], vertices[1], vertices[2], vertices[3]}, baseHP)); // Bottom
			poly.faces.push_back(CreateFace({vertices[4], vertices[7], vertices[6], vertices[5]}, baseHP)); // Top
			poly.faces.push_back(CreateFace({vertices[0], vertices[4], vertices[5], vertices[1]}, baseHP)); // Side 1
			poly.faces.push_back(CreateFace({vertices[1], vertices[5], vertices[6], vertices[2]}, baseHP)); // Side 2
			poly.faces.push_back(CreateFace({vertices[2], vertices[6], vertices[7], vertices[3]}, baseHP)); // Side 3
			poly.faces.push_back(CreateFace({vertices[3], vertices[7], vertices[4], vertices[0]}, baseHP)); // Side 4
			break;
		}
	}

	poly.GenerateMesh();
	return poly;
}

// Generate mesh from faces
void PolyhedronData::GenerateMesh() {
	// Count total vertices (triangulate all faces)
	int totalVertices = 0;
	for (const auto &face : faces) {
		totalVertices += (face.vertices.size() - 2) * 3; // Fan triangulation
	}

	mesh = Mesh {};
	mesh.vertexCount = totalVertices;
	mesh.triangleCount = totalVertices / 3;

	// Allocate mesh data
	mesh.vertices = (float *)MemAlloc(totalVertices * 3 * sizeof(float));
	mesh.normals = (float *)MemAlloc(totalVertices * 3 * sizeof(float));
	mesh.colors = (unsigned char *)MemAlloc(totalVertices * 4 * sizeof(unsigned char));

	int idx = 0;
	for (size_t faceIdx = 0; faceIdx < faces.size(); faceIdx++) {
		const PolyFace &face = faces[faceIdx];
		Color faceColor = GetFaceColor(face);

		// Triangulate face (fan triangulation from vertex 0)
		for (size_t i = 1; i < face.vertices.size() - 1; i++) {
			AddTriangleToMesh(mesh, idx, face.vertices[0], face.vertices[i], face.vertices[i + 1], face.normal,
			                  faceColor);
			idx += 3;
		}
	}

	UploadMesh(&mesh, false);
	model = LoadModelFromMesh(mesh);

	// Enable transparency
	model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = Color {255, 255, 255, 243};
}

// Update mesh colors based on current HP
void PolyhedronData::UpdateFaceColors() {
	if (mesh.colors == nullptr)
		return;

	int idx = 0;
	for (size_t faceIdx = 0; faceIdx < faces.size(); faceIdx++) {
		const PolyFace &face = faces[faceIdx];
		Color faceColor = GetFaceColor(face);

		// Update color for all triangles in this face
		int numTriangles = face.vertices.size() - 2;
		for (int t = 0; t < numTriangles; t++) {
			for (int v = 0; v < 3; v++) {
				mesh.colors[(idx + v) * 4 + 0] = faceColor.r;
				mesh.colors[(idx + v) * 4 + 1] = faceColor.g;
				mesh.colors[(idx + v) * 4 + 2] = faceColor.b;
				mesh.colors[(idx + v) * 4 + 3] = faceColor.a;
			}
			idx += 3;
		}
	}

	UpdateMeshBuffer(mesh, 3, mesh.colors, mesh.vertexCount * 4 * sizeof(unsigned char), 0);
}

// Project face to 2D net position (simplified grid layout)
Vector2 PolyhedronData::ProjectFaceToNet(int faceIndex) {
	int cols = sqrt(faces.size()) + 1;
	int row = faceIndex / cols;
	int col = faceIndex % cols;

	float spacing = 50.0f;
	return Vector2 {col * spacing + 25, row * spacing + 25};
}
