#ifndef OPENWANZER_HEXCOORD_H
#define OPENWANZER_HEXCOORD_H

struct HexCoord {
  int row;
  int col;

  bool operator==(const HexCoord &other) const {
    return row == other.row && col == other.col;
  }
};

#endif // OPENWANZER_HEXCOORD_H
