#include <iostream>
#include <cmath>

int main() {
    // For pointy-top hexagons:
    // start_angle = 0.5, so corner 0 is at 30°
    // Corners go clockwise: 30°, 90°, 150°, 210°, 270°, 330°
    
    std::cout << "Corner angles (pointy-top):\n";
    for (int i = 0; i < 6; i++) {
        double angle = 60.0 * (0.5 + i);
        std::cout << "Corner " << i << ": " << angle << "°\n";
    }
    
    std::cout << "\nEdges (from corner i to corner i+1):\n";
    std::cout << "Edge 0 (corners 0→1): 30° to 90° (NE edge)\n";
    std::cout << "Edge 1 (corners 1→2): 90° to 150° (NW edge)\n";
    std::cout << "Edge 2 (corners 2→3): 150° to 210° (W edge)\n";
    std::cout << "Edge 3 (corners 3→4): 210° to 270° (SW edge)\n";
    std::cout << "Edge 4 (corners 4→5): 270° to 330° (SE edge)\n";
    std::cout << "Edge 5 (corners 5→0): 330° to 30° (E edge)\n";
    
    std::cout << "\nHex directions (from hex.h):\n";
    std::cout << "Direction 0: E (0°/360°)\n";
    std::cout << "Direction 1: SE (300°)\n";
    std::cout << "Direction 2: SW (240°)\n";
    std::cout << "Direction 3: W (180°)\n";
    std::cout << "Direction 4: NW (120°)\n";
    std::cout << "Direction 5: NE (60°)\n";
    
    std::cout << "\nCorrect mapping (direction → edge):\n";
    int edgeForDirection[6] = {5, 4, 3, 2, 1, 0};
    for (int dir = 0; dir < 6; dir++) {
        std::cout << "Direction " << dir << " → Edge " << edgeForDirection[dir] << "\n";
    }
    
    return 0;
}
