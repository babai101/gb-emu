#include "include/bootloader.hpp"
#include "include/cpu.hpp"
#include <iostream>

int main(int argc, char **argv) {
    CPU::reset();
    if (!bootloader::load(argv[1])) {
        std::cout << "Failed to load ROM...Quitting" << std::endl;
    }
    CPU::run();
    return 0;
}