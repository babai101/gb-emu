#pragma once
#include "common.hpp"
#include <vector>
#define LCDC 0xFF40
#define STAT 0xFF41
#define SCY 0xFF42
#define SCX 0xFF43
#define LY 0xFF44
#define LYC 0xFF45
#define WY 0xFF4A
#define BGP 0xFF47
#define OBP0 0xFF48
#define OBP1 0xFF49

namespace PPU
{
    std::vector<std::vector<std::vector<u8>>> build_chrgrid(u16);
    void build_chrgrid_half(u16, std::vector<std::vector<std::vector<u8>>>&);
    void tick();
    void render_scanline();
    void reset();
    void render_offscreen_buffer();
} // namespace PPU