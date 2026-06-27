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
    bestLap = BESTLAP_INIT_VAL;
    bestLapHolder = nullptr;
    countdownMode = true;
    countdownTime = 3.f;
    raceLeaderboard.clear();
    const CarPreset &selected = carPresets[(size_t)selectedCarLvl];
    for (auto &car : cars)
    {
        car->setMaxSpeed(selected.maxSpeed);
        car->setAcc(selected.AccRate);
        car->setMaxReverseSpeed(-100.f);
        car->setAngle(90.f);
        car->setCurrSpeed(0.f);
        car->resetWaypointIndex();
        car->setCurrLap(1);
        car->resetAllLapTime();
    }
    totalLaps = maxLaps[(size_t)selectedLaps];
    totalRaceTime = 0.f;
    const float y1 = 2550.f;
    const float y2 = 2600.f;
    player1.setPosition(sf::Vector2f(1620.f, y1));
    if (selectedMode == Gamemode::PVP)
    {
        player2.setPosition(sf::Vector2f(1620.f, y2));
    }
    else if (selectedMode == Gamemode::AI)
    {
        for (size_t i = 2; i < cars.size(); i += 2)
        {
            float xPos = 1610.f - (i - 1) * 30.f;
            cars[i]->setPosition(sf::Vector2f(xPos, y1));
            if (i + 1 < cars.size())
                cars[i + 1]->setPosition(sf::Vector2f(xPos, y2));
        }
    }
    for (size_t i = 0; i < cars.size(); ++i)
    {
        if (selectedMode == Gamemode::TimeTrial)
        {
            cars[i]->setActive(i == 0);
            if (i == 0)
                raceLeaderboard.push_back(cars[0]);
        }
        else if (selectedMode == Gamemode::PVP)
        {
            if (i == 0 || i == 1)
            {
                cars[i]->setActive(true);
                cars[i]->setRacePos(i + 1);
                raceLeaderboard.push_back(cars[i]);
            }
            else
                cars[i]->setActive(false);
        }
        else if (selectedMode == Gamemode::AI)
        {
            if (i == 1)
                cars[i]->setActive(false);
            else
            {
                cars[i]->setActive(true);
                cars[i]->setRacePos(i + 1);
                raceLeaderboard.push_back(cars[i]);
            }
        }
    }
}

