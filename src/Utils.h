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
    Playing,
    Paused,
    LevelComplete,
    GameOver,
    Settings
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

struct CarPreset
{
    float maxSpeed;
    float AccRate;
    CarPreset(float m, float a) : maxSpeed(m), AccRate(a) {}
};

enum class Gamemode
{
    TimeTrial,
    PVP,
    AI,
    None
};

inline std::string formatRaceTime(float secondsTotal)
{
    int minutes = (int)(secondsTotal / 60.f);
    int seconds = (int)(secondsTotal) % 60;
    int millis = (int)((secondsTotal - (int)secondsTotal) * 100);

    // Always formats uniformly as MM:SS.mm
    return std::to_string(minutes) + ":" +
           (seconds < 10 ? "0" : "") + std::to_string(seconds) + "." +
           (millis < 10 ? "0" : "") + std::to_string(millis);
}

struct CarCollisionResult
{
    bool hit = false;
    int car1Index = -1; // Which side of car1 hit
    int car2Index = -1; // Which side of car2 hit
};

struct impactCarryover
{
    sf::Vector2f pos;
    sf::Vector2f dir;
    float speed = 0;
    float angle = 0;
    float angularVelocity = 0;
    int index = -1;
};