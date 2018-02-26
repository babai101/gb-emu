#include "gui.hpp"
#include <iostream>

int main(int argc, char **argv) {
    GUI::init(argv[1]);
    GUI::run();
    return 0;
}