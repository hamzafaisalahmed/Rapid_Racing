#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Car.h"
#include "Track.h"
#include "Utils.h"

class CollisionHandler
{
public:
    CollisionHandler(Track &track, std::vector<Car *> &cars);

    bool handleCollision(Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt);

private:
    Track &track;
    std::vector<Car *> &cars;

    CarCollisionResult checkCarCollisions(Car *car1, Car *car2, sf::Vector2f pos, float angle) const;
    int checkWallCollisions(Car *car, sf::Vector2f pos, float angle) const;
    impactCarryover handleCollisionResponse(int index, Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt) const;
    void applyPhysicsImpulse(Car *car1, sf::Vector2f &pos1, Car *car2, const CarCollisionResult &result);
    float distancePointToSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) const;
};