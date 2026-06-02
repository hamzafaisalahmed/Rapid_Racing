#include "Game.h"
#include <algorithm>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <stdexcept>

void Game::init()
{
    std::srand((unsigned)std::time(0));

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window.create(desktop, "Racing", sf::Style::Default);
    window.setFramerateLimit(60);
    gameView = sf::View(sf::FloatRect(0.f, 0.f, 480.f, 720.f));
    gameView.setViewport(sf::FloatRect(
        (float)desktop.width,
        0.f,
        (float)desktop.width,
        1.f));
    window.setView(gameView);

    stateStack.push(GameState::Home);
}

void Game::run()
{
    sf::Clock clock;
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f)
            dt = 0.05f;
        handleEvents();
        GameState current = stateStack.top();
        if (current == GameState::Playing)
        {
            update(dt);
        }
        render();
        window.display();
    }
}

void Game::handleEvents()
{
    return;
}

void Game::update(float dt)
{
    return;
}

void Game::render()
{
    return;
}