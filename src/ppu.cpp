#include "include/ppu.hpp"
#include "gui.hpp"
#include "cpu.hpp"

namespace PPU
{
    u16 cycle = 0;
    int main_cycles = 0;
    u8 mode = 1;
    int scanline = 144;
    u16 bg_map_offset = 0;
    u16 window_map_offset = 0;
    u16 bg_tile_display_offset = 0;
    u16 window_tile_display_offset = 0;
    u32 screen_pixels[144][160];
    u32 bgTiles_pixesl[256][256];
    u32 colors[4] = {0x009bbc0f, 0x008bac0f, 0x00306230, 0x000f380f}; // 4 shades of green
    std::vector<std::vector<std::vector<u8>>> grid;
    std::vector<std::vector<std::vector<u8>>> window_grid;
    std::vector<std::vector<std::vector<u8>>> sprite_grid;

    struct sprite
    {
        u8 sprite_y;
        u8 sprite_x;
        u8 sprite_index;
        u8 sprite_attr;
    };
    std::vector<sprite> sprites_to_render;
    // DEBUG
    u8 prev_stat = 0;
    // DEBUG
    void tick()
    {
        // DEBUG
        prev_stat = CPU::memory[STAT];
        // if (CPU::memory[JOYP] != 0xCF)
        //     printf("JOYP state changed: %x\n", CPU::memory[JOYP]);
        // DEBUG
        CPU::memory[LY] = scanline;
        if (cycle == 0)
        {
            // printf("LY = %d, LYC = %d\n", CPU::memory[LY], CPU::memory[LYC]);

            if (CPU::memory[LYC] == CPU::memory[LY])
            {
                u8 temp = CPU::memory[STAT] & 0xFB;
                temp |= 0x04;
                CPU::memory[STAT] = temp;
                // STAT interrupt
                if ((CPU::memory[STAT] & 0x40) == 0x40)
                {
                    u8 intrpt = CPU::memory[IF];
                    intrpt &= 0xFD;
                    intrpt |= 0x02;
                    CPU::memory[IF] = intrpt;
                    // printf("STAT Interrupt, LY==LYC..scanline: %d\n", scanline);
                }
            }
            else
                CPU::memory[STAT] = CPU::memory[STAT] & 0xFB;
        }
        if (scanline >= 0 and scanline < 144)
        {
            if (cycle == 0)
            {
                mode = 2;

                // scanning of OAM starts kickoff mode 2
                u8 temp = (CPU::memory[STAT] & 0xFC);
                temp |= 0x02;
                CPU::memory[STAT] = temp;
                // STAT interrupt
                if ((CPU::memory[STAT] & 0x20) == 0x20)
                {
                    u8 intrpt = CPU::memory[IF];
                    intrpt &= 0xFD;
                    intrpt |= 0x02;
                    CPU::memory[IF] = intrpt;
                    // printf("STAT Interrupt, Mode 2 (OAM Scanning) starts..scanline: %d\n", scanline);
                }
            }
            if (cycle == 80)
            {
                // scanning of OAM done, kickoff mode 3
                u8 temp = (CPU::memory[STAT] & 0xFC);
                temp |= 0x03;
                CPU::memory[STAT] = temp;
                mode = 3;
            }
            if (cycle == (80 + 172))
            {
                // rendering pixels done kickoff mode 0 (hblank)
                CPU::memory[STAT] = (CPU::memory[STAT] & 0xFC);
                // STAT interrupt
                if ((CPU::memory[STAT] & 0x08) == 0x08)
                {
                    u8 intrpt = CPU::memory[IF];
                    intrpt &= 0xFD;
                    intrpt |= 0x02;
                    CPU::memory[IF] = intrpt;
                    //printf("STAT Interrupt, Mode 0 (HBLANK) starts..scanline: %d\n", scanline);
                }
                mode = 0;
            }
            if (cycle == 455)
            {
                // scanline finished
                render_scanline();
                if (scanline == 143)
                {
                    // Start of MODE 1
                    u8 intrpt = CPU::memory[IF];
                    u8 temp = (CPU::memory[STAT] & 0xFC);
                    temp |= 0x01;
                    CPU::memory[STAT] = temp;
                    // printf("STAT Set at VBLANK: %x\n", CPU::memory[STAT]);
                    //  STAT interrupt
                    if ((CPU::memory[STAT] & 0x10) == 0x10)
                    {
                        intrpt = CPU::memory[IF];
                        intrpt &= 0xFD;
                        intrpt |= 0x02;
                        CPU::memory[IF] = intrpt;
                        // printf("STAT Interrupt, Mode 1 (VBLANK) starts..scanline: %d\n", scanline);
                    }
                    //  vblank starts
                    //  request interrupt
                    intrpt = CPU::memory[IF];
                    intrpt &= 0xFE;
                    intrpt |= 0x01;
                    CPU::memory[IF] = intrpt;
                    // printf("VBLANK Interrupt, Mode 1 starts..scanline: %d\n", scanline);
                    mode = 1;
                    render_offscreen_buffer();
                }
                else
                {
                    // // scanning of OAM starts kickoff mode 2
                    // u8 temp = CPU::memory[STAT] & 0xFC;
                    // temp |= 0x02;
                    // CPU::memory[STAT] = temp;
                    // // STAT interrupt
                    // if ((CPU::memory[STAT] & 0x20) == 0x20)
                    // {
                    //     u8 intrpt = CPU::memory[IF];
                    //     intrpt &= 0xFD;
                    //     intrpt |= 0x02;
                    //     CPU::memory[IF] = intrpt;
                    //     // printf("STAT Interrupt, Mode 2 (OAM Scanning) starts..scanline: %d\n", scanline);
                    // }
                    // mode = 2;
                }
            }
        }
        else
        {
            // now in vblank
            if (cycle == 455)
            {
                // if (scanline == 153)
                // {
                //     // vblank ends, kick off mode 2
                //     u8 temp = CPU::memory[STAT] & 0xFC;
                //     temp |= 0x02;
                //     CPU::memory[STAT] = temp;
                //     mode = 2;
                // }
                // printf("SCROLLX: %x SCROLL Y: %x\n", CPU::memory[0xFF42], CPU::memory[0xFF43]);
            }
        }
        // printf("cycle: %d\n", cycle);
        cycle++;
        // DEBUG
        if (CPU::memory[STAT] != prev_stat)
        {
            // printf("LCD STAT: %x cycle: %d scanline: %d\n", CPU::memory[STAT], cycle, scanline);
        }
        prev_stat = CPU::memory[STAT];

        // DEBUG
        if (cycle == 456)
        {
            cycle = 0;
            // printf("scanline: %d\n", scanline);
            // printf("LY: %d\n", CPU::memory[LY]);
            scanline++;
            if (scanline == 154)
            {
                scanline = 0;
            }
        }
        main_cycles++;
        // printf("Cycle: %d Scanline: %d Mode: %d\n", cycle, scanline, mode);
    }

