#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <stack>
#include <vector>
#include <queue>
#include <string>

#include "GameState.h"
#include "Track.h"
class Game
{
    float dt;
    int score;

    std::stack<GameState> stateStack;
    sf::RenderWindow window;
    sf::View gameView;
    Track track;

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