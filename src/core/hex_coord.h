#ifndef OPENWANZER_CORE_HEX_COORD_H
#define OPENWANZER_CORE_HEX_COORD_H

struct HexCoord {
  int row;
  int col;

  bool operator==(const HexCoord &other) const {
    return row == other.row && col == other.col;
  }
};

#endif // OPENWANZER_CORE_HEX_COORD_H