    int normalize(int x)
    {
        if (x < 8)
            return x + 8;
        else
            return x;
    }

    // New Scanline Rendering
    void render_background()
    {
        u8 tile_to_fetch = 0;       // The tile to render at current scanline and pixel
        u8 tile_fine_offsetx = 0;   // starting pixel X offset of tile to draw from
        u8 tile_fine_offsety = 0;   // starting pixel Y offset of tile to draw from
        u8 tile_coarse_offsetx = 0; // X position of the tile to draw from
        u8 tile_coarse_offsety = 0; // Y position of the tile to draw from
        u16 tile_data_address = 0;  // Fetch the startign addr of tile data to fetch
        u8 tile_first_byte = 0;     // Fetch the first byte corresponding to current line of the tile
        u8 tile_second_byte = 0;    // Fetch the second byte corresponding to current line of the tile
        u8 current_pixel = 0;
        int current_pixel_color = 0;
        u8 mask = 0;
        u8 palette = 0;
        // loop to render each pixel in current scanline
        for (int i = 0; i < 160; i++)
        {
            if ((CPU::memory[LCDC] & 0x08) == 0x08)
                bg_map_offset = 0x9C00;
            else
                bg_map_offset = 0x9800;

            // DEBUG
            // printf("Tile# at first bg map: %x\n", CPU::memory[bg_map_offset]);
            // printf("LCDC Value: %x\n", CPU::memory[LCDC]);
            // DEBUG

            tile_fine_offsetx = i % 8;
            tile_fine_offsety = scanline % 8;
            tile_coarse_offsetx = i / 8;
            tile_coarse_offsety = scanline / 8;
            tile_to_fetch = CPU::memory[bg_map_offset + tile_coarse_offsety * 32 + tile_coarse_offsetx]; // We now have to tile # to fetch from VRAM tile block
            // printf("Tile to fetch: %d\n", tile_to_fetch);
            if ((CPU::memory[LCDC] & 0x10) == 0x10)
                bg_tile_display_offset = 0x8000;
            else
                bg_tile_display_offset = 0x8800;

            if (tile_to_fetch < 128)
            {
                if (bg_tile_display_offset == 0x8000)
                {
                    tile_data_address = 0x8000 + (tile_to_fetch * 0x10);
                }
                else
                {
                    tile_data_address = 0x9000 + (tile_to_fetch * 0x10);
                }
            }
            else
            {
                tile_data_address = 0x8800 + ((tile_to_fetch - 128) * 0x10);
            }
            // printf("Tile data starting address: %x\n", tile_data_address);
            tile_first_byte = CPU::memory[tile_data_address + (tile_fine_offsety * 2)];
            tile_second_byte = CPU::memory[tile_data_address + (tile_fine_offsety * 2) + 1];
            mask = 0b00000001 << (7 - tile_fine_offsetx); // Create mask according to the X value of the pixel we are drawing
            tile_second_byte &= mask;
            tile_first_byte &= mask;
            tile_second_byte = tile_second_byte >> (7 - tile_fine_offsetx);
            tile_first_byte = tile_first_byte >> (7 - tile_fine_offsetx);
            current_pixel = (tile_second_byte << 1) | tile_first_byte;
            palette = CPU::memory[BGP];
            switch (current_pixel)
            {
            case 0:
                current_pixel_color = colors[palette & 0x03];
                break;
            case 1:
                current_pixel_color = colors[(palette & 0x0C) >> 2];
                break;
            case 2:
                current_pixel_color = colors[(palette & 0x30) >> 4];
                break;
            case 3:
                current_pixel_color = colors[(palette & 0xC0) >> 6];
                break;
            }
            screen_pixels[scanline][i] = current_pixel_color;
        }
    }

