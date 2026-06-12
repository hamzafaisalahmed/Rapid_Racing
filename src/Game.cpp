#include "Game.h"
#include "Utils.h"
#include "Graphics.h"
#include <algorithm>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <stdexcept>
#include <fstream>
#include <functional>

void Game::resetLevel()
{
    player.setPosition(sf::Vector2f(1620.f, 2550.f));
    player.setAngle(90.f);
    player.setCurrSpeed(0.f);
    // player.setMaxSpeed(300.f);
    // player.setAcc(50.f);
    player.setMaxSpeed(500.f);
    player.setAcc(200.f);

    player.setMaxReverseSpeed(-100.f);

    currentLap = 1;
    totalLaps = 2;
    currentLapTime = 0.f;
    track.resetCooldown();
    endscreen.stop();
    engineAudio.setVolume(0.f);
    engineAudio.play();
    totalRaceTime = 0.f;
}

void Game::init()
{
    std::srand((unsigned)std::time(0));

    window.create(sf::VideoMode(1200, 800), "Rapid Racing", sf::Style::Default);
    window.setFramerateLimit(60);

    track.LoadTrack("assets/textures/track1.png");
    player.load("assets/textures/car1.png");

    waypoints = track.getWaypoints();
    if (!engineAudio.openFromFile("assets/audio/engine.ogg"))
        throw std::runtime_error("Engine sound not found\n");
    engineAudio.setLoop(true);

    if (!endscreen.openFromFile("assets/audio/endscreen.ogg"))
        throw std::runtime_error("Endscreen music file not found\n");
    endscreen.setLoop(true);
    endscreen.setVolume(70.f);

    resetLevel();

    hudView = sf::View(sf::FloatRect(0.f, 0.f, 1200.f, 800.f));

    // Set up game view to show a reasonable portion of the track
    // View should be smaller than track to allow zooming on player
    gameView = sf::View(sf::FloatRect(0.f, 0.f, 300.f, 200.f));
    gameView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    window.setView(gameView);
    graphics = std::make_unique<Graphics>(window, gameView, hudView, track, player);
    graphics->init();
    player.setCollisionFunction([this](sf::Vector2f pos, float angle)
                                { return this->checkCollisions(pos, angle); });
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
        if (current == GameState::TimeTrial)
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
        {
            if (lapData.laps == totalLaps)
                saveLapTime(lapData);
            engineAudio.stop();
            endscreen.stop();
            window.close();
        }
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), hudView);
            if (stateStack.top() == GameState::Home)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

                    std::vector<sf::FloatRect> hb = graphics->getHomeButtons();

                    for (size_t i = 0; i < hb.size(); ++i)
                    {
                        if (hb[i].contains(mappedMousePos))
                        {
                            if (i == 2)
                                stateStack.push(GameState::TimeTrial);
                            // else if (i == 1)
                            //     stateStack.push(GameState::Multiplayer);
                            // else if (i == 2)
                            //     stateStack.push(GameState::Leaderboard);
                            // else if (i == 3)
                            //     stateStack.push(GameState::Settings);

                            break; // Exit loop once button action triggers
                        }
                    }
                }
            }
            else if (stateStack.top() == GameState::LevelComplete)
            {
                std::vector<sf::FloatRect> levelCompleteButtons = graphics->getLevelCompleteButtons();
                if (levelCompleteButtons[0].contains(mousePos))
                {
                    saveLapTime(lapData);
                    lapData = LapTime();
                    resetLevel();
                    stateStack.pop();
                }
                else if (levelCompleteButtons[1].contains(mousePos))
                {
                    saveLapTime(lapData);
                    endscreen.stop();
                    window.close();
                }
            }
            else if (stateStack.top() == GameState::Paused)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

                const auto &pauseScreenButtons = graphics->getPauseScreenButtons();

                // 1. Check CONTINUE Button (Index 0)
                if (pauseScreenButtons[0].contains(mappedMousePos))
                {
                    stateStack.pop(); // Remove Pause state, resumes playing
                    engineAudio.play();
                }
                // 2. Check EXIT TO MENU Button (Index 1)
                else if (pauseScreenButtons[1].contains(mappedMousePos))
                {
                    resetLevel();
                    // Unwind the stack back down to the Home Screen
                    while (stateStack.size() > 1)
                    {
                        stateStack.pop();
                    }
                    stateStack.push(GameState::Home);
                    engineAudio.stop();
                }
            }
            else if (stateStack.top() == GameState::TimeTrial)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

                // Check if player clicked the top-right HUD pause icon
                if (graphics->getPauseButton().contains(mappedMousePos))
                {
                    stateStack.push(GameState::Paused);
                    engineAudio.pause();
                }
            }
        }
    }
}
void Game::handlePlayerMovement(float dt)
{
    if (window.hasFocus())
    {
        auto xInput = carInput::None;
        auto yInput = carInput::None;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            xInput = carInput::Left;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            xInput = carInput::Right;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            yInput = carInput::Up;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            yInput = carInput::Down;
        }
        float targetVolume = player.handleMovement(dt, xInput, yInput, track.getFriction());
        engineAudio.setVolume(lerp(engineAudio.getVolume(), targetVolume, 10.f * dt));
    }
}

