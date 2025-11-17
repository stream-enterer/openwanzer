#ifndef OPENWANZER_POLYHEDRON_RENDERER_HPP
#define OPENWANZER_POLYHEDRON_RENDERER_HPP

#include "GameState.hpp"
#include "Polyhedron.hpp"
#include "Hex.hpp"
#include "rl/raylib.h"
#include "rl/raymath.h"

struct TrackballCamera {
	Camera3D camera;
	Vector2 previousMousePos;
	Vector2 angularVelocity; // Radians per second
	float friction;          // Decay factor (0.9 = 10% decay per frame)
	bool isDragging;

	void Update(Rectangle viewport);
	void Rotate(Vector2 delta); // Convert mouse delta to rotation
};

struct PolyhedronView {
	TrackballCamera trackball;
	bool lockedToGridView;
	RenderTexture2D renderTarget;

	void Render(const PolyhedronData &poly, Rectangle viewport, bool drawNumbers);
	void DrawFaceNumbers(const PolyhedronData &poly);
	void Initialize(int width, int height);
	void Cleanup();
};

// Net view rendering
struct NetView {
	RenderTexture2D renderTarget;
	int hoveredFaceIndex;

	void Render(const PolyhedronData &poly, Rectangle viewport);
	Vector2 GetFacePosition(const PolyhedronData &poly, int faceIndex);
	void Initialize(int width, int height);
	void Cleanup();
	int DetectHoveredFace(const PolyhedronData &poly, Vector2 mousePos, Rectangle viewport);
};

// 3D world conversion
Vector3 HexToWorld3D(HexCoord coord, const Layout &layout);

// Debug visualization
void RenderDebugFaceNormals(const PolyhedronData &poly, Vector3 worldPos);
void HighlightVisibleFaces(const PolyhedronData &attacker, const PolyhedronData &defender, Vector3 attackerPos,
                           Vector3 defenderPos);

// Face-to-face combat helpers (declared here, defined in Combat.cpp)
std::vector<int> CalculateHittableFaces(const PolyhedronData &attackerPoly, const PolyhedronData &defenderPoly,
                                        HexCoord attackerPos, HexCoord defenderPos, int attackerFacing,
                                        int defenderFacing);

#endif // OPENWANZER_POLYHEDRON_RENDERER_HPP