    void render_sprite()
    {
        u8 sprite_size = 1; // Sprite size, 1: 8x8 2: 8x16
        u8 sprite_y = 0;
        u8 sprite_x = 0;
        u8 sprite_index = 0;
        u8 sprite_attr = 0;
        if ((CPU::memory[LCDC] & 0x04) == 0x04)
            sprite_size = 2;
        else
            sprite_size = 1;
        // search through OAM 40x4 bytes
        for (int i = 0; i < 160; i += 4)
        {
            sprite_y = CPU::memory[0xFE00 + i] - 16;
            sprite_x = CPU::memory[0xFE00 + i + 1];
            sprite_index = CPU::memory[0xFE00 + i + 2];
            sprite_attr = CPU::memory[0xFE00 + i + 3];
        }
    }

    void render_scanline()
    {
        // approach: read tilemap
        // read tile data for current scanline
        // decode tile data
        // render to offscreen buffer
        // get tile offsets
        if ((CPU::memory[LCDC] & 0x08) == 0x08)
            bg_map_offset = 0x9C00;
        else
            bg_map_offset = 0x9800;
        if ((CPU::memory[LCDC] & 0x40) == 0x40)
            window_map_offset = 0x9C00;
        else
            window_map_offset = 0x9800;

        // u8 scroll_x = 0; // CPU::memory[SCX];
        // u8 scroll_y = 0; // CPU::memory[SCY];
        // // u8 window_scroll_x = 0;
        // // u8 window_scroll_y = 0;

        // // u8 window_tile_coarse_offsetx = window_scroll_x / 8;
        // // u8 window_tile_coarse_offsety = window_scroll_y / 8;

        // // u8 window_tile_fine_offsetx = normalize(window_scroll_x) % 8;
        // // u8 window_tile_fine_offsety = normalize(window_scroll_y) % 8;

        // u8 sprite_tile_fine_offsetx = 0;
        // u8 sprite_tile_fine_offsety = 0;

        u8 sprites_this_scanline = 0;
        // // search through OAM 40x4 bytes
        // for (int o = 0; o < 160; o += 4)
        // {
        //     // check if sprite Y falls in current scanline
        //     u8 sprite_y = CPU::memory[0xFE00 + o] - 16;
        //     u8 sprite_x = CPU::memory[0xFE00 + o + 1];
        //     u8 sprite_index = CPU::memory[0xFE00 + o + 2];
        //     u8 sprite_attr = CPU::memory[0xFE00 + o + 3];
        //     // 8x8 sprites
        //     if ((sprite_y <= scanline) && (sprite_y >= scanline - 8))
        //     {
        //         // printf("scanline: %i, sprite Y: %i, sprite X: %i\n", scanline, sprite_y, sprite_x);
        //         sprites_to_render.push_back(sprite{sprite_y, sprite_x, sprite_index, sprite_attr});
        //         sprites_this_scanline++;
        //         if (sprites_this_scanline >= 10)
        //         {
        //             sprites_this_scanline = 0;
        //             break;
        //         }
        //     }
        // }
        if (CPU::memory[LCDC] & 0x01)
            render_background();

        // render window
        // if ((CPU::memory[LCDC] & 0x20) == 0x20)
        // {
        //     if ((CPU::memory[LCDC] & 0x01) == 0x01)
        //     {
        //         tile_to_fetch = CPU::memory[window_map_offset + window_tile_coarse_offsety * 32 + window_tile_coarse_offsetx];
        //         // We now have the tile and the row to be rendered lets calculate the bytes we have to fetch 2 bytes for one row

        //         std::vector<std::vector<u8>> tile_data = window_grid[tile_to_fetch];
        //         u8 palette = CPU::memory[BGP];
        //         u32 pixel_color_num = tile_data[window_tile_fine_offsety][window_tile_fine_offsetx];

        //         switch (pixel_color_num)
        //         {
        //         case 0:
        //             pixel_color_num = colors[palette & 0x03];
        //             break;
        //         case 1:
        //             pixel_color_num = colors[(palette & 0x0C) >> 2];
        //             break;
        //         case 2:
        //             pixel_color_num = colors[(palette & 0x30) >> 4];
        //             break;
        //         case 3:
        //             pixel_color_num = colors[(palette & 0xC0) >> 6];
        //             break;
        //         }
        //         screen_pixels[scanline][i] = pixel_color_num;
        //     }
        // }
        // for (int i = 0; i < 160; i++)
        // {
        //     if ((CPU::memory[LCDC] & 0x02) == 0x02)
        //     {
        //         u32 sprite_pixel_color_num = 0;
        //         // if((CPU::memory[LCDC] & 0x04) == 0x04)
        //         // {
        //         //     printf("Sprite size: 8x16\n");
        //         // }
        //         // else printf("Sprite size: 8x8\n");
        //         // render sprites
        //         for (auto it : sprites_to_render)
        //         {
        //             if ((it.sprite_x - 8 >= i - 7) && (it.sprite_x - 8 + 8 <= i + 7))
        //             {
        //                 // sprite_tile_to_fetch = CPU::memory[0x8000 + it.sprite_index];
        //                 // std::vector<std::vector<u8>> sprite_tile_data = sprite_grid[sprite_tile_to_fetch];
        //                 std::vector<std::vector<u8>> sprite_tile_data = sprite_grid[it.sprite_index];
        //                 u8 sprite_palette = 0;
        //                 if ((it.sprite_attr & 0x10) == 0x10)
        //                 {
        //                     sprite_palette = CPU::memory[OBP1];
        //                 }
        //                 else
        //                 {
        //                     sprite_palette = CPU::memory[OBP0];
        //                 }
        //                 sprite_tile_fine_offsety = normalize(scanline) % 8;
        //                 if ((it.sprite_attr & 0x40) == 0x40)
        //                 {
        //                     // Y flip
        //                     sprite_tile_fine_offsety = std::abs(8 - sprite_tile_fine_offsety);
        //                 }
        //                 sprite_tile_fine_offsetx = normalize(i) % 8;
        //                 if ((it.sprite_attr & 0x20) == 0x20)
        //                 {
        //                     // X flip
        //                     sprite_tile_fine_offsetx = std::abs(8 - sprite_tile_fine_offsetx);
        //                 }
        //                 sprite_pixel_color_num = sprite_tile_data[sprite_tile_fine_offsety][sprite_tile_fine_offsetx];
        //                 switch (sprite_pixel_color_num)
        //                 {
        //                 case 1:
        //                     sprite_pixel_color_num = colors[(sprite_palette & 0x0C) >> 2];
        //                     break;
        //                 case 2:
        //                     sprite_pixel_color_num = colors[(sprite_palette & 0x30) >> 4];
        //                     break;
        //                 case 3:
        //                     sprite_pixel_color_num = colors[(sprite_palette & 0xC0) >> 6];
        //                     break;
        //                 }
        //                 if ((it.sprite_attr & 0x80) != 0x80)
        //                     screen_pixels[scanline][i] = sprite_pixel_color_num;
        //             }
        //         }
        //     }
        // }
        // sprites_to_render.clear();
        // sprites_this_scanline = 0;

        //}
    }

