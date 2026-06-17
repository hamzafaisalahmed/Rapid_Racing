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

class Game
{
    float dt;
    int score;

    std::stack<GameState> stateStack;
    sf::RenderWindow window;
    sf::View gameView;
    Track track;

    Player player1;
    Player player2;
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
    int checkWallCollisions(Car *car, sf::Vector2f pos, float angle);
    impactCarryover handleCollisionResponse(int index, Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt);
    CarCollisionResult checkCarCollisions(Car *car1, Car *car2, sf::Vector2f pos, float angle);

    bool CollisionHandler(Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt);

    void update(float dt);
    void render();
    void handleEvents();
    void handlePlayerMovement(float dt);
    void resetLevel();
    void checkWinner(Car *player, int playerNo);
    void saveLapTime(const LapTime &lt);
    std::vector<LapTime> loadLapTimes();
    void updateRacePositions();
    bool carPosition(const Car &a, const Car &b);
    float distancePointToSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b);
    Game() : dt(0.0f), score(0) {}
    ~Game() = default;
};