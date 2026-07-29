#include "Game.h"

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
        for (int i = 0; i < 10000; i++)
            std::cout << " ERROR";
    }
}