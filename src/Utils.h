#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>

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