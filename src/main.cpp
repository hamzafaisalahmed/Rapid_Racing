#include "Game.h"
#include <iostream>
int main()
{
    try
    {
        Game game;
        game.init();
        game.run();
    }
    catch (...)
    {
        std::cout << "Fatal error" << std::endl;
    }
}