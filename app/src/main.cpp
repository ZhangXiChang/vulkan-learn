#include "App.h"

#include <iostream>

int main(int argc, char *argv[])
{
    try
    {
        App().run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