    void renderBGTiles()
    {
        // u16 bg_map_offset = 0x9800;
        u16 bg_tile_display_offset = 0x8000;
        std::vector<std::vector<std::vector<u8>>> grid;
        grid = build_chrgrid(bg_tile_display_offset);
        u32 pixel_color_num = 0;
        u8 palette = CPU::memory[BGP];
        std::vector<std::vector<u8>> tile_data((0));
        for (int i = 0; i < 256; i++)
        {
            for (int j = 0; j < 256; j++)
            {
                u16 tile = (i / 8) * 32 + (j / 8);
                if (tile < grid.size())
                    tile_data = grid[tile];
                pixel_color_num = tile_data[normalize(i) % 8][normalize(j) % 8];
                switch (pixel_color_num)
                {
                case 0:
                    pixel_color_num = colors[palette & 0x03];
                    break;
                case 1:
                    pixel_color_num = colors[(palette & 0x0C) >> 2];
                    break;
                case 2:
                    pixel_color_num = colors[(palette & 0x30) >> 4];
                    break;
                case 3:
                    pixel_color_num = colors[(palette & 0xC0) >> 6];
                    break;
                }
                GUI::bgTiles_screen_buffer[i * 256 + j] = pixel_color_num;
            }
        }
    }

    // void render_sprite_tiles()
    // {
    //     u16 sprite_tile_display_offset = 0x8000;
    //     std::vector<std::vector<std::vector<u8>>> grid;
    //     std::vector<sprite> oam_sprites_to_render;
    //     grid = build_chrgrid(sprite_tile_display_offset);
    //     for (u8 o = 0; o < 160; o += 4)
    //     {
    //         u8 sprite_y = CPU::memory[0xFE00 + o] + 16;
    //         u8 sprite_x = CPU::memory[0xFE00 + o + 1];
    //         u8 sprite_index = CPU::memory[0xFE00 + o + 2];
    //         u8 sprite_attr = CPU::memory[0xFE00 + o + 3];
    //         // 8x8 sprites
    //         oam_sprites_to_render.push_back(sprite{sprite_y, sprite_x, sprite_index, sprite_attr});
    //     }
    //     u8 oam_sprite_tile_to_fetch = 0;
    //     for (auto it : oam_sprites_to_render)
    //     {
    //         oam_sprite_tile_to_fetch = CPU::memory[0x8000 + it.sprite_index];
    //         std::vector<std::vector<u8>> sprite_tile_data = sprite_grid[oam_sprite_tile_to_fetch];
    //         u8 sprite_palette = 0;
    //         if ((it.sprite_attr & 0x10) == 0x10)
    //         {
    //             sprite_palette = CPU::memory[OBP1];
    //         }
    //         else
    //         {
    //             sprite_palette = CPU::memory[OBP0];
    //         }
    //         sprite_pixel_color_num = sprite_tile_data[sprite_tile_fine_offsety][sprite_tile_fine_offsetx];
    //         switch (sprite_pixel_color_num)
    //         {
    //         case 1:
    //             sprite_pixel_color_num = colors[(sprite_palette & 0x0C) >> 2];
    //             break;
    //         case 2:
    //             sprite_pixel_color_num = colors[(sprite_palette & 0x30) >> 4];
    //             break;
    //         case 3:
    //             sprite_pixel_color_num = colors[(sprite_palette & 0xC0) >> 6];
    //             break;
    //         }
    //         screen_pixels[scanline][i] = sprite_pixel_color_num;
    //     }
    // }

