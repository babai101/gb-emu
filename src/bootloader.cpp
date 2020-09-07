#include "bootloader.hpp"
#include <fstream>
#include <iostream>
#include <string>
namespace bootloader {
char *cart;
std::size_t rom_size;
bool load(char *rom_name) {
    char *raw_cart;
    raw_cart = load_cart(rom_name);
    if (rom_size > 0) {
        parse(raw_cart);
        for (std::size_t i = 0; i < rom_size - 1; i++) {
            CPU::memory[i] = raw_cart[i];
        }
        return true;
    } else {
        return false;
    }
}
char *load_cart(char *rom_name) {
    std::streampos rom_file_end;
    std::cout << "Loading rom " << rom_name << std::endl;
    std::ifstream rom(rom_name, std::ifstream::ate | std::ifstream::binary);
    if ((rom_file_end = rom.tellg()) > 32768) {
        std::cout << "Rom size > 32k .. Mappers not implemented yet"
                  << std::endl;
        cart = new char[0];
        rom_size = 0;
        return cart;
    }
    rom_size = rom_file_end;
    std::cout << "Rom size: " << rom_size << " bytes" << std::endl;
    cart = new char[rom_size];
    rom.seekg(0, std::ifstream::beg);
    rom.read(cart, rom_size);
    rom.close();
    return cart;
}

void parse(char *cart) {
    std::cout << "Parsing Header...." << std::endl;
    std::string title = "";
    for (size_t i = 0x134; i <= 0x142; i++) {
        title.append(std::string(1, cart[i]));
    }
    std::cout << "Rom Title: " << title << std::endl;
    std::string gb_color;
    if (static_cast<int>(cart[0x143]) == 0x80) {
        gb_color = "GBC";
        CPU::gb_type = "CGB";
    } else {
        gb_color = "GB";
    }
    std::cout << "GameBoy Color type: " << gb_color << std::endl;
    std::string gb_type;
    if (cart[0x146] == 0) {
        gb_type = "GB";
    } else if (cart[0x146] == 3) {
        gb_type = "SGB";
    }
    std::cout << "GameBoy type: " << gb_type << std::endl;
}
} // namespace bootloader