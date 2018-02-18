#include "include/cpu.hpp"
#include <array>
#include <cstdint>
#include <iostream>

namespace CPU {
u8 memory[65536];
u8 A, B, C, D, E, F, H, L, Flag, opcode;
std::string gb_type = "DMG";
u16 PC, SP;
std::array<u8, 8> rst_addrs = {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38};
int cycles = 0;
void reset() {
    for (std::size_t i = 0; i < 65536; i++)
        memory[i] = 0;
    seed();
}
void seed() {
    PC = 0x100;
    A = 0x00;
    if (gb_type == "DMG") {
        F = 0x01;
    } else if (gb_type == "CGB") {
        F = 0x11;
    } else if (gb_type == "GBP") {
        F = 0xFF;
    }
    Flag = 0xB0;
    B = 0x00;
    C = 0x13;
    D = 0x00;
    E = 0xD8;
    H = 0x01;
    L = 0x4D;
    SP = 0xFFFE;

    memory[0xFF05] = 0x00; // TIMA
    memory[0xFF06] = 0x00; // TMA
    memory[0xFF07] = 0x00; // TAC
    memory[0xFF10] = 0x80; // NR10
    memory[0xFF11] = 0xBF; // NR11
    memory[0xFF12] = 0xF3; // NR12
    memory[0xFF14] = 0xBF; // NR14
    memory[0xFF16] = 0x3F; // NR21
    memory[0xFF17] = 0x00; // NR22
    memory[0xFF19] = 0xBF; // NR24
    memory[0xFF1A] = 0x7F; // NR30
    memory[0xFF1B] = 0xFF; // NR31
    memory[0xFF1C] = 0x9F; // NR32
    memory[0xFF1E] = 0xBF; // NR33
    memory[0xFF20] = 0xFF; // NR41
    memory[0xFF21] = 0x00; // NR42
    memory[0xFF22] = 0x00; // NR43
    memory[0xFF23] = 0xBF; // NR30
    memory[0xFF24] = 0x77; // NR50
    memory[0xFF25] = 0xF3; // NR51
    if (gb_type == "DMG") {
        memory[0xFF26] = 0xF1; // NR52
    } else if (gb_type == "CGB") {
        memory[0xFF26] = 0xF0; // NR52
    }
    memory[0xFF40] = 0x91; // LCDC
    memory[0xFF42] = 0x00; // SCY
    memory[0xFF43] = 0x00; // SCX
    memory[0xFF45] = 0x00; // LYC
    memory[0xFF47] = 0xFC; // BGP
    memory[0xFF48] = 0xFF; // OBP0
    memory[0xFF49] = 0xFF; // OBP1
    memory[0xFF4A] = 0x00; // WY
    memory[0xFF4B] = 0x00; // WX
    memory[0xFFFF] = 0x00; // IE
}
u8 read_memory(u16 addr) { return 0; }
void write_memory(u16 addr, u8 val) {}
u16 to_u16(u8 lsb, u8 msb) {
    u16 temp = (msb << 8) | lsb;
    return temp;
}
u8 msb(u16 val) { return val >> 8; }
u8 lsb(u16 val) { return val & 0xFF; }
void jp_nn() {
    u8 lsb = read_memory(PC++);
    u8 msb = read_memory(PC++);
    PC = to_u16(lsb, msb);
    cycles += 4;
}
void jp_hl() {
    PC = to_u16(L, H);
    cycles += 1;
}
void jp_cc_nn() {
    u8 lsb = read_memory(PC++);
    u8 msb = read_memory(PC++);
    bool result = false;
    switch (opcode) {
    case 0xC2:
        result = !check_flag(zero);
        break;
    case 0xCA:
        result = check_flag(zero);
        break;
    case 0xD2:
        result = !check_flag(carry);
        break;
    case 0xDA:
        result = check_flag(carry);
        break;
    }
    cycles += 3;
    if (result) {
        PC = to_u16(lsb, msb);
        cycles++;
    }
}
void jr_r() {
    s8 r = read_memory(PC++);
    PC += r;
    cycles += 3;
}
void jr_cc_r() {
    s8 r = read_memory(PC++);
    bool result = false;
    ;
    switch (opcode) {
    case 0x20:
        result = !check_flag(zero);
        break;
    case 0x28:
        result = check_flag(zero);
        break;
    case 0x30:
        result = !check_flag(carry);
        break;
    case 0x38:
        result = check_flag(carry);
        break;
    }
    cycles += 2;
    if (result) {
        PC += r;
        cycles++;
    }
}
void call_nn() {
    u8 addr_lsb = read_memory(PC++);
    u8 addr_msb = read_memory(PC++);
    u16 nn = to_u16(addr_lsb, addr_msb);
    write_memory(--SP, msb(PC));
    write_memory(--SP, lsb(PC));
    PC = nn;
    cycles += 6;
}
void call_cc_nn() {
    u8 addr_lsb = read_memory(PC++);
    u8 addr_msb = read_memory(PC++);
    u16 nn = to_u16(addr_lsb, addr_msb);
    bool result = false;
    switch (opcode) {
    case 0xC4:
        result = !check_flag(zero);
        break;
    case 0xCC:
        result = check_flag(zero);
        break;
    case 0xD4:
        result = !check_flag(carry);
        break;
    case 0xDC:
        result = check_flag(carry);
        break;
    }
    cycles += 3;
    if (result) {
        write_memory(--SP, msb(PC));
        write_memory(--SP, lsb(PC));
        PC = nn;
        cycles += 3;
    }
}
void ret() {
    u8 lsb = read_memory(SP++);
    u8 msb = read_memory(SP++);
    PC = to_u16(lsb, msb);
    cycles += 4;
}
void ret_cc() {
    bool result = false;
    switch (opcode) {
    case 0xC0:
        result = !check_flag(zero);
        break;
    case 0xC8:
        result = check_flag(zero);
        break;
    case 0xD0:
        result = !check_flag(carry);
        break;
    case 0xD8:
        result = check_flag(carry);
        break;
    }
    cycles += 2;
    if (result) {
        u8 ret_lsb = read_memory(SP++);
        u8 ret_msb = read_memory(SP++);
        PC = to_u16(ret_lsb, ret_msb);
        cycles += 3;
    }
}
void reti() {
    u8 ret_lsb = read_memory(SP++);
    u8 ret_msb = read_memory(SP++);
    PC = to_u16(ret_lsb, ret_msb);
    cycles += 4;
    // TODO IME = 1;
}
void rst_n() {
    u8 rst_addr = (opcode >> 3) & 0x07;
    rst_addr = rst_addrs[rst_addr];
    write_memory(--SP, msb(PC));
    write_memory(--SP, lsb(PC));
    PC = to_u16(rst_addr, 0x00);
    cycles += 4;
}
bool check_flag(enum flags f) {
    switch (f) {
    case zero:
        break;
    case carry:
        break;
    }
}
void fetch_opcode() { opcode = read_memory(PC++); }
void decode_opcode() {
    switch (opcode) {
    // JUMPs
    case 0xC3:
        jp_nn();
        break;
    case 0xE9:
        jp_hl();
        break;
    case 0xC2:
    case 0xD2:
    case 0xCA:
    case 0xDA:
        jp_cc_nn();
        break;
    case 0x18:
        jr_r();
        break;
    case 0x20:
    case 0x30:
    case 0x28:
    case 0x38:
        jr_cc_r();
        break;
    // CALLs
    case 0xCD:
        call_nn();
        break;
    case 0xC4:
    case 0xD4:
    case 0xCC:
    case 0xDC:
        call_cc_nn();
        break;
    // RETURNs
    case 0xC9:
        ret();
        break;
    case 0xC0:
    case 0xD0:
    case 0xC8:
    case 0xD8:
        ret_cc();
        break;
    case 0xD9:
        reti();
        break;
    // RESTARTs
    case 0xC7:
    case 0xD7:
    case 0xE7:
    case 0xF7:
    case 0xCF:
    case 0xDF:
    case 0xEF:
    case 0xFF:
        rst_n();
        break;
    default:
        break;
    }
}
int run() {
    while (true) {
        fetch_opcode();
        decode_opcode();
    }
    return 0;
}
} // namespace CPU