void Game::update(float dt)
{
    if (stateStack.top() == GameState::TimeTrial)
    {
        handlePlayerMovement(dt);
        totalRaceTime += dt;
        currentLapTime += dt;
        if (player.updateWaypoint(waypoints))
        {
            currentLap++;
            player.setCurrLap(currentLap);
            if (currentLap > 1)
            {
                lapData.totalTime += currentLapTime;
                if (currentLapTime < lapData.bestLap)
                    lapData.bestLap = currentLapTime;
                currentLapTime = 0.0f;
                lapData.laps++;
            }

            if (currentLap > totalLaps)
            {
                engineAudio.pause();
                endscreen.play();
                stateStack.push(GameState::LevelComplete);
            }
        }
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

void Game::saveLapTime(const LapTime &lt)
{
    std::ofstream file("scores.txt", std::ios::app);
    if (file.is_open())
    {
        file << lt.id << " " << lt.laps << " "
             << lt.bestLap << " " << lt.totalTime << "\n";
        file.close();
    }
}

std::vector<LapTime> Game::loadLapTimes()
{
    std::vector<LapTime> times;
    std::ifstream file("scores.txt");
    if (file.is_open())
    {
        LapTime lt;
        while (file >> lt.id >> lt.laps >> lt.bestLap >> lt.totalTime)
            times.push_back(lt);
        file.close();
    }
    return times;
}

bool Game::carPosition(const Car &a, const Car &b)
{
    if (a.getCurrLap() != b.getCurrLap())
        return a.getCurrLap() > b.getCurrLap();
    if (a.getCurrWaypointIndex() != b.getCurrWaypointIndex())
        return a.getCurrWaypointIndex() > b.getCurrWaypointIndex();
    const Waypoint &w = waypoints[a.getCurrWaypointIndex()];
    return distance(a.getPosition(), w.mid) < distance(b.getPosition(), w.mid);
}

void Game::render()
{
    window.clear(sf::Color::Black);
    if (stateStack.top() == GameState::Home)
    {
        graphics->renderHomeScreen();
    }
    else if (stateStack.top() == GameState::TimeTrial)
    {
        graphics->renderGamePlay();
        graphics->renderHUD(totalRaceTime, currentLap, totalLaps, currentLapTime, lapData);
    }
    else if (stateStack.top() == GameState::LevelComplete)
    {
        graphics->renderLevelComplete(lapData, loadLapTimes());
    }
    else if (stateStack.top() == GameState::Paused)
    {
        graphics->renderPauseScreen();
    }
}
