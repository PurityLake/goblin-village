#ifndef __HPP_ACTOR__
#define __HPP_ACTOR__

#include <libtcod.hpp>

class Actor {
 public:
  int x, y;
  std::string str;
  tcod::ColorRGB color;

  Actor(int x, int y, const std::string &str, const tcod::ColorRGB& color);
  void render(tcod::Console& con) const;
};

#endif /* __HPP_ACTOR__ */
