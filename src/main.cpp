#include "gui.hpp"

int main(int argc, char **argv)
{
    if (GUI::init(argv[1]))
    // if (GUI::init("/home/soumyadeep/Projects/gb-emu/build/02-interrupts.gb"))
    {
        GUI::run();
    }
    else
        std::cout << "Error occured" << std::endl;
    return 0;
}