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

    sf::Music engineAudio;
    sf::Music endscreen;

    std::unique_ptr<Graphics> graphics;

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

    Game() : dt(0.0f), score(0) {}
    ~Game() = default;
};