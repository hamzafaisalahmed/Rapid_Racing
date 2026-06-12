#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

inline float clamp(float a, float min, float max)
{
    if (a > max)
        return max;
    if (a < min)
        return min;
    return a;
}

struct LapTime
{
    int id;
    int laps;
    float bestLap;
    float totalTime;

    LapTime() : id((int)std::time(0)), laps(0), bestLap(999999), totalTime(0) {}
};

enum class GameState
{
    Home,
    Menu,
    TimeTrial,
    Paused,
    LevelComplete,
    GameOver,
    PVP
};

enum class carInput
{
    None,
    Up,
    Down,
    Left,
    Right
};

struct Waypoint
{
    sf::Vector2f left;
    sf::Vector2f right;
    sf::Vector2f mid;
    Waypoint(sf::Vector2f l, sf::Vector2f r) : left(l), right(r), mid((l.x + r.x) / 2, (l.y + r.y) / 2) {}
};

inline float distance(sf::Vector2f a, sf::Vector2f b)
{
    return (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
}