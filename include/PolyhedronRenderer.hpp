#ifndef OPENWANZER_POLYHEDRON_RENDERER_HPP
#define OPENWANZER_POLYHEDRON_RENDERER_HPP

#include <vector>
#include "Hex.hpp"
#include "HexCoord.hpp"
#include "Polyhedron.hpp"
#include "rl/raylib.h"
#include "rl/raymath.h"

// 2D face representation for net view
struct FacePosition2D {
	std::vector<Vector2> vertices;
};

// Alias for backward compatibility
using Face2D = FacePosition2D;

// Edge in 3D space
struct Edge3D {
	Vector3 v1, v2;
};

// Complete net layout
struct NetLayout {
	std::vector<FacePosition2D> positions;
	Rectangle boundingBox;
};

// Net layout with optimal rotation
struct NetLayoutWithRotation {
	NetLayout net;
	float rotation;    // Degrees
	float scaleFactor; // Uniform scale
};

// Pre-computed net template for a single face
struct NetFaceTemplate {
	Vector2 position; // Position in net coordinate space
	float rotation;   // Rotation in degrees
};

// Pre-computed net template for a complete shape
struct NetTemplate {
	std::vector<NetFaceTemplate> faceLayouts;
	float defaultScale; // Default scale factor for this shape
};

struct TrackballCamera {
	Camera3D camera;
	Vector2 previousMousePos;
	Vector2 dragStartPos;    // Where the drag started (for detecting drag vs click)
	Vector2 angularVelocity; // Radians per second
	float friction;          // Decay factor (0.9 = 10% decay per frame)
	bool isDragging;

	void Update(Rectangle viewport, bool panelIsDragging = false);
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

// Net view helper functions
bool FacesShareEdge(const PolyFace &f1, const PolyFace &f2);
Edge3D FindSharedEdge(const PolyFace &f1, const PolyFace &f2);
FacePosition2D ProjectFaceToPlane(const PolyFace &face);
Vector2 RotatePoint(Vector2 point, float angleRad);
Vector2 FindVertex2D(const FacePosition2D &face2D, const PolyFace &face3D, Vector3 vertex3D);
float CalculateAlignmentAngle(Vector2 edgeStart1, Vector2 edgeEnd1, Vector2 edgeStart2, Vector2 edgeEnd2);
NetLayout GenerateContiguousNet(const PolyhedronData &poly);
NetLayoutWithRotation OptimizeNetOrientation(const NetLayout &net, Rectangle viewport);
bool PointInPolygon(Vector2 point, const std::vector<Vector2> &polygon);

// Pre-computed net template functions
NetTemplate GetNetTemplate(PolyhedronShape shape);
NetLayout GenerateNetFromTemplate(const PolyhedronData &poly);

#endif // OPENWANZER_POLYHEDRON_RENDERER_HPP
