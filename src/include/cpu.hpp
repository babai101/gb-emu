#pragma once
#include "common.hpp"
#include <string>
namespace CPU {
extern u8 memory[];
extern std::string gb_type;
char read_memory(u8);
void write_memory(u16);
int run();
void fetch_opcode();
void decode_opcode();
void reset();
void seed();
} // namespace CPU