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
    int activePlayerCount = 0;
    if (selectedMode == Gamemode::TimeTrial)
    {
        activePlayerCount = 1;
    }
    else if (selectedMode == Gamemode::PVP)
    {
        activePlayerCount = 2;
    }
    else if (selectedMode == Gamemode::AI)
    {
        activePlayerCount = 2; // player1 + aiCar (or however many)
    }

    // Set active status
    for (size_t i = 0; i < cars.size(); ++i)
    {
        cars[i]->setActive(i < (size_t)activePlayerCount);
    }
}

void Game::init()
{
    std::srand((unsigned)std::time(0));

    window.create(sf::VideoMode(1200, 800), "Rapid Racing", sf::Style::Default);
    window.setFramerateLimit(60);

    track.LoadTrack("assets/textures/track1.png");
    player1.load("assets/textures/car1.png");
    player2.load("assets/textures/car2.png");
    cars.push_back(&player1);
    cars.push_back(&player2);

    waypoints = track.getWaypoints();
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
    player1.setCollisionFunction([this](Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt)
                                 { return this->CollisionHandler(car, pos, angle, oldPos, oldAngle, dt); });
    player2.setCollisionFunction([this](Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt)
                                 { return this->CollisionHandler(car, pos, angle, oldPos, oldAngle, dt); });
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
                    if (selectedMode == Gamemode::TimeTrial)
                    {
                        saveLapTime(lapData);
                        lapData = LapTime();
                    }
                    resetLevel();
                    stateStack.pop();
                }
                else if (levelCompleteButtons[1].contains(mousePos))
                {
                    if (selectedMode == Gamemode::TimeTrial)
                        saveLapTime(lapData);
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
                        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
                            std::cout << "YAY";
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
        if (selectedMode == Gamemode::AI || selectedMode == Gamemode::TimeTrial)
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
                    stateStack.push(GameState::LevelComplete);
                }
            }
        }
        else if (selectedMode == Gamemode::PVP)
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
    }
}

// CarCollisionResult Game::checkCarCollisions(Car *car1, Car *car2, sf::Vector2f pos, float angle)
// {
//     CarCollisionResult result;
//     auto corners1 = car1->getCorners(pos, angle);
//     auto corners2 = car2->getCorners();

//     std::vector<int> hitCar1;
//     std::vector<int> hitCar2;
//     float touchThreshold = 2.f;
//     for (int i = 0; i < 4; i++)
//     {
//         for (int j = 0; j < 4; j++)
//         {
//             float sqDist = distance(corners1[i], corners2[j]);
//             if (sqDist < (touchThreshold * touchThreshold))
//             {
//                 hitCar1.push_back(i);
//                 hitCar2.push_back(j);
//             }
//         }
//     }
//     if (hitCar1.empty())
//     {
//         result.hit = false;
//         return result;
//     }
//     result.hit = true;
//     auto getCollisionIndex = [](const std::vector<int> &corners) -> int
//     {
//         if (corners.size() == 1)
//         {
//             return corners[0]; // Single corner: 0,1,2,3
//         }
//         else if (corners.size() == 2)
//         {
//             // Two corners = a side
//             int a = corners[0];
//             int b = corners[1];

//             if ((a == 0 && b == 1) || (a == 1 && b == 0))
//                 return 4; // Front
//             if ((a == 2 && b == 3) || (a == 3 && b == 2))
//                 return 5; // Rear
//             if ((a == 0 && b == 2) || (a == 2 && b == 0))
//                 return 6; // Left
//             if ((a == 1 && b == 3) || (a == 3 && b == 1))
//                 return 7; // Right
//         }
//         return -1;
//     };

//     result.car1Index = getCollisionIndex(hitCar1);
//     result.car2Index = getCollisionIndex(hitCar2);

//     return result;
// }
CarCollisionResult Game::checkCarCollisions(Car *car1, Car *car2, sf::Vector2f pos, float angle)
{
    CarCollisionResult result;
    auto corners1 = car1->getCorners(pos, angle);
    auto corners2 = car2->getCorners();

    float touchThreshold = 5.f;
    int hitCornerIndex = -1;
    float closestDist = touchThreshold * touchThreshold;

    // Check each corner of car1 against each edge of car2
    for (int i = 0; i < 4; i++)
    {
        // Define edges: 0-1 (front), 2-3 (rear), 0-2 (left), 1-3 (right)
        std::vector<std::pair<int, int>> edges = {{0, 1}, {2, 3}, {0, 2}, {1, 3}};

        for (auto &edge : edges)
        {
            sf::Vector2f p1 = corners2[edge.first];
            sf::Vector2f p2 = corners2[edge.second];

            // Distance from corner to line segment
            float dist = distancePointToSegment(corners1[i], p1, p2);

            if (dist < closestDist)
            {
                closestDist = dist;
                hitCornerIndex = i;
                cout << "Car: " << car2->getCurrSpeed() << endl;
                // Determine collision side based on which edge
                if (edge == std::pair<int, int>{0, 1})
                {
                    result.car2Index = 5; // rear
                    cout << "rear\n";
                }
                else if (edge == std::pair<int, int>{2, 3})
                {
                    result.car2Index = 4; // front
                    cout << "front\n";
                }
                else if (edge == std::pair<int, int>{0, 2})
                {
                    result.car2Index = 6;
                    cout << "Left\n";
                }
                // Left
                else if (edge == std::pair<int, int>{1, 3})
                {
                    result.car2Index = 7; // Right
                    cout << "Right\n";
                }
            }
        }
    }

    if (hitCornerIndex == -1)
    {
        result.hit = false;
        return result;
    }

    result.hit = true;
    result.car1Index = hitCornerIndex;
    return result;
}
float Game::distancePointToSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / (dx * dx + dy * dy);
    t = std::max(0.f, std::min(1.f, t));

    sf::Vector2f closest(a.x + t * dx, a.y + t * dy);
    float distx = p.x - closest.x;
    float disty = p.y - closest.y;
    return distx * distx + disty * disty;
}
int Game::checkWallCollisions(Car *car, sf::Vector2f pos, float angle)
{
    auto corners = car->getCorners(pos, angle);
    // corners: 0=front-left, 1=front-right, 2=rear-left, 3=rear-right

    bool rearHit = !track.isOnRoad(corners[0]) && !track.isOnRoad(corners[1]);
    bool frontHit = !track.isOnRoad(corners[2]) && !track.isOnRoad(corners[3]);
    bool leftHit = !track.isOnRoad(corners[0]) && !track.isOnRoad(corners[2]);
    bool rightHit = !track.isOnRoad(corners[1]) && !track.isOnRoad(corners[3]);

    if (frontHit)
        return 4; // Front
    if (rearHit)
        return 5; // Rear
    if (leftHit)
        return 6; // Left
    if (rightHit)
        return 7; // Right

    // Single corner hit
    for (int i = 0; i < 4; i++)
        if (!track.isOnRoad(corners[i]))
            return i;

    return -1;
}

