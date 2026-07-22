#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <stack>
#include <vector>
#include <queue>
#include <string>
#include <memory>
#include "Track.h"
#include "Car.h"
#include "Utils.h"
#include "Graphics.h"
#include "CollisionHandler.h"
#include "Leaderboard.h"
#include "StateManager.h"
#include "WaypointHandler.h"
class Game
{

    Leaderboard leaderboardManager;
    std::unique_ptr<StateManager> stateManager;
    sf::RenderWindow window;
    sf::View gameView;
    Track track;

    Player player1;
    Player player2;
    int selectedAICount = 7;
    const int maxAICount = 7;
    sf::View hudView;

    float totalRaceTime;
    int totalLaps;
    float bestLap;
    Car *bestLapHolder;
    std::vector<Waypoint> waypoints;

    std::vector<Car *> cars;

    sf::Music engineAudio;
    sf::Music endscreen;
    sf::Music homeAudio;

    std::unique_ptr<Graphics> graphics;
    std::unique_ptr<CollisionHandler> collisionHandler;

    int selectedLaps;
    int selectedCarLvl;

    Gamemode selectedMode;
    std::vector<CarPreset> carPresets{CarPreset(300.f, 50.f), CarPreset(400.f, 100.f), CarPreset(500.f, 200.f)};
    std::vector<int> maxLaps{1, 2, 3, 5};

    std::vector<Car *> raceLeaderboard;

    bool countdownMode;
    float countdownTime;

    WaypointHandler wpHandler;

    int finishedCount = 0;
    int totalRacers = 0;

    bool aiSpectatorMode = false;
    bool spectatorModeToggled = false;
    Car *spectatorTarget = nullptr;
    Car *findSpectatorTarget(Car *curr);

public:
    void init();
    void run();

    void update(float dt);
    void render();
    void handleEvents();
    void handlePlayerMovement(float dt);
    void resetLevel();
    void checkWinner(Car *car);
    void updateBestLap();
    void updateRacePositions();
    bool carPosition(const Car &a, const Car &b);
    ~Game()
    {
        for (size_t i = 2; i < cars.size(); ++i)
        {
            delete cars[i];
        }
        cars.clear();
    }
};