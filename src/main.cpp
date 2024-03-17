/* C++ */
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <iostream>

/* custom */
#include "error.hpp"

/* libtco d*/
#include <libtcod.hpp>
#include <SDL.h>

/* OpenAL */
#include <AL/al.h>
#include <AL/alc.h>

/* OGG Vorbis */
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#ifdef _WIN32
# ifdef _MSC_VER
#  include <fcntl.h>
#  include <io.h>
# else
#  include <clocale>
#  include <locale>
# endif
#endif

#if defined(__MACOS__) && defined(__MWERKS__)
# include <console.h>
#endif

char buffer[4096];
int convsize = 4096;

static constexpr auto WHITE = tcod::ColorRGB{255, 255, 255};

static tcod::Console g_console;
static tcod::Context g_context;

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

struct Player {
  int x, y;
} player{10, 10};

auto main_loop() -> void {
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

static auto list_audio_devices(const ALCchar* devices) -> void {
  const ALCchar *device = devices, *next = devices + 1;
  size_t len = 0;

  std::cout << "Devices list:\n";
  std::cout << "-------------\n";
  while (device && *device != '\0' && next && *next != '\0') {
    std::fprintf(stdout, "%s\n", device);
    len = std::strlen(device);
    device += (len + 1);
    next += (len + 2);
  }
  std::cout << "-------------\n";
}

auto main(int argc, char** argv) -> int {
  ALCdevice* device;
  ALCcontext* context;
  ALCenum error;

  device = alcOpenDevice(nullptr);
  if (!device) {
    std::cerr << "Failed to create device.\n";
    goto failed;
  }

  context = alcCreateContext(device, nullptr);
  if (!alcMakeContextCurrent(context)) {
    std::cerr << "Failed to set audio context current.\n";
  }
  error = alGetError();
  if (error != AL_NO_ERROR) {
    std::cerr << "Error creating device.\n";
    goto failedCtx;
  }

  ALboolean enumeration;
  enumeration = alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT");
  if (enumeration == AL_FALSE) {
    std::cerr << "Can't enumerate OpenAL Devices.\n";
    goto failedDev;
  }

  list_audio_devices(alcGetString(nullptr, ALC_DEVICE_SPECIFIER));

#if defined(macintosh) && defined(__MWERKS__)
  {
    int argc;
    char** argcv;
    argc = ccommand(&argv);
  }
#endif
  do {
    auto p = get_data_dir() / "example.ogg";

    OggVorbis_File vf{};
    int eof = 0;
    int current_section = 0;
    const char* audioData = nullptr;

    if (ov_fopen(p.string().c_str(), &vf) < 0) {
      std::cerr << "Input does not appear to be an Ogg file.\n";
      goto failedCtx;
    }

    /* Throw the comments plus a few lines about the bitstream we're
       decoding */
    {
      char** ptr = ov_comment(&vf, -1)->user_comments;
      vorbis_info* vi = ov_info(&vf, -1);
      while (*ptr) {
        fprintf(stderr, "%s\n", *ptr);
        ++ptr;
      }
      fprintf(stderr, "\nBitstream is %d channel, %ldHz\n", vi->channels, vi->rate);
      fprintf(stderr, "\nDecoded length: %ld samples\n", (long)ov_pcm_total(&vf, -1));
      fprintf(stderr, "Encoded by: %s\n\n", ov_comment(&vf, -1)->vendor);
    }

    std::stringstream ss;
    while (!eof) {
      long ret = ov_read(&vf, buffer, sizeof(buffer), 0, 2, 1, &current_section);
      if (ret == 0) {
        eof = 1;
      } else if (ret < 0) {
        if (ret == OV_EBADLINK) {
          std::cerr << "Corrupt butsream section! Exiting.\n";
          goto failedCtx;
        }
      } else {
        ss << buffer;
      }
    }
    auto str = ss.str();
    audioData = str.c_str();
  } while (0);


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
    goto failedCtx;
  } catch (const QuitRequest& e) {
  }

  alcMakeContextCurrent(nullptr);
  alcDestroyContext(context);
  alcCloseDevice(device);

  return EXIT_SUCCESS;

failedCtx:
  alcMakeContextCurrent(nullptr);
  alcDestroyContext(context);

failedDev:
  alcCloseDevice(device);

failed:
  return EXIT_FAILURE;
}
