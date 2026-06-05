#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <stack>
#include <vector>
#include <queue>
#include <string>

#include "Track.h"
#include "Car.h"
#include "Utils.h"

enum class GameState
{
    Home,
    Menu,
    Playing,
    Paused,
    LevelComplete,
    GameOver
};

class Game
{
    float dt;
    int score;

    std::stack<GameState> stateStack;
    sf::RenderWindow window;
    sf::View gameView;
    Track track;

    Player player;

    const float standardTurnFactor = 500.f;

    sf::Font font;
    sf::Text timerText;
    sf::Text lapText;
    sf::Text speedometer;
    sf::View hudView;

    sf::Clock raceTimer;
    int currentLap;
    int totalLaps;
    float currentLapTime;

public:
    void init();
    void run();
    bool checkCollisions(sf::Vector2f pos, float angle);
    void update(float dt);
    void render();
    void handleEvents();
    void handlePlayerMovement(float dt);

    void renderHUD();
    Game() : dt(0.0f), score(0) {}
    ~Game() = default;
};