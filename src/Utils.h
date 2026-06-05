#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
