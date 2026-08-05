#include "Game.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <ctime>
#include <stdexcept>
#include <functional>
#include <random>
#include "AIController.h"
#include "WaypointHandler.h"
#include "Utils.h"
#include "TrackLoader.h"
void Game::resetLevel()
{
    if (currentTrackPathIndex != selectedTrackPathIndex)
    {
        currentTrackPathIndex = selectedTrackPathIndex;
        trackInit();
    }
    spectatorTarget = nullptr;
    spectatorModeToggled = aiSpectatorMode;
    bestLap = BESTLAP_INIT_VAL;
    bestLapHolder = nullptr;
    countdownMode = true;
    countdownTime = 3.f;
    raceLeaderboard.clear();
    const CarPreset &selected = carPresets[(size_t)selectedCarLvl];

    float difficultyDelay[3] = {0.02f, 0.005f, 0.f}; // Easy, Medium, Hard

    float diffMultiplier = 1.f;
    if (selectedDifficulty == 0) // Easy
        diffMultiplier = 0.9f;
    else if (selectedDifficulty == 1) // Medium
        diffMultiplier = 1.f;
    else if (selectedDifficulty == 2) // Hard
        diffMultiplier = 1.1f;
    for (auto &car : cars)
    {
        if (car->isAI())
        {
            car->setMaxSpeed(selected.maxSpeed * diffMultiplier);
            car->setAcc(selected.AccRate * diffMultiplier);
        }
        else
        {
            car->setMaxSpeed(selected.maxSpeed);
            car->setAcc(selected.AccRate);
        }
        car->setMaxReverseSpeed(-100.f);
        car->setAngle(track.getStartAngle());
        car->setCurrSpeed(0.f);
        car->resetWaypointIndex();
        car->setCurrLap(1);
        car->resetAllLapTime();
        car->setFinishedRace(false);
        car->setTrackID(track.getID());
        if (car->isAI())
        {
            AI *aiCar = static_cast<AI *>(car);
            if (aiCar->aiController)
            {
                aiCar->aiController->reset(track.getScaleFactor());
                aiCar->aiController->setReactionDelay(difficultyDelay[selectedDifficulty]);
            }
        }
    }
    totalLaps = maxLaps[(size_t)selectedLaps];
    totalRaceTime = 0.f;

    const float posA = track.getStartPosA();
    const float posB1 = track.getStartPosB1();
    const float posB2 = track.getStartPosB2();
    const float rowSpacing = track.getStartRowSpacing();
    const float startAngle = track.getStartAngle();
    auto getGridPosition = [&](int slot) -> sf::Vector2f
    {
        float lane = (slot % 2 == 0) ? posB1 : posB2;
        float rowOffset = (slot / 2) * rowSpacing;
        int angle = normalizeAngle(startAngle);

        switch (angle)
        {
        case 0:
            return {lane, posA + rowOffset}; // Facing Up: A is Y, B is X
        case 180:
            return {lane, posA - rowOffset}; // Facing Down: A is Y, B is X
        case 270:
            return {posA + rowOffset, lane}; // Facing Left: A is X, B is Y
        case 90:                             // Facing Right: A is X, B is Y
        default:
            return {posA - rowOffset, lane};
        }
    };

    if (selectedMode == Gamemode::PVP)
    {
        player1.setPosition(getGridPosition(0));
        player2.setPosition(getGridPosition(1));
    }
    else
    {
        // ── Build the list of cars that need a shuffled grid slot ──
        std::vector<Car *> gridCars;
        if (selectedMode == Gamemode::TimeTrial)
        {
            gridCars.push_back(&player1);
        }
        else if (selectedMode == Gamemode::AI)
        {
            if (!aiSpectatorMode)
                gridCars.push_back(&player1);

            for (size_t i = 2; i < cars.size(); ++i)
            {
                int aiIndex = (int)i - 2;
                if (aiIndex < selectedAICount)
                    gridCars.push_back(cars[i]);
            }
        }

        static std::mt19937 gridRng(std::random_device{}());
        std::shuffle(gridCars.begin(), gridCars.end(), gridRng);

        for (size_t slot = 0; slot < gridCars.size(); ++slot)
        {
            gridCars[slot]->setPosition(getGridPosition((int)slot));
        }
    }

    // ── Active flags / race positions / leaderboard (unchanged logic) ──
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
                cars[i]->setRacePos((int)i + 1);
                raceLeaderboard.push_back(cars[i]);
            }
            else
                cars[i]->setActive(false);
        }
        else if (selectedMode == Gamemode::AI)
        {
            if (i == 1)
                cars[i]->setActive(false);
            else if (i >= 2)
            {
                int aiIndex = (int)i - 2;
                bool shouldBeActive = (aiIndex < selectedAICount);
                cars[i]->setActive(shouldBeActive);
                if (shouldBeActive)
                {
                    cars[i]->setRacePos((int)raceLeaderboard.size() + 1);
                    raceLeaderboard.push_back(cars[i]);
                }
            }
            else
            {
                cars[i]->setActive(!aiSpectatorMode);
                if (!aiSpectatorMode)
                {
                    cars[i]->setRacePos((int)raceLeaderboard.size() + 1);
                    raceLeaderboard.push_back(cars[i]);
                }
            }
        }
    }

    totalRacers = (int)raceLeaderboard.size();
    finishedCount = 0;
    spectatorTarget = raceLeaderboard.empty() ? nullptr : raceLeaderboard.front();
}

