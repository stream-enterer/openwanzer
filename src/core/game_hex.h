#ifndef OPENWANZER_CORE_GAME_HEX_H
#define OPENWANZER_CORE_GAME_HEX_H

#include "hex_coord.h"
#include "enums.h"

struct GameHex {
  HexCoord coord;
  TerrainType terrain;
  int owner; // -1 = neutral, 0 = axis, 1 = allied
  bool isVictoryHex;
  bool isDeployment;
  bool isSpotted[2]; // spotted by each side
  int zoc[2];        // zone of control counter per side
  bool isMoveSel;    // highlighted for movement
  bool isAttackSel;  // highlighted for attack

  GameHex()
      : terrain(TerrainType::PLAINS), owner(-1), isVictoryHex(false),
        isDeployment(false), isMoveSel(false), isAttackSel(false) {
    isSpotted[0] = false;
    isSpotted[1] = false;
    zoc[0] = 0;
    zoc[1] = 0;
  }

  void setZOC(int side, bool on) {
    if (on) {
      zoc[side]++;
    } else if (zoc[side] > 0) {
      zoc[side]--;
    }
  }

  bool isZOC(int side) const { return zoc[side] > 0; }
};

#endif // OPENWANZER_CORE_GAME_HEX_H
