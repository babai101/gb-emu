#pragma once
#include "common.hpp"
namespace CPU { 
    char read_memory(u8);
    int run();
    void fetch_opcode();
}