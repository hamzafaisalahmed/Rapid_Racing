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

class Game
{

    Leaderboard leaderboardManager;
    std::stack<GameState> stateStack;
    sf::RenderWindow window;
    sf::View gameView;
    Track track;

    Player player1;
    Player player2;
    const int aiCount = 7;
    sf::View hudView;

    float totalRaceTime;
    int currentLap;
    int totalLaps;
    float currentLapTime;
    LapTime lapData;
    std::vector<Waypoint> waypoints;

    std::vector<Car *> cars;

    sf::Music engineAudio;
    sf::Music endscreen;

    std::unique_ptr<Graphics> graphics;
    std::unique_ptr<CollisionHandler> collisionHandler;

    int selectedLaps;
    int selectedCarLvl;
    bool audioMuted;

    Gamemode selectedMode;
    std::vector<CarPreset> carPresets{CarPreset(300.f, 50.f), CarPreset(400.f, 100.f), CarPreset(500.f, 200.f)};
    std::vector<int> maxLaps{1, 2, 3, 5};

    int winner;

public:
    void init();
    void run();

    void update(float dt);
    void render();
    void handleEvents();
    void handlePlayerMovement(float dt);
    void resetLevel();
    void checkWinner(Car *player, int playerNo);
    void updateRacePositions();
    bool carPosition(const Car &a, const Car &b);
    ~Game()
    {
        for (int i = 2; i < aiCount; ++i)
        {
            delete cars[i];
        }
        cars.clear();
    }
};