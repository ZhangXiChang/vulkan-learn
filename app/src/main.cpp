#include "App.h"

#include <iostream>

int main(int argc, char *argv[])
{
    App *app;
    try
    {
        app = new App;
        app->loop();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    delete app;
    return EXIT_SUCCESS;
}
