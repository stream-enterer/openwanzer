#ifndef OPENWANZER_HEX_COORD_HPP
#define OPENWANZER_HEX_COORD_HPP

struct HexCoord {
  int row;
  int col;

  bool operator==(const HexCoord &other) const {
    return row == other.row && col == other.col;
  }
};

#endif // OPENWANZER_HEX_COORD_HPP