void Game::init()
{
    std::srand((unsigned)std::time(0));

    window.create(sf::VideoMode(1200, 800), "Rapid Racing", sf::Style::Default);
    window.setFramerateLimit(60);

    player1.load("assets/textures/detail.png", "assets/textures/base.png");
    player2.load("assets/textures/detail.png", "assets/textures/base.png");
    player1.setTitle("PL1");
    player2.setTitle("PL2");
    cars.push_back(&player1);
    cars.push_back(&player2);
    for (size_t i = 0; i < (size_t)selectedAICount;
         i++)
    {
        AI *newAi = new AI();
        newAi->load("assets/textures/detail.png", "assets/textures/base.png");
        // ── PASS gridSlot as (int)i ──
        newAi->aiController = std::make_unique<AIController>(newAi, waypoints, wpHandler, (int)i);
        newAi->setTitle("AI" + std::to_string(i + 1));
        newAi->aiController->setPreferredLane(static_cast<TargetSide>(std::rand() % 3));
        cars.push_back(newAi);
    }

    if (!loadMusic(engineAudio, "assets/audio/engine.ogg"))
        throw std::runtime_error("Engine sound not found\n");
    engineAudio.setLoop(true);

    if (!loadMusic(ambientEngineAudio, "assets/audio/engine.ogg"))
        throw std::runtime_error("Ambient engine sound not found\n");
    ambientEngineAudio.setLoop(true);
    ambientEngineAudio.setVolume(0.f);

    if (!loadMusic(endscreen, "assets/audio/endscreen.ogg"))
        throw std::runtime_error("Endscreen music file not found\n");
    endscreen.setLoop(true);
    endscreen.setVolume(70.f);

    if (!loadMusic(homeAudio, "assets/audio/homescreen.ogg"))
        throw std::runtime_error("Homescreen music not found");
    homeAudio.setLoop(true);
    homeAudio.setVolume(70.f);

    selectedCarLvl = 1;
    selectedLaps = 0;

    trackPaths = {
        "assets/tracks/track1.json",
        "assets/tracks/track2.json"
        // Add more track paths here as needed
    };

    hudView = sf::View(sf::FloatRect(0.f, 0.f, 1200.f, 800.f));

    // Set up game view to show a reasonable portion of the track
    // View should be smaller than track to allow zooming on player
    gameView = sf::View(sf::FloatRect(0.f, 0.f, 300.f, 200.f));
    gameView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    window.setView(gameView);
    graphics = std::make_unique<Graphics>(window, gameView, hudView, track);
    graphics->init();
    std::vector<std::string> trackImages;
    for (auto track : trackPaths)
    {
        trackImages.push_back(getTrackMinimapPath(track));
    }
    graphics->initTrackSelect(trackImages);
    collisionHandler = std::make_unique<CollisionHandler>(track, cars);
    for (Car *car : cars)
    {
        car->setCollisionFunction([this](Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt)
                                  { return this->collisionHandler->handleCollision(car, pos, angle, oldPos, oldAngle, dt); });
    }

    graphics->setCarColors(cars);
    stateManager = std::make_unique<StateManager>(engineAudio, endscreen, homeAudio, ambientEngineAudio);
    maxVol = stateManager->getMaxVol();
    stateManager->pushHome();
}
void Game::trackInit()
{
    trackLoader(track, trackPaths[(size_t)currentTrackPathIndex]);
    waypoints = track.getWaypoints();
    wpHandler.init(waypoints, carPresets[(size_t)selectedCarLvl].maxSpeed, track.getScaleFactor());
    graphics->loadMinimap();
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
            // ── Single, uniform mouse-position computation for every state branch ──
            if (event.mouseButton.button != sf::Mouse::Left)
                continue; // nothing below handles right/middle click; skip early

            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), hudView);

            if (stateManager->getCurrentState() == GameState::Home)
            {
                const std::vector<sf::FloatRect> &hb = graphics->getHomeButtons();

                for (size_t i = 0; i < hb.size(); ++i)
                {
                    if (hb[i].contains(mousePos))
                    {
                        if (i < 3)
                        {
                            if (i == 0)
                            {
                                selectedMode = Gamemode::AI;
                                stateManager->pushAISetup();
                                break;
                            }
                            else if (i == 1)
                                selectedMode = Gamemode::PVP;
                            else if (i == 2)
                                selectedMode = Gamemode::TimeTrial;

                            stateManager->pushTrackSelect();
                        }
                        else if (i == 3)
                            stateManager->pushSettings();

                        break;
                    }
                }
            }
            else if (stateManager->getCurrentState() == GameState::LevelComplete)
            {
                const std::vector<sf::FloatRect> &levelCompleteButtons = graphics->getLevelCompleteButtons();
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
                const auto &pauseScreenButtons = graphics->getPauseScreenButtons();

                if (pauseScreenButtons[0].contains(mousePos))
                {
                    stateManager->pop();
                }
                else if (pauseScreenButtons[1].contains(mousePos))
                {
                    resetLevel();
                    stateManager->clear();
                }
            }
            else if (stateManager->getCurrentState() == GameState::Playing)
            {
                if (graphics->getPauseButton().contains(mousePos))
                {
                    stateManager->pushPause();
                }

                if (selectedMode != Gamemode::PVP && player1.isStuck() && graphics->getResetButton().contains(mousePos))
                {
                    player1.resetPosition(waypoints);
                }
                else
                {
                    const auto &resetButtons = graphics->getResetButtonsPVP();
                    if (player1.isStuck() && resetButtons[0].contains(mousePos))
                    {
                        player1.resetPosition(waypoints);
                    }
                    if (player2.isStuck() && resetButtons[1].contains(mousePos))
                    {
                        player2.resetPosition(waypoints);
                    }
                }
            }
            else if (stateManager->getCurrentState() == GameState::Settings)
            {
                const auto &buttons = graphics->getSettingsButtons();

                for (size_t i = 0; i < buttons.size(); ++i)
                {
                    if (buttons[i].contains(mousePos))
                    {
                        if (i <= 2)
                        {
                            selectedCarLvl = i;
                        }
                        else if (i <= 6)
                        {
                            selectedLaps = i - 3;
                        }
                        else if (i == 7) // Mute
                        {
                            stateManager->toggleMute();
                        }
                        else if (i == 8) // Debug
                        {
                            debugDisplay = !debugDisplay;
                            if (debugDisplay)
                                wpHandler.debugWaypointData(waypoints, track.getScaleFactor());
                        }
                        else if (i == 9) // Menu
                        {
                            stateManager->pop();
                        }
                        break;
                    }
                }
            }
            else if (stateManager->getCurrentState() == GameState::AISetup)
            {
                const auto &buttons = graphics->getAISetupButtons();

                for (size_t i = 0; i < buttons.size(); ++i)
                {
                    if (buttons[i].contains(mousePos))
                    {
                        if (i <= 6)
                        { // AI Count 1-7
                            selectedAICount = i + 1;
                        }
                        else if (i == 7)
                        { // Spectator
                            aiSpectatorMode = !aiSpectatorMode;
                        }
                        else if (i <= 10)
                        { // Difficulty
                            selectedDifficulty = i - 8;
                        }
                        else if (i == 11)
                        { // BACK
                            stateManager->pop();
                        }
                        else if (i == 12)
                        { // START RACE
                            stateManager->pushTrackSelect();
                        }
                        break;
                    }
                }
            }
            else if (stateManager->getCurrentState() == GameState::TrackSelect)
            {
                size_t numTracks = trackPaths.size();
                const auto &buttons = graphics->getTrackSelectButtons();

                for (size_t i = 0; i < numTracks; ++i)
                {
                    if (buttons[i].contains(mousePos))
                    {
                        selectedTrackPathIndex = static_cast<int>(i);
                        break;
                    }
                }

                if (buttons.size() > numTracks && buttons[numTracks].contains(mousePos))
                {
                    stateManager->pop();
                }
                else if (buttons.size() > numTracks + 1 && buttons[numTracks + 1].contains(mousePos))
                {
                    resetLevel();
                    stateManager->pop();
                    if (selectedMode == Gamemode::AI)
                        stateManager->pop();
                    stateManager->pushPlaying();
                }
            }
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            if (stateManager->getCurrentState() == GameState::Playing && selectedMode == Gamemode::AI)
            {
                bool spectating = aiSpectatorMode || spectatorModeToggled;

                if (spectating && event.key.code == sf::Keyboard::Escape)
                {
                    stateManager->pushLevelComplete();
                }
                else if (spectating && event.key.code == sf::Keyboard::Tab)
                {
                    Car *next = findSpectatorTarget(spectatorTarget);
                    if (next && next != spectatorTarget)
                        spectatorTarget = next;
                }
            }

            if (debugDisplay && stateManager->getCurrentState() == GameState::Playing && sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
            {
                stateManager->pushLevelComplete();
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
        if (selectedMode == Gamemode::TimeTrial || (selectedMode == Gamemode::AI && !aiSpectatorMode))
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
            player1.handleMovement(dt, xInput, yInput, track.getFriction());
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
            player1.handleMovement(dt, p1x, p1y, track.getFriction());
            player2.handleMovement(dt, p2x, p2y, track.getFriction());
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
        updateEngineAudio(dt);

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
                    aiCar->aiController->update(raceLeaderboard, dt);
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
                // Auto-switch camera if the current target just finished
                if ((aiSpectatorMode || spectatorModeToggled) && (!spectatorTarget || spectatorTarget->isFinishedRace()))
                {
                    spectatorTarget = findSpectatorTarget(spectatorTarget);
                }
            }
        }
    }
}

