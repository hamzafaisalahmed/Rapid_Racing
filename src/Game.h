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

public:
    void init();
    void run();
    void checkCollisions();
    void update(float dt);
    void render();
    void handleEvents();

    Game() : dt(0.0f), score(0) {}
    ~Game() = default;
};