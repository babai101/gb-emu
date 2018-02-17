#include "include/cpu.hpp"
#include <iostream>
#include <cstdint>

namespace CPU {
    u8 memory[65536];
    u8 A, B, C, D, E, F, H, L;
    std::string gb_type = "DMG";
    u16 PC;
    u16 SP;
    void reset() 
    {
        for(std::size_t i = 0; i < 65536; i++) 
            memory[i] = 0;
        seed();
    }
    void seed()
    {
        PC = 0x100;
        if(gb_type == "DMG")
        {
            A = 0x01;
        }
        F = 0xB0;
        B = 0x00;
        C = 0x13;
        D = 0x00;
        E = 0xD8;
        H = 0x01;
        L = 0x4D;
        SP = 0xFFFE;
    }
    char read_memory(u8 addr)
    {
        return 0;
    }
    void write_memory(u16 addr)
    {

    }
    void fetch_opcode() 
    {
        int opcode = read_memory(PC++);
    }
    void decode_opcode()
    {

    }
    int run()
    {
        while(true)
        {
            fetch_opcode();
            decode_opcode();
        }
        return 0;
    }
}