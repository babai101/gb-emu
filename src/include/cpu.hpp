#pragma once
#include "common.hpp"
#include <string>
namespace CPU {
extern u8 memory[];
extern std::string gb_type;
u8 read_memory(u16);
enum flags { zero, substract, half_carry, carry };
void write_memory(u16, u8);
int run();
void fetch_opcode();
void decode_opcode();
void reset();
void seed();
bool check_flag(enum flags);
} // namespace CPU