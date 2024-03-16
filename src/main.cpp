#include <SDL.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <libtcod.hpp>

#include <AL/al.h>
#include <AL/alc.h>

#include "error.hpp"

auto get_data_dir() -> std::filesystem::path {
  static auto root_dir = std::filesystem::path{"."};

  while (!std::filesystem::exists(root_dir / "data")) {
    root_dir /= "..";
    if (!std::filesystem::exists(root_dir)) {
      throw std::runtime_error("Could not find the data directory.");
    }
  }

  return root_dir / "data";
}

static constexpr auto WHITE = tcod::ColorRGB{255, 255, 255};

static tcod::Console g_console;
static tcod::Context g_context;

struct Player {
  int x, y;
} player{10, 10};

void main_loop() {
  g_console.clear();
  tcod::print(g_console, {player.x, player.y}, "@", {{255, 255, 255}}, {{0, 0, 0}});
  g_context.present(g_console);

  // handle event
  SDL_Event event;
#ifndef __EMSCRIPTEN__
  SDL_WaitEvent(nullptr);
#endif
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
#ifndef __EMSCRIPTEN__
        throw QuitRequest{};
#endif
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_UP:
            player.y--;
            break;
          case SDLK_DOWN:
            player.y++;
            break;
          case SDLK_LEFT:
            player.x--;
            break;
          case SDLK_RIGHT:
            player.x++;
            break;
        }
    }
  }
}

int main(int argc, char** argv) {
  ALCdevice* device;
  device = alcOpenDevice(nullptr);
  if (!device) {
    return EXIT_FAILURE;
  }

  try {
    g_console = tcod::Console{70, 40};
    auto tileset = tcod::load_tilesheet(get_data_dir() / "dejavu16x16_gs_tc.png", {32, 8}, tcod::CHARMAP_TCOD);
    // configure the context.
    auto params = TCOD_ContextParams{};
    params.window_title = "Libtcod Project";
    params.tcod_version = TCOD_COMPILEDVERSION;
    params.argc = argc;
    params.argv = argv;
    params.renderer_type = TCOD_RENDERER_SDL2;
    params.sdl_window_flags = SDL_WINDOW_RESIZABLE;
    params.vsync = true;
    params.pixel_width = 800;
    params.pixel_height = 600;
    params.tileset = tileset.get();
    params.console = g_console.get();

    g_context = tcod::Context{params};

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 0);
#else
    while (true) {
      main_loop();
    }
#endif
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (const QuitRequest& e) {
  }

  alcCloseDevice(device);

  return EXIT_SUCCESS;
}
