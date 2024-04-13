#pragma once
#include "common.hpp"
#include <string>
#define IE 0xFFFF
#define IF 0xFF0F
#define VSYNCVEC 0x0040
#define LCDSTAT 0x0048
#define TIMER 0x0050
#define DIV 0xFF04
#define TIMA 0xFF05
#define TMA 0xFF06
#define TAC 0xFF07
#define DMA 0xFF46

namespace CPU
{
    extern u8 memory[];
    extern std::string gb_type;
    u8 read_memory(u16);
    enum flags
    {
        zero,
        subtract,
        half_carry,
        carry
    };
    void write_memory(u16, u8);
    int run();
    void fetch_opcode();
    void decode_opcode();
    void reset();
    void seed();
    bool check_flag(enum flags);
    void serve_isr(u16);
} // namespace CPU