// Test to understand the hex border drawing issue
#include <iostream>
#include "include/hex.h"

int main() {
    // Simulate a 3x3 movement zone
    // X marks hexes in movement range
    //   X X X
    //    X X X  
    //   X X X
    
    std::cout << "Testing hex neighbor directions:\n";
    
    // Center hex at offset (1, 1)
    OffsetCoord center(1, 1);
    Hex centerCube = offset_to_cube(center);
    
    std::cout << "Center: offset(" << center.col << ", " << center.row << ")\n";
    std::cout << "Center cube: (" << centerCube.q << ", " << centerCube.r << ", " << centerCube.s << ")\n\n";
    
    // Check all 6 neighbors
    for (int dir = 0; dir < 6; dir++) {
        Hex neighborCube = hex_neighbor(centerCube, dir);
        OffsetCoord neighborOffset = cube_to_offset(neighborCube);
        std::cout << "Direction " << dir << ": offset(" << neighborOffset.col << ", " << neighborOffset.row << ")\n";
    }
    
    return 0;
}
