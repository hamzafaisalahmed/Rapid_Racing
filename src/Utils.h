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