#include <iostream>
#include "include/cpu.hpp"
#include "include/bootloader.hpp"

int main(int argc, char** argv) 
{
    CPU::reset();
    if(!bootloader::load(argv[1])) 
    {
        std::cout << "Failed to load ROM...Quitting" << std::endl;
    }
    return 0;
}