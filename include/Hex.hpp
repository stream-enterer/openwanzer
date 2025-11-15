#ifndef OPENWANZER_HEX_HPP
#define OPENWANZER_HEX_HPP

#include <cmath>
#include <vector>
#include <algorithm>

// Hexagon library based on Red Blob Games implementation
// https://www.redblobgames.com/grids/hexagons/

// Forward declarations
struct Hex;
struct FractionalHex;
struct OffsetCoord;
struct Point;

// ============================================================================
// Cube coordinate system for hexagons
// ============================================================================
struct Hex {
    int q, r, s;

    Hex(int q_, int r_, int s_) : q(q_), r(r_), s(s_) {
        if (q + r + s != 0) {
            // This should never happen in valid hex coordinates
            // Cube coordinates must satisfy q + r + s = 0
        }
    }

    Hex(int q_, int r_) : q(q_), r(r_), s(-q_ - r_) {}

    Hex() : q(0), r(0), s(0) {}

    bool operator==(const Hex& other) const {
        return q == other.q && r == other.r && s == other.s;
    }

    bool operator!=(const Hex& other) const {
        return !(*this == other);
    }

    Hex operator+(const Hex& other) const {
        return Hex(q + other.q, r + other.r, s + other.s);
    }

    Hex operator-(const Hex& other) const {
        return Hex(q - other.q, r - other.r, s - other.s);
    }

    Hex operator*(int k) const {
        return Hex(q * k, r * k, s * k);
    }
};

// ============================================================================
// Fractional hex coordinates (for pixel-to-hex conversion)
// ============================================================================
struct FractionalHex {
    double q, r, s;

    FractionalHex(double q_, double r_, double s_) : q(q_), r(r_), s(s_) {}
    FractionalHex(double q_, double r_) : q(q_), r(r_), s(-q_ - r_) {}
};

// ============================================================================
// Offset coordinates (for traditional row/col access)
// ============================================================================
struct OffsetCoord {
    int col, row;

    OffsetCoord(int col_, int row_) : col(col_), row(row_) {}
    OffsetCoord() : col(0), row(0) {}

    bool operator==(const OffsetCoord& other) const {
        return col == other.col && row == other.row;
    }
};

// ============================================================================
// 2D Point for pixel coordinates
// ============================================================================
struct Point {
    double x, y;

    Point(double x_, double y_) : x(x_), y(y_) {}
    Point() : x(0), y(0) {}
};

// ============================================================================
// Hex directions (6 neighbors)
// ============================================================================
const std::vector<Hex> kHexDirections = {
    Hex(1, 0, -1), Hex(1, -1, 0), Hex(0, -1, 1),
    Hex(-1, 0, 1), Hex(-1, 1, 0), Hex(0, 1, -1)
};

inline Hex HexDirection(int direction) {
    return kHexDirections[direction];
}

inline Hex HexNeighbor(Hex hex, int direction) {
    return hex + HexDirection(direction);
}

// ============================================================================
// Hex distance
// ============================================================================
inline int HexLength(Hex hex) {
    return int((abs(hex.q) + abs(hex.r) + abs(hex.s)) / 2);
}

inline int HexDistance(Hex a, Hex b) {
    return HexLength(a - b);
}

// ============================================================================
// Conversion between cube and offset coordinates
// We use "odd-r" offset (odd rows are shifted right)
// ============================================================================
inline Hex OffsetToCube(OffsetCoord offset) {
    int q = offset.col - (offset.row - (offset.row & 1)) / 2;
    int r = offset.row;
    return Hex(q, r);
}

inline OffsetCoord CubeToOffset(Hex hex) {
    int col = hex.q + (hex.r - (hex.r & 1)) / 2;
    int row = hex.r;
    return OffsetCoord(col, row);
}

// ============================================================================
// Layout and orientation
// ============================================================================
struct Orientation {
    double f0, f1, f2, f3;  // forward matrix
    double b0, b1, b2, b3;  // backward matrix
    double startAngle;      // in multiples of 60 degrees

    Orientation(double f0_, double f1_, double f2_, double f3_,
                double b0_, double b1_, double b2_, double b3_,
                double startAngle_)
        : f0(f0_), f1(f1_), f2(f2_), f3(f3_),
          b0(b0_), b1(b1_), b2(b2_), b3(b3_),
          startAngle(startAngle_) {}
};

