#include "gui.hpp"

int main(int argc, char **argv)
{
    if (GUI::init(argv[1]))    
    {
        GUI::run();
    }
    else
        std::cout << "Error occured" << std::endl;
    return 0;
}