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
  int spotted[2];    // spotting counter per side (team-based FOW)
  int zoc[2];        // zone of control counter per side
  bool isMoveSel;    // highlighted for movement
  bool isAttackSel;  // highlighted for attack

  GameHex()
      : terrain(TerrainType::PLAINS), owner(-1), isVictoryHex(false),
        isDeployment(false), isMoveSel(false), isAttackSel(false) {
    spotted[0] = 0;
    spotted[1] = 0;
    zoc[0] = 0;
    zoc[1] = 0;
  }

  void setSpotted(int side, bool on) {
    if (on) {
      spotted[side]++;
    } else if (spotted[side] > 0) {
      spotted[side]--;
    }
  }

  void setZOC(int side, bool on) {
    if (on) {
      zoc[side]++;
    } else if (zoc[side] > 0) {
      zoc[side]--;
    }
  }

  bool isSpotted(int side) const { return spotted[side] > 0; }
  bool isZOC(int side) const { return zoc[side] > 0; }
};

#endif // OPENWANZER_CORE_GAME_HEX_H
