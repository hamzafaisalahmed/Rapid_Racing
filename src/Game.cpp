#include "Game.h"
#include "Utils.h"
#include "Graphics.h"
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

    track.LoadTrack("assets/textures/track1.png");
    player.load("assets/textures/car1.png");

    if (!homeTexture.loadFromFile("assets/textures/homescreen.png"))
        throw std::runtime_error("Home background texture not found");
    homeBackground.setTexture(homeTexture);
    // Position player at center of track
    player.setPosition(sf::Vector2f(1620.f, 2800.f));

    // Initialize player details
    player.setAngle(90.f);
    player.setCurrSpeed(0.f);
    player.setMaxSpeed(300.f);
    player.setAcc(50.f);
    player.setMaxReverseSpeed(-100.f);

    if (!font.loadFromFile("assets/fonts/ProFontWindows.ttf"))
        throw std::runtime_error("Font not found");

    timerText.setFont(font);
    timerText.setCharacterSize(16);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition(10.f, 10.f);

    lapText.setFont(font);
    lapText.setCharacterSize(20);
    lapText.setFillColor(sf::Color::Red);
    lapText.setPosition(10.f, 30.f);

    speedometer.setFont(font);
    speedometer.setCharacterSize(35);
    speedometer.setFillColor(sf::Color::White);
    speedometer.setPosition(10.f, 700.f);

    hudView = sf::View(sf::FloatRect(0.f, 0.f, 1200.f, 800.f));

    currentLap = 0;
    totalLaps = 3;
    currentLapTime = 0.f;
    raceTimer.restart();

    // Set up game view to show a reasonable portion of the track
    // View should be smaller than track to allow zooming on player
    gameView = sf::View(sf::FloatRect(0.f, 0.f, 300.f, 200.f));
    gameView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
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
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window.close();
        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (stateStack.top() == GameState::Home)
            {
                return;
            }
        }
    }
}

void Game::handlePlayerMovement(float dt)
{
    if (window.hasFocus())
    {
        sf::Vector2f oldPosition = player.getPosition();
        float angle = player.getAngle();
        float oldAngle = angle;
        float turnFactor = 0.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            turnFactor = -1.f * standardTurnFactor;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            turnFactor = standardTurnFactor;
        }
        angle += turnFactor * player.getTurnSpeed() * dt;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            player.accelerate(dt);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            player.decelerate(dt);
        }
        else
        {
            if (player.getCurrSpeed() < 60.f && player.getCurrSpeed() > -60.f)
                player.setCurrSpeed(player.getCurrSpeed() * track.getFriction() * 0.98f);
            else
                player.setCurrSpeed(player.getCurrSpeed() * track.getFriction());
        }

        float speed = player.getCurrSpeed();
        sf::Vector2f position = player.getPosition();
        position.x += std::cos((angle - 90.f) * (3.14159f / 180.f)) * speed * dt;
        position.y += std::sin((angle - 90.f) * (3.14159f / 180.f)) * speed * dt;
        if (checkCollisions(position, angle))
        {
            player.setPosition(oldPosition);
            player.setCurrSpeed(speed * -0.3f);
            player.setAngle(oldAngle);
        }
        else
        {
            player.setPosition(position);
            player.setCurrSpeed(speed);
            player.setAngle(lerp(oldAngle, angle, 0.6f));
        }
    }
}

void Game::update(float dt)
{
    handlePlayerMovement(dt);
    currentLapTime += dt;
    if (track.isFinishLine(player.getCorners(player.getPosition(), player.getAngle()), dt))
    {
        currentLap++;
        if (currentLap > totalLaps)
        {
            stateStack.push(GameState::LevelComplete);
        }
    }
}

void Game::render()
{
    window.clear(sf::Color::Black);
    if (stateStack.top() == GameState::Home)
    {
        renderHomeScreen();
        return;
    }
    if (stateStack.top() == GameState::Playing)
    {
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
        renderHUD();
    }
}

bool Game::checkCollisions(sf::Vector2f pos, float angle)
{
    for (auto &corner : player.getCorners(pos, angle))
        if (!track.isOnRoad(corner))
        {
            return true;
        }

    return false;
}