bool Game::carPosition(const Car &a, const Car &b)
{
    bool aFinished = !a.getActive();
    bool bFinished = !b.getActive();

    if (aFinished != bFinished)
        return aFinished; // finished cars always rank ahead of still-racing ones

    if (aFinished && bFinished)
        return a.getRacePos() < b.getRacePos(); // preserve fixed finish order

    // both still racing — existing track-progress logic, unchanged
    if (a.getCurrLap() != b.getCurrLap())
        return a.getCurrLap() > b.getCurrLap();
    if (a.getCurrWaypointIndex() != b.getCurrWaypointIndex())
        return a.getCurrWaypointIndex() > b.getCurrWaypointIndex();

    int currIdx = a.getCurrWaypointIndex();
    int prevIdx = (currIdx == 0) ? 0 : currIdx - 1;
    int dirIdx = (currIdx == 0) ? 1 : currIdx;

    sf::Vector2f segDir = normalize(waypoints[dirIdx].mid - waypoints[prevIdx].mid);
    float aProgress = dotProduct(a.getPosition() - waypoints[prevIdx].mid, segDir);
    float bProgress = dotProduct(b.getPosition() - waypoints[prevIdx].mid, segDir);

    return aProgress > bProgress;
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
            graphics->renderGamePlay(cars, &player1, debugDisplay);
            graphics->renderTTHUD(player1, totalRaceTime, player1.getCurrLap(), totalLaps);
            if (player1.isStuck())
                graphics->renderResetButton(selectedMode);
        }
        else if (selectedMode == Gamemode::PVP)
        {
            graphics->renderPVPGameplay(player1, player2, debugDisplay);
            graphics->renderPVPHUD(player1, player2, totalLaps, totalRaceTime);
            graphics->renderResetButton(selectedMode, player1.isStuck(), player2.isStuck());
        }
        else if (selectedMode == Gamemode::AI)
        {
            bool spectating = aiSpectatorMode || spectatorModeToggled;
            Car *cameraFocus = (spectating && spectatorTarget) ? spectatorTarget : &player1;

            graphics->renderGamePlay(cars, cameraFocus, debugDisplay);
            graphics->renderAIHUD(player1, raceLeaderboard, totalLaps, totalRaceTime, spectating);

            if (!spectating && player1.isStuck())
                graphics->renderResetButton(selectedMode);
        }
        if (debugDisplay)
            graphics->debugWaypointAI(wpHandler, track.getWaypoints());
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
            graphics->renderPVPLvlComplete(player1, player2, raceLeaderboard);
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
        graphics->renderSettingsScreen(selectedCarLvl, selectedLaps, stateManager->getCurrVol() == 0.f, debugDisplay);
    }
    else if (stateManager->getCurrentState() == GameState::AISetup)
    {
        graphics->renderAISetupScreen(selectedAICount, aiSpectatorMode, selectedDifficulty);
    }
    else if (stateManager->getCurrentState() == GameState::TrackSelect)
    {
        graphics->renderTrackSelectScreen(selectedTrackPathIndex);
    }
}

