#pragma once
#include "bootloader.hpp"
#include "common.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

namespace GUI
{
    // const int SCREEN_WIDTH = 480;
    // const int SCREEN_HEIGHT = 432;
    const int SCREEN_WIDTH = 160;
    const int SCREEN_HEIGHT = 144;
    extern u32 off_screen_buffer [23040];
    void update_texture(u32 *);
    bool init(char *);
    int run();

} // namespace GUI