    std::vector<std::vector<std::vector<u8>>> build_chrgrid(u16 offset)
    {
        std::vector<std::vector<std::vector<u8>>> chrGrid;
        if (offset == 0x8000)
        {
            build_chrgrid_half(0x8000, chrGrid);
            build_chrgrid_half(0x8800, chrGrid);
        }
        else if (offset == 0x8800)
        {
            build_chrgrid_half(0x9000, chrGrid);
            build_chrgrid_half(0x8800, chrGrid);
        }
        return chrGrid;
    }

    void build_chrgrid_half(u16 offset, std::vector<std::vector<std::vector<u8>>> &chrGrid)
    {
        u8 mask, lowVal, highVal, compoundVal;
        for (int i = offset; i < offset + 2048; i += 16)
        {
            std::vector<u8> tileLow;
            std::vector<u8> tileHigh;

            for (int j = 0; j < 16; j += 2)
            {
                tileLow.push_back(CPU::memory[i + j]);
            }
            for (int k = 1; k < 16; k += 2)
            {
                tileHigh.push_back(CPU::memory[i + k]);
            }
            std::vector<std::vector<u8>> tile;
            for (int l = 0; l < 8; l++)
            {
                std::vector<u8> compoundTileRow;
                for (int k = 0; k < 8; k++)
                {
                    mask = 0b00000001 << (7 - k);
                    lowVal = mask & tileLow[l];
                    highVal = mask & tileHigh[l];
                    lowVal = lowVal >> (7 - k);
                    highVal = highVal >> (7 - k);
                    compoundVal = (highVal << 1) | lowVal;
                    compoundTileRow.push_back(compoundVal);
                }
                tile.push_back(compoundTileRow);
            }
            chrGrid.push_back(tile);
        }
        // return chrGrid;
    }

    void reset()
    {
        for (int i = 0; i < 144; i++)
        {
            for (int j = 0; j < 160; j++)
            {
                screen_pixels[i][j] = 0;
                GUI::off_screen_buffer[i * 160 + j] = 0;
            }
        }
    }

    void render_offscreen_buffer()
    {
        for (int i = 0; i < 144; i++)
        {
            for (int j = 0; j < 160; j++)
            {
                GUI::off_screen_buffer[i * 160 + j] = screen_pixels[i][j];                
            }
        }
    }

    void build_tile_grids()
    {
        if ((CPU::memory[LCDC] & 0x10) == 0x10)
            bg_tile_display_offset = 0x8000;
        else
            bg_tile_display_offset = 0x8800;
        grid = build_chrgrid(bg_tile_display_offset);
        window_grid = build_chrgrid(bg_tile_display_offset);
        sprite_grid = build_chrgrid(0x8000);
    }
} // namespace PPU