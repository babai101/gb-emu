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
#define JOYP 0xFF00

namespace CPU
{
    extern u8 memory[];
    extern std::string gb_type;
    extern int T_CYCLES_PER_FRAME;
    extern bool isr_served;
    extern bool cpu_halted;
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
    void check_interrupts();
    void run_timers(int);
} // namespace CPU