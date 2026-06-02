#include <iostream>
#include <SFML/Graphics.hpp>

inline float clamp(float val, float lo, float hi)
{
    if (val > hi)
        return hi;
    if (val < lo)
        return lo;
    return val;
}