// Flat-topped hexagons
const Orientation kLayoutFlat = Orientation(
    3.0 / 2.0, 0.0, sqrt(3.0) / 2.0, sqrt(3.0),
    2.0 / 3.0, 0.0, -1.0 / 3.0, sqrt(3.0) / 3.0,
    0.0
);

// Pointy-topped hexagons
const Orientation kLayoutPointy = Orientation(
    sqrt(3.0), sqrt(3.0) / 2.0, 0.0, 3.0 / 2.0,
    sqrt(3.0) / 3.0, -1.0 / 3.0, 0.0, 2.0 / 3.0,
    0.5
);

struct Layout {
    Orientation orientation;
    Point size;
    Point origin;

    Layout(Orientation orientation_, Point size_, Point origin_)
        : orientation(orientation_), size(size_), origin(origin_) {}
};

// ============================================================================
// Pixel to hex and hex to pixel conversion
// ============================================================================
inline Point HexToPixel(Layout layout, Hex h) {
    const Orientation& M = layout.orientation;
    double x = (M.f0 * h.q + M.f1 * h.r) * layout.size.x;
    double y = (M.f2 * h.q + M.f3 * h.r) * layout.size.y;
    return Point(x + layout.origin.x, y + layout.origin.y);
}

inline FractionalHex PixelToHex(Layout layout, Point p) {
    const Orientation& M = layout.orientation;
    Point pt = Point((p.x - layout.origin.x) / layout.size.x,
                     (p.y - layout.origin.y) / layout.size.y);
    double q = M.b0 * pt.x + M.b1 * pt.y;
    double r = M.b2 * pt.x + M.b3 * pt.y;
    return FractionalHex(q, r, -q - r);
}

// ============================================================================
// Rounding fractional hex to integer hex
// ============================================================================
inline Hex HexRound(FractionalHex h) {
    int q = int(round(h.q));
    int r = int(round(h.r));
    int s = int(round(h.s));

    double qDiff = abs(q - h.q);
    double rDiff = abs(r - h.r);
    double sDiff = abs(s - h.s);

    if (qDiff > rDiff && qDiff > sDiff) {
        q = -r - s;
    } else if (rDiff > sDiff) {
        r = -q - s;
    } else {
        s = -q - r;
    }

    return Hex(q, r, s);
}

// ============================================================================
// Hex corner offset (for drawing)
// ============================================================================
inline Point HexCornerOffset(Layout layout, int corner) {
    const Orientation& M = layout.orientation;
    double angle = 2.0 * M_PI * (M.startAngle + corner) / 6.0;
    return Point(layout.size.x * cos(angle), layout.size.y * sin(angle));
}

// Get all 6 corners of a hexagon
inline std::vector<Point> PolygonCorners(Layout layout, Hex h) {
    std::vector<Point> corners;
    Point center = HexToPixel(layout, h);

    for (int i = 0; i < 6; i++) {
        Point offset = HexCornerOffset(layout, i);
        corners.push_back(Point(center.x + offset.x, center.y + offset.y));
    }

    return corners;
}

// ============================================================================
// Line drawing (interpolation between hexes)
// ============================================================================
inline double Lerp(double a, double b, double t) {
    return a + (b - a) * t;
}

inline FractionalHex HexLerp(FractionalHex a, FractionalHex b, double t) {
    return FractionalHex(Lerp(a.q, b.q, t),
                        Lerp(a.r, b.r, t),
                        Lerp(a.s, b.s, t));
}

inline std::vector<Hex> HexLinedraw(Hex a, Hex b) {
    int N = HexDistance(a, b);
    FractionalHex aNudge = FractionalHex(a.q + 1e-6, a.r + 1e-6, a.s - 2e-6);
    FractionalHex bNudge = FractionalHex(b.q + 1e-6, b.r + 1e-6, b.s - 2e-6);
    std::vector<Hex> results;
    double step = 1.0 / std::max(N, 1);
    for (int i = 0; i <= N; i++) {
        results.push_back(HexRound(HexLerp(aNudge, bNudge, step * i)));
    }
    return results;
}

// ============================================================================
// Range (all hexes within distance N)
// ============================================================================
inline std::vector<Hex> HexRange(Hex center, int N) {
    std::vector<Hex> results;
    for (int q = -N; q <= N; q++) {
        for (int r = std::max(-N, -q - N); r <= std::min(N, -q + N); r++) {
            results.push_back(center + Hex(q, r, -q - r));
        }
    }
    return results;
}

#endif // OPENWANZER_HEX_HPP
