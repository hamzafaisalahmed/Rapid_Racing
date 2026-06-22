#include "Game.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <ctime>
#include <stdexcept>
#include <functional>
#include "AIController.h"

void Game::resetLevel()
{
    const CarPreset &selected = carPresets[(size_t)selectedCarLvl];
    for (auto &car : cars)
    {
        car->setMaxSpeed(selected.maxSpeed);
        car->setAcc(selected.AccRate);
        car->setMaxReverseSpeed(-100.f);
        car->setAngle(90.f);
        car->setCurrSpeed(0.f);
        car->resetWaypointIndex();
    }
    winner = 0;
    currentLap = 1;
    totalLaps = maxLaps[(size_t)selectedLaps];
    currentLapTime = 0.f;
    endscreen.stop();
    engineAudio.setVolume(0.f);
    if (audioMuted)
    {
        endscreen.setVolume(0.f);
    }
    else
    {
        endscreen.setVolume(70.f);
    }
    engineAudio.play();
    totalRaceTime = 0.f;
    if (selectedMode == Gamemode::TimeTrial)
    {
        player1.setPosition(sf::Vector2f(1620.f, 2550.f));
    }
    else if (selectedMode == Gamemode::PVP)
    {
        player1.setPosition(sf::Vector2f(1620.f, 2550.f));
        player2.setPosition(sf::Vector2f(1620.f, 2600.f));
    }
    else if (selectedMode == Gamemode::AI)
    {
        player1.setPosition(sf::Vector2f(1620.f, 2550.f));
        ai1.setPosition(sf::Vector2f(1620.f, 2600.f));
    }
    // int activePlayerCount = 0;
    // if (selectedMode == Gamemode::TimeTrial)
    // {
    //     activePlayerCount = 1;
    // }
    // else if (selectedMode == Gamemode::PVP)
    // {
    //     activePlayerCount = 2;
    // }
    // else if (selectedMode == Gamemode::AI)
    // {
    //     activePlayerCount = 1; // player1 + aiCar (or however many)
    // }

    // // Set active status
    // for (size_t i = 0; i < cars.size(); ++i)
    // {
    //     if (selectedMode == Gamemode::PVP)
    //         cars[i]->setActive(i < (size_t)activePlayerCount);
    //     else if (selectedMode == Gamemode::AI)
    //         cars[i]->setActive(i != 1);
    // }

    if (selectedMode == Gamemode::TimeTrial)
    {
        player1.setActive(true);
        player2.setActive(false);
        ai1.setActive(false);
    }
    else if (selectedMode == Gamemode::PVP)
    {
        player1.setActive(true);
        player2.setActive(true);
        ai1.setActive(false);
    }
    else if (selectedMode == Gamemode::AI)
    {
        player1.setActive(true);
        player2.setActive(false);
        ai1.setActive(true);
    }
    lapData = LapTime();
}

