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
    friend CarCollisionResult callCheckCarCollisions(const CollisionHandler &,
                                                     Car *car1, Car *car2,
                                                     sf::Vector2f pos, float angle);

private:
    Track &track;
    std::vector<Car *> &cars;

    CarCollisionResult checkCarCollisions(Car *car1, Car *car2, sf::Vector2f pos, float angle) const;
    int checkWallCollisions(Car *car, sf::Vector2f pos, float angle) const;
    impactCarryover handleCollisionResponse(int index, Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt) const;
    void applyPhysicsImpulse(Car *car1, sf::Vector2f &pos1, Car *car2, const CarCollisionResult &result);
    float distancePointToSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) const;
    void resolveCarOverlap(Car *car1, Car *car2, sf::Vector2f &pos1, const CarCollisionResult &result);

    float degenAxisThreshold = 0.001f;
    float overlapPushFactor = 0.51f;
    float touchThreshold = 5.f;
    float sideCollisionDamping = 0.3f;
    float headCollisionDamping = -0.7f;
    float impactScale = 1.1f;
    float wallPushMultiplier = 2.f;
    float angularDamping = 0.65f;
    float baseRestitution = 0.35f;
    float torqueTurnFactor = 0.2f;
    float minThreshold = 0.0001f;

    float rightSpin = 1.f;
    float leftSpin = -1.f;
};