#ifndef __HPP_MAP__
#define __HPP_MAP__

#include <array>
#include <libtcod.hpp>

struct Tile {
  bool canWalk;
  Tile() : Tile(true) {}
  Tile(bool canWalk) : canWalk(canWalk) {}
};

struct MapSettings {
  const tcod::ColorRGB darkWall;
  const tcod::ColorRGB darkGround;

  MapSettings() : darkWall({0, 0, 100}), darkGround({50, 50, 150}) {}
};


template<size_t W, size_t H>
class Map {
 public:
  Map() = default;
  ~Map() = default;

  bool isWall(int x, int y) const { return tiles[x * H + y].canWalk; }
  void render(tcod::Console console) const {
    static const MapSettings& mapSettings;

    for (int x = 0; x < W; x++) {
      for (int y = 0; y < H; y++) {
        if (isWall(x, y)) {
          tcod::print(console, {x, y}, "", std::nullopt, mapSettings.darkWall);
        } else {
          tcod::print(console, {x, y}, "", std::nullopt, mapSettings.darkGround);
        }
      }
    }
  }

  void setWall(int x, int y, bool b) { tiles[x * H + y].canWalk = b; }

 protected:
  std::array<Tile, W * H> tiles;
};

#endif /* __HPP_MAP__ */
