#include "gui.hpp"
#include <iostream>

namespace GUI {
void init(char *rom_name) {
    CPU::reset();
    if (!bootloader::load(rom_name)) {
        std::cout << "Failed to load ROM...Quitting" << std::endl;
    }
}
void run() { CPU::run(); }
} // namespace GUI
