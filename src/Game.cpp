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

    window.create(sf::VideoMode(1200, 800), "Racing", sf::Style::Default);
    window.setFramerateLimit(60);

    // Load track first
    track.LoadTrack("assets/textures/track1.png");
    sf::Vector2u trackSize = track.getSize();

    // Load player
    player.load("assets/textures/car1.png");

    // Position player at center of track
    sf::Vector2f playerStartPos(trackSize.x / 2.f, trackSize.y / 2.f);
    player.setPosition(playerStartPos);

    // Initialize player details
    player.setAngle(0.f);
    player.setCurrSpeed(0.f);
    player.setMaxSpeed(200.f);
    player.setAcc(100.f);

    // Set up game view to show a reasonable portion of the track
    // View should be smaller than track to allow zooming on player
    gameView = sf::View(sf::FloatRect(0.f, 0.f, 480.f, 720.f));
    gameView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    window.setView(gameView);

    stateStack.push(GameState::Playing);
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
    if (window.hasFocus())
    {
        float angle = player.getAngle();
        float turnFactor = 0.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            turnFactor = -1.f;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            turnFactor = 1.f;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            player.accelerate(dt);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            player.decelerate(dt);
        }
        float speed = player.getCurrSpeed();
        angle += turnFactor * (speed / player.getMaxSpeed()) * dt;
        player.setAngle(angle);

        speed *= track.getFriction();
        player.setCurrSpeed(speed);
        sf::Vector2f position = player.getPosition();
        position.x += std::sin(angle) * player.getCurrSpeed() * dt;
        position.y += std::cos(angle) * player.getCurrSpeed() * dt;
        player.setPosition(position);
    }
}

void Game::render()
{
    window.clear(sf::Color::Black);

    // Get track dimensions
    sf::Vector2u trackSize = track.getSize();

    // Get player position
    sf::Vector2f playerPos = player.getPosition();

    // Center view on player, constrained within track bounds
    sf::Vector2f viewCenter = playerPos;

    // Constrain view to not go out of track bounds
    float viewWidth = gameView.getSize().x;
    float viewHeight = gameView.getSize().y;

    // Keep view centered on player, but within track bounds
    if (viewCenter.x - viewWidth / 2.f < 0.f)
        viewCenter.x = viewWidth / 2.f;
    if (viewCenter.x + viewWidth / 2.f > trackSize.x)
        viewCenter.x = trackSize.x - viewWidth / 2.f;
    if (viewCenter.y - viewHeight / 2.f < 0.f)
        viewCenter.y = viewHeight / 2.f;
    if (viewCenter.y + viewHeight / 2.f > trackSize.y)
        viewCenter.y = trackSize.y - viewHeight / 2.f;

    gameView.setCenter(viewCenter);
    window.setView(gameView);

    track.draw(window, sf::VideoMode::getDesktopMode());
    player.draw(window);
}