void Game::checkWinner(Car *car)
{
    if (car->updateWaypoint(waypoints, track.getScaleFactor()))
    {
        car->incrementLaps();
        car->resetCurrentLapTime(); // must run first so THIS lap's time is in lapData.bestLap
        updateBestLap();

        if (car->getCurrLap() > totalLaps && car->getActive())
        {
            car->setActive(false); // ← ADDED: stops AI update/movement for this car
            car->setFinishedRace(true);
            car->setRacePos(++finishedCount);

            if (!car->isAI())
            {
                leaderboardManager.saveLapTime(car->getLapData());
            }

            if (stateManager->getCurrentState() != GameState::Playing)
                return;

            if (selectedMode == Gamemode::AI)
            {
                if (!car->isAI())
                    spectatorModeToggled = true;

                if (finishedCount >= totalRacers)
                    stateManager->pushLevelComplete();
            }
            else
            {
                if (finishedCount >= 1)
                    stateManager->pushLevelComplete();
            }
        }
    }
}

void Game::updateRacePositions()
{
    for (size_t i = 0; i < raceLeaderboard.size(); ++i)
    {
        if (!raceLeaderboard[i]->getActive() || raceLeaderboard[i]->isFinishedRace())
            continue;
        raceLeaderboard[i]->setRacePos(i + 1);
    }
}

