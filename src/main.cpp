#include "Game.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int main()
{
    try
    {
        Game game;
        game.init();
        game.run();
    }
    catch (const std::exception &e)
    {
        MessageBoxA(nullptr, e.what(), "Rapid Racing - Fatal Error", MB_OK | MB_ICONERROR);
    }
    catch (...)
    {
        MessageBoxA(nullptr, "An unknown error occurred.", "Rapid Racing - Fatal Error", MB_OK | MB_ICONERROR);
    }
}