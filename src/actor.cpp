#include "actor.hpp"

Actor::Actor(int x, int y, const std::string& str, const tcod::ColorRGB& color) : x(x), y(y), str(str), color(color) {}

void Actor::render(tcod::Console& con) const { tcod::print(con, {x, y}, str, color, std::nullopt); }
