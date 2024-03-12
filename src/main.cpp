#include <libtcod.hpp>

int main(int argc, char **argv) {
    TCODConsole::initRoot(80, 50, "libtcod C++ tutorial", false);
    while(!TCODConsole::isWindowClosed()) {
        TCOD_key_t key;
        TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS, &key, nullptr);
        TCODConsole::root->clear();
        TCODConsole::root->putChar(40, 25, '@');
        TCODConsole::flush();
    }
    return 0;
}
