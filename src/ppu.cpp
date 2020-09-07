#include "include/ppu.hpp"
#include "gui.hpp"
#include "cpu.hpp"

namespace PPU
{
    u16 cycle = 0;
    u8 mode = 1;
    u8 scanline = 0;
    u16 bg_map_offset = 0;
    u16 bg_tile_display_offset = 0;
    u32 screen_pixels[144][160];
    u32 colors[4] = {0x009bbc0f, 0x008bac0f, 0x00306230, 0x000f380f}; //4 shades of green
    std::vector<std::vector<std::vector<u8>>> grid;

    void tick()
    {
        if (scanline == 0 && cycle == 0)
        {
            //get tile offsets
            if ((CPU::read_memory(LCDC) & 0x08) == 0x08)
                bg_map_offset = 0x9C00;
            else
                bg_map_offset = 0x9800;
            if ((CPU::read_memory(LCDC) & 0x10) == 0x10)
                bg_tile_display_offset = 0x8000;
            else
                bg_tile_display_offset = 0x8800;
            grid = build_chrgrid(bg_tile_display_offset);
        }
        if (mode != 1)
        {
            if (cycle == 80)
            {
                //scanning of OAM done, kickoff mode 3
                u8 temp = CPU::read_memory(STAT) & 0xFC;
                temp |= 0x03;
                CPU::write_memory(STAT, temp);
                mode = 3;
                cycle++;
            }
            else if (cycle == (80 + 172))
            {
                //rendering pixels done kickoff mode 0 (hblank)
                CPU::write_memory(STAT, (CPU::read_memory(STAT) & 0xFC));
                mode = 0;
                cycle++;
            }
            else if (cycle == 455)
            {
                //scanline finished
                render_scanline();
                scanline++;
                CPU::write_memory(LY, scanline);
                if (scanline == 144)
                {
                    //vblank starts
                    //request interrupt
                    u8 intrpt = CPU::read_memory(IF);
                    intrpt &= 0xFE;
                    intrpt |= 0x01;
                    CPU::write_memory(IF, intrpt);
                    u8 temp = CPU::read_memory(STAT) & 0xFC;
                    temp |= 0x01;
                    CPU::write_memory(STAT, temp);
                    mode = 1;

                    render_offscreen_buffer();
                }
                else
                {
                    //scanning of OAM starts kickoff mode 2
                    u8 temp = CPU::read_memory(STAT) & 0xFC;
                    temp |= 0x02;
                    CPU::write_memory(STAT, temp);
                    mode = 2;
                }
                cycle = 0;
            }
            else
                cycle++;
        }
        else
        {
            if (cycle == 455)
            {
                scanline++;
                CPU::write_memory(LY, scanline);
                if (scanline == 154)
                {
                    //vblank ends, kick off mode 2
                    u8 temp = CPU::read_memory(STAT) & 0xFC;
                    temp |= 0x02;
                    CPU::write_memory(STAT, temp);
                    mode = 2;
                    scanline = 0;
                    CPU::write_memory(LY, scanline);
                }
                cycle = 0;
            }
            else
                cycle++;
        }
    }

    void render_scanline()
    {
        //approach: read tilemap
        //read tile data for current scanline
        //decode tile data
        //render to offscreen buffer
        if ((CPU::read_memory(LCDC) & 0x80) == 0x80)
        {
            u8 scroll_x = CPU::memory[SCX];
            u8 scroll_y = CPU::memory[SCY];

            //Now we have the exact tile in tile map that we want to render from

            //loop for each pixel of scanline
            u8 tile_to_fetch = 0;

            u8 tile_coarse_offsetx = scroll_x / 8; //compute which tile corresponds to the scrolling values
            u8 tile_coarse_offsety = scroll_y / 8;

            u8 tile_fine_offsetx = scroll_x % 8; //starting pixel X offset of tile to draw from
            u8 tile_fine_offsety = scroll_y % 8; //starting row offset

            tile_coarse_offsety += scanline / 8; //Adjust for scanlines rendered
            tile_fine_offsety = scanline % 8;

            for (int i = 0; i < 160; i++)
            {
                if (i > 0)
                    tile_fine_offsetx++;
                if (tile_fine_offsetx == 8)
                {
                    tile_fine_offsetx = 0;
                    tile_coarse_offsetx++;
                }
                if (tile_coarse_offsetx > 31)
                {
                    tile_coarse_offsetx -= 32;
                }
                if (tile_coarse_offsety > 31)
                {
                    tile_coarse_offsety -= 32;
                }
                tile_to_fetch = CPU::read_memory(bg_map_offset + tile_coarse_offsety * 32 + tile_coarse_offsetx);
                //We now have the tile and the row to be rendered lets calculate the bytes we have to fetch 2 bytes for one row
                
                std::vector<std::vector<u8>> tile_data = grid[tile_to_fetch];
                u8 palette = CPU::read_memory(BGP);
                u32 pixel_color_num = tile_data[tile_fine_offsety][tile_fine_offsetx];
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
                screen_pixels[scanline][i] = pixel_color_num;
            }            
        }
    }

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
        // std::vector<std::vector<std::vector<u8>>> chrGrid;
        // u8 mask, lowVal, highVal, compoundVal;
        // for (int i = offset; i < offset + 4096; i += 16)
        // {
        //     std::vector<u8> tileLow;
        //     std::vector<u8> tileHigh;
            
        //     for (int j = 0; j < 16; j+=2)
        //     {
        //         tileLow.push_back(CPU::memory[i + j]);
        //     }
        //     for (int k = 1; k < 16; k+=2)
        //     {
        //         tileHigh.push_back(CPU::memory[i + k]);
        //     }
        //     std::vector<std::vector<u8>> tile;
        //     for (int l = 0; l < 8; l++)
        //     {
        //         std::vector<u8> compoundTileRow;
        //         for (int k = 0; k < 8; k++)
        //         {
        //             mask = 0b00000001 << (7 - k);
        //             lowVal = mask & tileLow[l];
        //             highVal = mask & tileHigh[l];
        //             lowVal = lowVal >> (7 - k);
        //             highVal = highVal >> (7 - k);
        //             compoundVal = (highVal << 1) | lowVal;
        //             compoundTileRow.push_back(compoundVal);
        //         }
        //         tile.push_back(compoundTileRow);
        //     }
        //     chrGrid.push_back(tile);
        // }
        return chrGrid;
    }

    void build_chrgrid_half(u16 offset, std::vector<std::vector<std::vector<u8>>>& chrGrid)
    {       
        u8 mask, lowVal, highVal, compoundVal;
        for (int i = offset; i < offset + 2048; i += 16)
        {
            std::vector<u8> tileLow;
            std::vector<u8> tileHigh;
            
            for (int j = 0; j < 16; j+=2)
            {
                tileLow.push_back(CPU::memory[i + j]);
            }
            for (int k = 1; k < 16; k+=2)
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
} // namespace PPU