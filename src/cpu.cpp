#include "include/cpu.hpp"
#include <cstdint>
#include <iostream>

namespace CPU {
u8 memory[65536];
u8 A, B, C, D, E, F, H, L, P;
std::string gb_type = "DMG";
u16 PC;
u16 SP;
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
  P = 0xB0;
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
char read_memory(u8 addr) { return 0; }
void write_memory(u16 addr) {}
void fetch_opcode() { int opcode = read_memory(PC++); }
void decode_opcode() {}
int run() {
  while (true) {
    fetch_opcode();
    decode_opcode();
  }
  return 0;
}
} // namespace CPU