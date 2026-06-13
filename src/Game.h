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

    Player player;
    sf::View hudView;

    float totalRaceTime;
    int currentLap;
    int totalLaps;
    float currentLapTime;
    LapTime lapData;
    std::vector<Waypoint> waypoints;

    std::vector<Player> PlayerCars;
    std::vector<AI> AICars;

    sf::Music engineAudio;
    sf::Music endscreen;

    std::unique_ptr<Graphics> graphics;

    int selectedLaps;
    int selectedCarLvl;
    bool audioMuted;

    std::vector<CarPreset> carPresets{CarPreset(300.f, 50.f), CarPreset(400.f, 100.f), CarPreset(500.f, 200.f)};
    std::vector<int> maxLaps{1, 2, 3, 5};

public:
    void init();
    void run();
    bool checkCollisions(sf::Vector2f pos, float angle);
    void update(float dt);
    void render();
    void handleEvents();
    void handlePlayerMovement(float dt);
    void resetLevel();

    void saveLapTime(const LapTime &lt);
    std::vector<LapTime> loadLapTimes();

    bool carPosition(const Car &a, const Car &b);
    Game() : dt(0.0f), score(0) {}
    ~Game() = default;
};