void Game::init()
{
    std::srand((unsigned)std::time(0));

    window.create(sf::VideoMode(1200, 800), "Rapid Racing", sf::Style::Default);
    window.setFramerateLimit(60);

    track.LoadTrack("assets/textures/track1.png");
    player1.load("assets/textures/car1.png");
    player2.load("assets/textures/car2.png");
    ai1.load("assets/textures/car2.png");

    cars.push_back(&player1);
    cars.push_back(&player2);
    cars.push_back(&ai1);

    waypoints = track.getWaypoints();

    ai1.aiController = std::make_unique<AIController>(&ai1, waypoints);

    if (!engineAudio.openFromFile("assets/audio/engine.ogg"))
        throw std::runtime_error("Engine sound not found\n");
    engineAudio.setLoop(true);

    if (!endscreen.openFromFile("assets/audio/endscreen.ogg"))
        throw std::runtime_error("Endscreen music file not found\n");
    endscreen.setLoop(true);
    endscreen.setVolume(70.f);

    selectedCarLvl = 1;
    selectedLaps = 1;
    audioMuted = false;

    hudView = sf::View(sf::FloatRect(0.f, 0.f, 1200.f, 800.f));

    // Set up game view to show a reasonable portion of the track
    // View should be smaller than track to allow zooming on player
    gameView = sf::View(sf::FloatRect(0.f, 0.f, 300.f, 200.f));
    gameView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    window.setView(gameView);
    graphics = std::make_unique<Graphics>(window, gameView, hudView, track, player1);
    graphics->init();
    collisionHandler = std::make_unique<CollisionHandler>(track, cars);
    for (Car *car : cars)
    {
        car->setCollisionFunction([this](Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt)
                                  { return this->collisionHandler->handleCollision(car, pos, angle, oldPos, oldAngle, dt); });
    }
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
        {
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
                            if (i < 3)
                            {
                                if (i == 0)
                                    selectedMode = Gamemode::AI;
                                else if (i == 1)
                                    selectedMode = Gamemode::PVP;
                                else if (i == 2)
                                    selectedMode = Gamemode::TimeTrial;
                                resetLevel();
                                stateStack.push(GameState::Playing);
                            }
                            else if (i == 3)
                                stateStack.push(GameState::Settings);

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
                    resetLevel();
                    stateStack.pop();
                }
                else if (levelCompleteButtons[1].contains(mousePos))
                {
                    engineAudio.stop();
                    endscreen.stop();
                    while (stateStack.size() > 0)
                        stateStack.pop();
                    stateStack.push(GameState::Home);
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
            else if (stateStack.top() == GameState::Playing)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

                // Check if player clicked the top-right HUD pause icon
                if (graphics->getPauseButton().contains(mappedMousePos))
                {
                    stateStack.push(GameState::Paused);
                    engineAudio.pause();
                }

                if (selectedMode != Gamemode::PVP && player1.isStuck() && graphics->getResetButton().contains(mappedMousePos))
                {
                    player1.resetPosition(waypoints);
                }
                else
                {
                    const auto &ResetButtons = graphics->getResetButtonsPVP();
                    if (player1.isStuck() && ResetButtons[0].contains(mappedMousePos))
                    {
                        player1.resetPosition(waypoints);
                    }
                    if (player2.isStuck() && ResetButtons[1].contains(mappedMousePos))
                    {
                        player2.resetPosition(waypoints);
                    }
                }
            }
            else if (stateStack.top() == GameState::Settings)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mp = window.mapPixelToCoords(mousePos, hudView);

                const auto &buttons = graphics->getSettingsButtons();

                for (size_t i = 0; i < buttons.size(); ++i)
                {
                    if (buttons[i].contains(mp))
                    {
                        if (i <= 2)
                        {
                            selectedCarLvl = i;
                        }
                        else if (i <= 6)
                        {
                            selectedLaps = i - 3;
                        }
                        else if (i == 7)
                        {
                            audioMuted = !audioMuted;
                        }
                        else if (i == 8)
                        {
                            stateStack.pop();
                        }
                        break;
                    }
                }
            }
        }
    }
    if (window.hasFocus())
    {
        if (player1.isStuck() && sf::Keyboard::isKeyPressed(sf::Keyboard::R))
            player1.resetPosition(waypoints);
        if (selectedMode == Gamemode::PVP && player2.isStuck() && sf::Keyboard::isKeyPressed(sf::Keyboard::M))
            player2.resetPosition(waypoints);
    }
}
void Game::handlePlayerMovement(float dt)
{
    if (window.hasFocus())
    {
        if (selectedMode == Gamemode::TimeTrial || selectedMode == Gamemode::AI)
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
            float targetVolume = player1.handleMovement(dt, xInput, yInput, track.getFriction());
            if (!audioMuted)
                engineAudio.setVolume(lerp(engineAudio.getVolume(), targetVolume, 10.f * dt));
        }
        else if (selectedMode == Gamemode::PVP)
        {
            auto p1x = carInput::None;
            auto p1y = carInput::None;
            auto p2x = carInput::None;
            auto p2y = carInput::None;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                p1x = carInput::Left;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                p1x = carInput::Right;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                p1y = carInput::Up;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                p1y = carInput::Down;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            {
                p2x = carInput::Left;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            {
                p2x = carInput::Right;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            {
                p2y = carInput::Up;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            {
                p2y = carInput::Down;
            }
            float vol1 = player1.handleMovement(dt, p1x, p1y, track.getFriction());
            float vol2 = player2.handleMovement(dt, p2x, p2y, track.getFriction());

            if (!audioMuted)
            {
                engineAudio.setVolume(lerp(engineAudio.getVolume(), (vol1 + vol2) / 2.f, 10.f * dt));
            }
        }
    }
}
void Game::update(float dt)
{
    if (stateStack.top() == GameState::Playing)
    {
        if (selectedMode == Gamemode::TimeTrial)
        {
            handlePlayerMovement(dt);
            totalRaceTime += dt;
            currentLapTime += dt;
            player1.updateStuckTime(dt);
            player1.updateITime(dt);
            if (player1.updateWaypoint(waypoints))
            {
                currentLap++;
                player1.setCurrLap(currentLap);
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
                    leaderboardManager.saveLapTime(lapData);
                    stateStack.push(GameState::LevelComplete);
                }
            }
        }
        else // Sort cars to process trailing vehicles first (natural chain reaction logic)
            std::sort(cars.begin(), cars.end(), [](Car *a, Car *b)
                      { return a->getRacePos() > b->getRacePos(); });
        if (selectedMode == Gamemode::PVP)
        {
            handlePlayerMovement(dt);
            totalRaceTime += dt;
            updateRacePositions();
            for (size_t i = 0; i < cars.size(); i++)
            {
                if (!cars[i]->getActive())
                    continue;
                cars[i]->updateStuckTime(dt);
                cars[i]->updateITime(dt);
                checkWinner(cars[i], i + 1);
            }
        }
        else if (selectedMode == Gamemode::AI)
        {
            handlePlayerMovement(dt);
            ai1.aiController->update(cars, dt);
            ai1.handleMovement(dt, ai1.aiController->getHorizontalInput(), ai1.aiController->getVerticalInput(), track.getFriction());

            totalRaceTime += dt;
            updateRacePositions();
            for (size_t i = 0; i < cars.size(); i++)
            {
                if (!cars[i]->getActive())
                    continue;
                cars[i]->updateStuckTime(dt);
                cars[i]->updateITime(dt);
                checkWinner(cars[i], i + 1);
            }
        }
    }
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
    else if (stateStack.top() == GameState::Playing)
    {
        if (selectedMode == Gamemode::TimeTrial || selectedMode == Gamemode::AI)
        {
            graphics->renderGamePlay(cars);
            graphics->renderHUD(totalRaceTime, currentLap, totalLaps, currentLapTime, lapData);
            if (player1.isStuck())
                graphics->renderResetButton(selectedMode);
        }
        else if (selectedMode == Gamemode::PVP)
        {
            graphics->renderPVPGameplay(player2);
            graphics->renderPVPHUD(player2, totalLaps, totalRaceTime);
            graphics->renderResetButton(selectedMode, player1.isStuck(), player2.isStuck());
        }

        graphics->renderMinimap(cars, selectedMode);
    }
    else if (stateStack.top() == GameState::LevelComplete)
    {
        if (selectedMode == Gamemode::TimeTrial)
            graphics->renderLevelComplete(lapData, leaderboardManager.loadLapTimes());
        else if (selectedMode == Gamemode::PVP)
        {
            graphics->renderPVPLvlComplete(player2);
        }
    }
    else if (stateStack.top() == GameState::Paused)
    {
        graphics->renderPauseScreen();
    }
    else if (stateStack.top() == GameState::Settings)
    {
        graphics->renderSettingsScreen(selectedCarLvl, selectedLaps, audioMuted);
    }
}

void Game::checkWinner(Car *player, int playerNo)
{
    if (player->updateWaypoint(waypoints))
    {
        player->incrementLaps();
        if (player->getCurrLap() > totalLaps)
        {
            winner = playerNo;
            engineAudio.pause();
            endscreen.play();
            stateStack.push(GameState::LevelComplete);
        }
    }
}

void Game::updateRacePositions()
{
    std::vector<Car *> sorted = cars;

    std::sort(sorted.begin(), sorted.end(),
              [this](Car *a, Car *b)
              { return carPosition(*a, *b); });

    for (size_t i = 0; i < sorted.size(); ++i)
    {
        sorted[i]->setRacePos(i + 1);
    }
}
