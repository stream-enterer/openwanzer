#ifndef OPENWANZER_GAME_HEX_HPP
#define OPENWANZER_GAME_HEX_HPP

#include "HexCoord.hpp"
#include "Enums.hpp"

struct GameHex {
  HexCoord coord;
  TerrainType terrain;
  int owner; // -1 = neutral, 0 = axis, 1 = allied
  bool isDeployment;
  int spotted[2];    // spotting counter per side (team-based FOW)
  bool isMoveSel;    // highlighted for movement
  bool isAttackSel;  // highlighted for attack

  GameHex()
      : terrain(TerrainType::PLAINS), owner(-1),
        isDeployment(false), isMoveSel(false), isAttackSel(false) {
    spotted[0] = 0;
    spotted[1] = 0;
  }

  void setSpotted(int side, bool on) {
    if (on) {
      spotted[side]++;
    } else if (spotted[side] > 0) {
      spotted[side]--;
    }
  }

  bool isSpotted(int side) const { return spotted[side] > 0; }
};

#endif // OPENWANZER_GAME_HEX_HPP
