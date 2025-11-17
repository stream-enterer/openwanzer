#ifndef OPENWANZER_POLYHEDRON_HPP
#define OPENWANZER_POLYHEDRON_HPP

#include <vector>
#include "rl/raylib.h"
#include "rl/raymath.h"

enum class PolyhedronShape {
	PYRAMID,          // 5 faces (4 triangular + 1 square base)
	CUBE,             // 6 faces
	OCTAHEDRON,       // 8 faces
	DODECAHEDRON,     // 12 faces
	TRIANGULAR_PRISM, // 5 faces
	PENTAGONAL_PRISM, // 7 faces
	ICOSAHEDRON,      // 20 faces
	TRUNCATED_PYRAMID // 5 faces
};

struct PolyFace {
	std::vector<Vector3> vertices; // 3+ vertices defining face
	Vector3 normal;                // Face normal (outward)
	Vector3 center;                // Face center (for text placement)
	int currentHP;                 // Armor HP (or structure if armor=0)
	int maxHP;                     // Max HP for this face
	bool isStructure;              // True if armor depleted
	bool isDestroyed;              // True if structure depleted
};

struct PolyhedronData {
	PolyhedronShape shape;
	std::vector<PolyFace> faces;
	Mesh mesh;     // Raylib mesh for rendering
	Model model;   // Raylib model
	Vector3 scale; // Uniform scale for this shape

	void GenerateMesh();                     // Create mesh from face data
	void UpdateFaceColors();                 // Update mesh colors based on HP
	Vector2 ProjectFaceToNet(int faceIndex); // Get 2D net position
	void VerifyNormals();                    // Ensure all normals point outward
};

// Forward declaration for weight class
enum class WeightClass;

// Factory functions
PolyhedronData CreatePolyhedron(PolyhedronShape shape, int weightClass);
std::vector<Vector3> GetPyramidVertices(float size);
std::vector<Vector3> GetCubeVertices(float size);
std::vector<Vector3> GetOctahedronVertices(float size);
std::vector<Vector3> GetDodecahedronVertices(float size);
std::vector<Vector3> GetTriangularPrismVertices(float size);
std::vector<Vector3> GetPentagonalPrismVertices(float size);
std::vector<Vector3> GetIcosahedronVertices(float size);
std::vector<Vector3> GetTruncatedPyramidVertices(float size);

// Helper functions
Vector3 CalculateFaceNormal(const std::vector<Vector3> &vertices);
Vector3 CalculateFaceCenter(const std::vector<Vector3> &vertices);
Color GetFaceColor(const PolyFace &face);
void AddTriangleToMesh(Mesh &mesh, int &idx, Vector3 v0, Vector3 v1, Vector3 v2, Vector3 normal, Color color);

// Create face helper
PolyFace CreateFace(const std::vector<Vector3> &vertices, int baseHP);

#endif // OPENWANZER_POLYHEDRON_HPP
