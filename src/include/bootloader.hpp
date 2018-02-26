#pragma once
#include "common.hpp"
#include "cpu.hpp"
namespace bootloader {
bool load(char *);
char *load_cart(char *);
void parse(char *);
} // namespace bootloader