impactCarryover Game::handleCollisionResponse(int index, Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt)
{
    impactCarryover returnValue;
    if (index == -1)
        return returnValue;

    float speed = car->getCurrSpeed();
    float impact = std::abs(speed);
    // Apply directional spin based on which side/corner hit
    float spinDirection = 0.f; // Clockwise by default
    sf::Vector2f pushDir = car->getPerpendicularVector();
    if (index == 2)
        spinDirection = 1.0f;
    if (index == 3) // Right side hits
    {
        spinDirection = -1.0f;
        pushDir *= -1.f;
    }
    if (index == 1)
    {
        spinDirection = 1.0f;
        pushDir *= -1.f;
    }
    if (index == 0) // Right side hits
    {
        spinDirection = -1.0f;
    }

    if (index == 4 || index == 5)
        pushDir = car->getDirectionVector();
    if (index == 6)
        pushDir *= -1.f;
    if (checkWallCollisions(car, oldPos, oldAngle) == -1)
    {
        if (index <= 3)
        {
            returnValue.pos = (pushDir * 2.f);
        }
        else if (index >= 6)
            returnValue.pos = (pushDir * 2.f);
    }

    else
    {
        bool flag = false;
        for (int i = 0; i < 5; i++)
        {
            sf::Vector2f test = oldPos + ((i + 1) / 5.f) * (pos - oldPos);
            float testAngle = oldAngle + ((i + 1) / 5.f) * (angle - oldAngle);
            if (checkWallCollisions(car, test, testAngle) == -1)
            {
                returnValue.pos = ((i + 1) / 5.f) * (pos - oldPos);
                car->setAngle(testAngle);
                flag = true;
                break;
            }
        }
        if (!flag)
        {
            car->setAngle(oldAngle);
            car->setCurrSpeed(0.f);
        }
    }
    if (index >= 6)
        speed *= 0.5f;
    else if (index <= 3)
        speed *= 0.2f;
    else
        speed *= -0.2f;

    returnValue.speed = speed;
    float av = impact * spinDirection;
    returnValue.angularVelocity = av;
    float afterSpin = av * dt;
    if (checkWallCollisions(car, oldPos + returnValue.pos, car->getAngle() + afterSpin) != -1)
    {
        returnValue.angularVelocity = 0.f;
        returnValue.pos = pushDir * 2.f;
        returnValue.speed = 0.f;
    }
    return returnValue;
}

bool Game::CollisionHandler(Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt)
{
    std::vector<float> preCol{car->getCurrSpeed(), car->getAngle(), car->getAngularVelocity()};
    bool flag = false;
    int index = checkWallCollisions(car, pos, angle);
    if (index != -1)
    {
        impactCarryover result = handleCollisionResponse(index, car, pos, angle, oldPos, oldAngle, dt);
        car->setCurrSpeed(result.speed);
        car->setAngle(result.angle);
        car->setAngularVelocity(result.angularVelocity);
        car->setPosition(oldPos + result.pos);
        flag = true;
    }

    for (auto other : cars)
    {
        if (other == car || !other->getActive() || other->isInvincible())
            continue;
        CarCollisionResult result = checkCarCollisions(car, other, pos, angle);
        if (result.hit)
        {
            flag = true;
            impactCarryover collResult = handleCollisionResponse(result.car1Index, car, pos, angle, oldPos, oldAngle, dt);
            car->setCurrSpeed(collResult.speed);
            car->setAngle(collResult.angle);
            car->setAngularVelocity(collResult.angularVelocity);
            car->setPosition(oldPos + collResult.pos);

            collResult.speed *= -0.5;
            collResult.angularVelocity *= -0.7;
            collResult.index = result.car2Index;
            // collResult.pos += pos;
            other->setCollisionCarryover(collResult);
            other->setPosition(other->getPosition() - collResult.pos);
        }
    }
    return flag;
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
    else if (stateStack.top() == GameState::Playing)
    {
        if (selectedMode == Gamemode::TimeTrial)
        {
            graphics->renderGamePlay();
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
            graphics->renderLevelComplete(lapData, loadLapTimes());
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
