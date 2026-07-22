#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>

const float BESTLAP_INIT_VAL = 999999;
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
    std::string title;
    float bestLap;
    float currentLapTime;

    LapTime() : bestLap(BESTLAP_INIT_VAL), currentLapTime(0) {}
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
    float minOverlap = 0.f;
    sf::Vector2f translationAxis;
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

// Vector math helpers
inline float dotProduct(const sf::Vector2f &a, const sf::Vector2f &b)
{
    return (a.x * b.x) + (a.y * b.y);
}

inline float crossProduct(const sf::Vector2f &a, const sf::Vector2f &b)
{
    return (a.x * b.y) - (a.y * b.x);
}

inline sf::Vector2f normalize(const sf::Vector2f &v)
{
    float len = std::sqrt((v.x * v.x) + (v.y * v.y));
    if (len < 0.0001f)
        return v; // Avoid division by zero
    return sf::Vector2f(v.x / len, v.y / len);
}

inline float magnitude(const sf::Vector2f &v)
{
    return std::sqrt((v.x * v.x) + (v.y * v.y));
}

enum class TargetSide : int
{
    Left = 0,
    Mid = 1,
    Right = 2
};