void Game::updateBestLap()
{
    for (Car *car : cars)
    {
        if ((car->getActive() || car->isFinishedRace()) && car->getCurrLap() > 0 && car->getBestLapTime() < bestLap)
        {
            bestLap = car->getBestLapTime();
            bestLapHolder = car;
        }
    }
}

Car *Game::findSpectatorTarget(Car *curr)
{
    std::vector<Car *> eligible;
    for (Car *c : raceLeaderboard)
        if (c->getActive() && !c->isFinishedRace())
            eligible.push_back(c);

    if (eligible.empty())
        return nullptr;
    if (eligible.size() == 1)
        return eligible[0];

    int currPos = curr ? curr->getRacePos() : 0;
    Car *best = nullptr;

    // find the eligible car with the smallest racePos strictly greater than curr's
    for (Car *c : eligible)
        if (c->getRacePos() > currPos && (!best || c->getRacePos() < best->getRacePos()))
            best = c;

    // wrap around: no one further back — go to whoever's currently leading
    if (!best)
        for (Car *c : eligible)
            if (!best || c->getRacePos() < best->getRacePos())
                best = c;

    return best;
}

void Game::updateEngineAudio(float dt)
{
    float masterVol = stateManager->getCurrVol();

    if (masterVol == 0.f)
    {
        engineAudio.setVolume(0.f);
        ambientEngineAudio.setVolume(0.f);
        return;
    }

    if (selectedMode == Gamemode::TimeTrial)
    {
        engineAudio.setVolume(clamp(lerp(engineAudio.getVolume(), player1.getAccelerating() ? masterVol : 0.f, 10.f * dt), 0.f, maxVol));
        ambientEngineAudio.setVolume(0.f);
    }
    else if (selectedMode == Gamemode::PVP)
    {
        // unchanged
        float vol1 = std::abs(player1.getCurrSpeed() / player1.getMaxSpeed()) * 100.f;
        float vol2 = std::abs(player2.getCurrSpeed() / player2.getMaxSpeed()) * 100.f;
        engineAudio.setVolume(clamp(lerp(engineAudio.getVolume(), (vol1 + vol2) / 2.f, 10.f * dt), 0.f, maxVol));
        ambientEngineAudio.setVolume(0.f);
    }
    else if (selectedMode == Gamemode::AI)
    {
        bool isSpectating = aiSpectatorMode || spectatorModeToggled;
        Car *focus = (isSpectating && spectatorTarget) ? spectatorTarget : &player1;

        engineAudio.setVolume(clamp(lerp(engineAudio.getVolume(), focus->getAccelerating() ? masterVol : 0.f, 10.f * dt), 0.f, maxVol));

        bool anyOtherAccelerating = false;
        for (Car *c : cars)
        {
            if (c == focus || !c->getActive())
                continue;
            if (c->getAccelerating())
            {
                anyOtherAccelerating = true;
                break;
            }
        }
        ambientEngineAudio.setVolume(clamp(lerp(ambientEngineAudio.getVolume(), anyOtherAccelerating ? masterVol * 0.5f : 0.f, 10.f * dt), 0.f, maxVol));
    }
}