void Game::init()
{
    std::srand((unsigned)std::time(0));

    window.create(sf::VideoMode(1200, 800), "Rapid Racing", sf::Style::Default);
    window.setFramerateLimit(60);

    track.LoadTrack("assets/textures/track1.png");
    waypoints = track.getWaypoints();

    player1.load("assets/textures/detail.png", "assets/textures/base.png");
    player2.load("assets/textures/detail.png", "assets/textures/base.png");
    player1.setTitle("PL1");
    player2.setTitle("PL2");
    cars.push_back(&player1);
    cars.push_back(&player2);
    for (size_t i = 0; i < (size_t)aiCount; i++)
    {
        AI *newAi = new AI();
        newAi->load("assets/textures/detail.png", "assets/textures/base.png");
        newAi->aiController = std::make_unique<AIController>(newAi, waypoints);
        newAi->setTitle("AI" + std::to_string(i + 1));
        cars.push_back(newAi);
    }

    if (!engineAudio.openFromFile("assets/audio/engine.ogg"))
        throw std::runtime_error("Engine sound not found\n");
    engineAudio.setLoop(true);

    if (!endscreen.openFromFile("assets/audio/endscreen.ogg"))
        throw std::runtime_error("Endscreen music file not found\n");
    endscreen.setLoop(true);
    endscreen.setVolume(70.f);

    if (!homeAudio.openFromFile("assets/audio/homescreen.ogg"))
        throw std::runtime_error("Homescreen music not found");
    homeAudio.setLoop(true);
    homeAudio.setVolume(70.f);

    selectedCarLvl = 2;
    selectedLaps = 0;

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

    graphics->setCarColors(cars);
    stateManager = std::make_unique<StateManager>(engineAudio, endscreen, homeAudio);
    stateManager->pushHome();
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
        GameState current = stateManager->getCurrentState();
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
            stateManager->stopAudio();
            window.close();
        }
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), hudView);
            if (stateManager->getCurrentState() == GameState::Home)
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
                                stateManager->pushPlaying();
                            }
                            else if (i == 3)
                                stateManager->pushSettings();

                            break; // Exit loop once button action triggers
                        }
                    }
                }
            }
            else if (stateManager->getCurrentState() == GameState::LevelComplete)
            {
                std::vector<sf::FloatRect> levelCompleteButtons = graphics->getLevelCompleteButtons();
                if (levelCompleteButtons[0].contains(mousePos))
                {
                    resetLevel();
                    stateManager->clear();
                    stateManager->pushPlaying();
                }
                else if (levelCompleteButtons[1].contains(mousePos))
                {
                    stateManager->clear();
                }
            }
            else if (stateManager->getCurrentState() == GameState::Paused)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

                const auto &pauseScreenButtons = graphics->getPauseScreenButtons();

                // 1. Check CONTINUE Button (Index 0)
                if (pauseScreenButtons[0].contains(mappedMousePos))
                {
                    stateManager->pop();
                }
                // 2. Check EXIT TO MENU Button (Index 1)
                else if (pauseScreenButtons[1].contains(mappedMousePos))
                {
                    resetLevel();
                    stateManager->clear();
                }
            }
            else if (stateManager->getCurrentState() == GameState::Playing)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

                // Check if player clicked the top-right HUD pause icon
                if (graphics->getPauseButton().contains(mappedMousePos))
                {
                    stateManager->pushPause();
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
            else if (stateManager->getCurrentState() == GameState::Settings)
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
                            stateManager->toggleMute();
                        }
                        else if (i == 8)
                        {
                            stateManager->pop();
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

        // FOR DEBUG ONLY, REMOVE LATER
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
        {
            stateManager->pushLevelComplete();
        }
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
            if (stateManager->getCurrVol() != 0.f)
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

            if (stateManager->getCurrVol() != 0.f)
            {
                engineAudio.setVolume(lerp(engineAudio.getVolume(), (vol1 + vol2) / 2.f, 10.f * dt));
            }
        }
    }
}
void Game::update(float dt)
{
    if (stateManager->getCurrentState() == GameState::Playing)
    {
        if (countdownMode)
        {
            countdownTime -= dt;
            if (countdownTime <= 0.01)
                countdownMode = false;
            return;
        }
        std::sort(raceLeaderboard.begin(), raceLeaderboard.end(),
                  [this](Car *a, Car *b)
                  { return carPosition(*a, *b); });

        handlePlayerMovement(dt);
        totalRaceTime += dt;

        if (selectedMode == Gamemode::TimeTrial)
        {
            player1.updateCarTimes(dt);
            checkWinner(&player1);
        }

        else if (selectedMode == Gamemode::PVP)
        {
            updateRacePositions();
            for (size_t i = 0; i < cars.size(); i++)
            {
                if (!cars[i]->getActive())
                    continue;
                cars[i]->updateCarTimes(dt);
                checkWinner(cars[i]);
            }
        }
        else if (selectedMode == Gamemode::AI)
        {
            for (size_t i = 0; i < cars.size(); i++)
            {
                if (!cars[i]->isAI() || !cars[i]->getActive())
                    continue;
                AI *aiCar = static_cast<AI *>(cars[i]);
                if (aiCar && aiCar->aiController)
                {
                    aiCar->aiController->update(cars, dt);
                    aiCar->handleAIMovement(dt, track.getFriction());
                }
            }
            updateRacePositions();
            for (size_t i = 0; i < cars.size(); i++)
            {
                if (!cars[i]->getActive())
                    continue;
                cars[i]->updateCarTimes(dt);
                if (cars[i]->isStuck() && cars[i] != &player1)
                    cars[i]->resetPosition(waypoints);
                checkWinner(cars[i]);
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
    if (stateManager->getCurrentState() == GameState::Home)
    {
        graphics->renderHomeScreen();
    }
    else if (stateManager->getCurrentState() == GameState::Playing)
    {
        if (selectedMode == Gamemode::TimeTrial)
        {
            graphics->renderGamePlay(cars);
            graphics->renderHUD(totalRaceTime, player1.getCurrLap(), totalLaps);
            if (player1.isStuck())
                graphics->renderResetButton(selectedMode);
        }
        else if (selectedMode == Gamemode::PVP)
        {
            graphics->renderPVPGameplay(player2);
            graphics->renderPVPHUD(player2, totalLaps, totalRaceTime);
            graphics->renderResetButton(selectedMode, player1.isStuck(), player2.isStuck());
        }
        else if (selectedMode == Gamemode::AI)
        {
            graphics->renderGamePlay(cars);
            graphics->renderAIHUD(raceLeaderboard, totalLaps, totalRaceTime);
            if (player1.isStuck())
                graphics->renderResetButton(selectedMode);
        }
        graphics->renderMinimap(cars, selectedMode);
        if (countdownMode)
            graphics->renderCountdown(countdownTime);
    }
    else if (stateManager->getCurrentState() == GameState::LevelComplete)
    {
        if (!bestLapHolder)
        {
            bestLapHolder = &player1;
            bestLap = BESTLAP_INIT_VAL;
        }
        if (selectedMode == Gamemode::TimeTrial)
            graphics->renderLevelComplete(bestLapHolder->getLapData(), leaderboardManager.loadLapTimes());
        else if (selectedMode == Gamemode::PVP)
        {
            graphics->renderPVPLvlComplete(player2, raceLeaderboard);
        }
        else if (selectedMode == Gamemode::AI)
        {
            graphics->renderAILvlComplete(player1, bestLapHolder->getLapData(), raceLeaderboard);
        }
    }
    else if (stateManager->getCurrentState() == GameState::Paused)
    {
        graphics->renderPauseScreen();
    }
    else if (stateManager->getCurrentState() == GameState::Settings)
    {
        graphics->renderSettingsScreen(selectedCarLvl, selectedLaps, stateManager->getCurrVol() == 0.f);
    }
}

void Game::checkWinner(Car *player) // updateWaypoint handled here
{
    if (player->updateWaypoint(waypoints))
    {
        player->incrementLaps();
        updateBestLap();
        player->resetCurrentLapTime();
        if (player1.getCurrLap() > totalLaps || (selectedMode == Gamemode::PVP && player2.getCurrLap() > totalLaps))
        {
            if (!player->isAI())
            {
                if (bestLapHolder && !bestLapHolder->isAI())
                {
                    leaderboardManager.saveLapTime(bestLapHolder->getLapData());
                }
                else
                    leaderboardManager.saveLapTime(player->getLapData());
                stateManager->pushLevelComplete();
            }
        }
    }
}

void Game::updateRacePositions()
{
    for (size_t i = 0; i < raceLeaderboard.size(); ++i)
    {
        raceLeaderboard[i]->setRacePos(i + 1);
    }
}

void Game::updateBestLap()
{
    for (Car *car : cars)
    {
        if (car->getActive() && car->getCurrLap() > 0 && car->getBestLapTime() < bestLap)
        {
            bestLap = car->getBestLapTime();
            bestLapHolder = car;
        }
    }
}