#include "CollisionHandler.h"
#include <algorithm>
#include <cmath>

CollisionHandler::CollisionHandler(Track &track, std::vector<Car *> &cars) : track(track), cars(cars) {}

bool CollisionHandler::handleCollision(Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt)
{
    bool flag = false;

    // 1. WALL COLLISIONS (Handled sequentially first)
    int wallIndex = checkWallCollisions(car, pos, angle);
    if (wallIndex != -1)
    {
        impactCarryover result = handleCollisionResponse(wallIndex, car, pos, angle, oldPos, oldAngle, dt);
        car->setCurrSpeed(result.speed);
        car->setAngularVelocity(result.angularVelocity);

        pos = oldPos + result.pos;
        car->setPosition(pos);
        flag = true;
    }
    // 2. VEHICLE TO VEHICLE COLLISIONS
    for (auto other : cars)
    {
        if (other == car || !other->getActive() || other->isInvincible())
            continue;

        CarCollisionResult result = checkCarCollisions(car, other, pos, angle);
        if (result.hit)
        {
            applyPhysicsImpulse(car, pos, other, result);
            flag = true;
            pos = car->getPosition(); // Keep tracked position in sync
        }
    }

    return flag;
}

int CollisionHandler::checkWallCollisions(Car *car, sf::Vector2f pos, float angle) const
{
    auto corners = car->getCorners(pos, angle);

    bool frontHit = !track.isOnRoad(corners[0]) && !track.isOnRoad(corners[1]);
    bool rearHit = !track.isOnRoad(corners[2]) && !track.isOnRoad(corners[3]);
    bool leftHit = !track.isOnRoad(corners[0]) && !track.isOnRoad(corners[3]);  // Updated (was 0, 2)
    bool rightHit = !track.isOnRoad(corners[1]) && !track.isOnRoad(corners[2]); // Updated (was 1, 3)

    if (frontHit)
        return 4;
    if (rearHit)
        return 5;
    if (leftHit)
        return 6;
    if (rightHit)
        return 7;

    for (int i = 0; i < 4; i++)
        if (!track.isOnRoad(corners[i]))
            return i;

    return -1;
}

impactCarryover CollisionHandler::handleCollisionResponse(int index, Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt) const
{
    impactCarryover returnValue;
    if (index == -1)
        return returnValue;

    float speed = car->getCurrSpeed();
    float impact = std::abs(speed) * impactScale;
    float spinDirection = 0.f;
    sf::Vector2f pushDir = car->getPerpendicularVector();

    if (index == 2) // Rear-Right (was index 3)
    {
        spinDirection = rightSpin;
        pushDir *= -1.f;
    }
    else if (index == 3) // Rear-Left (was index 2)
    {
        spinDirection = leftSpin;
    }
    else if (index == 1) // Front-Right
    {
        spinDirection = leftSpin;
        pushDir *= -1.f;
    }
    else if (index == 0) // Front-Left
    {
        spinDirection = rightSpin;
    }
    else if (index == 4 || index == 5)
    {
        pushDir = car->getDirectionVector();
    }
    else if (index == 6)
    {
        pushDir *= -1.f;
    }

    if (checkWallCollisions(car, oldPos, oldAngle) == -1)
    {
        if (index <= 3 || index >= 6)
        {
            returnValue.pos = pushDir;
        }
    }
    else
    {
        {
            car->setAngle(oldAngle);
            car->setCurrSpeed(0.f);
        }
    }

    if (index >= 6 || index <= 3)
        speed *= sideCollisionDamping;
    else if (index == 4 || index == 5)
        speed *= headCollisionDamping;

    returnValue.dir = car->getDirectionVector();
    returnValue.speed = speed;
    float av = impact * spinDirection * angularDamping;
    returnValue.angularVelocity = av;
    float afterSpin = av * dt;
    if (checkWallCollisions(car, oldPos + returnValue.pos, car->getAngle() + afterSpin) != -1)
    {
        returnValue.angularVelocity = 0.f;
        returnValue.pos = pushDir * wallPushMultiplier;
        returnValue.speed = 0.f;
    }
    return returnValue;
}

void CollisionHandler::applyPhysicsImpulse(Car *car1, sf::Vector2f &pos1, Car *car2, const CarCollisionResult &result)
{
    sf::Vector2f p1 = pos1;
    sf::Vector2f p2 = car2->getPosition();

    sf::Vector2f delta = p1 - p2;
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (dist < minThreshold)
        return;

    sf::Vector2f normal(delta.x / dist, delta.y / dist);
    sf::Vector2f v1 = car1->getDirectionVector() * car1->getCurrSpeed();
    sf::Vector2f v2 = car2->getDirectionVector() * car2->getCurrSpeed();
    sf::Vector2f relativeVelocity = v1 - v2;

    float velocityAlongNormal = (relativeVelocity.x * normal.x) + (relativeVelocity.y * normal.y);

    float restitution = baseRestitution;
    if (std::abs(velocityAlongNormal) < 15.f)
    {
        restitution = 0.0f;
    }

    if (velocityAlongNormal < 0)
    {
        float j = -(1.0f + restitution) * velocityAlongNormal;
        j /= 2.0f; // assuming all cars have equal mass

        sf::Vector2f impulse(normal.x * j, normal.y * j);
        sf::Vector2f newV1 = v1 + impulse;
        sf::Vector2f newV2 = v2 - impulse;

        auto project1DVelocity = [](sf::Vector2f vel, sf::Vector2f dir)
        {
            return (vel.x * dir.x) + (vel.y * dir.y);
        };

        car1->setCurrSpeed(project1DVelocity(newV1, car1->getDirectionVector()));
        car2->setCurrSpeed(project1DVelocity(newV2, car2->getDirectionVector()));

        auto corners1 = car1->getCorners(pos1, car1->getAngle());
        sf::Vector2f contactPoint = corners1[result.car1Index];

        // 2. Calculate the lever arms (vectors from centers of mass to the contact point)
        sf::Vector2f r1 = contactPoint - pos1;
        sf::Vector2f r2 = contactPoint - car2->getPosition();

        // 3. Compute 2D cross product (r.x * J.y - r.y * J.x) to determine true directional torque
        // Note: Car 2 receives the negative impulse vector (-impulse)
        float torque1 = (r1.x * impulse.y) - (r1.y * impulse.x);
        float torque2 = (r2.x * -impulse.y) - (r2.y * -impulse.x);

        car1->setAngularVelocity(car1->getAngularVelocity() + torque1 * torqueTurnFactor);
        car2->setAngularVelocity(car2->getAngularVelocity() + torque2 * torqueTurnFactor);
    }

    // ================================================
    // OVERLAP RESOLUTION
    // ================================================
    resolveCarOverlap(car1, car2, pos1, result);
}
float CollisionHandler::distancePointToSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) const
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float lengthSquared = dx * dx + dy * dy;

    if (lengthSquared < minThreshold)
    {
        float distx = p.x - a.x;
        float disty = p.y - a.y;
        return distx * distx + disty * disty;
    }

    float t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / lengthSquared;
    t = std::max(0.f, std::min(1.f, t));

    sf::Vector2f closest(a.x + t * dx, a.y + t * dy);
    float distx = p.x - closest.x;
    float disty = p.y - closest.y;
    return distx * distx + disty * disty;
}

CarCollisionResult CollisionHandler::checkCarCollisions(Car *car1, Car *car2, sf::Vector2f pos, float angle) const
{
    CarCollisionResult result;
    auto corners1 = car1->getCorners(pos, angle);
    auto corners2 = car2->getCorners();

    sf::Vector2f axes[4];
    axes[0] = corners1[1] - corners1[0]; // Front edge vector
    axes[1] = corners1[3] - corners1[0]; // Left edge vector (Updated: was corners1[2])
    axes[2] = corners2[1] - corners2[0]; // Front edge vector
    axes[3] = corners2[3] - corners2[0]; // Left edge vector (Updated: was corners2[2])

    float minOverlap = std::numeric_limits<float>::max();
    sf::Vector2f translationAxis;

    for (int i = 0; i < 4; i++)
    {
        float len = std::sqrt(axes[i].x * axes[i].x + axes[i].y * axes[i].y);
        if (len < degenAxisThreshold)
            continue;
        sf::Vector2f axis = axes[i] / len;

        float min1 = dotProduct(corners1[0], axis), max1 = min1;
        for (int j = 1; j < 4; j++)
        {
            float proj = dotProduct(corners1[j], axis);
            min1 = std::min(min1, proj);
            max1 = std::max(max1, proj);
        }

        float min2 = dotProduct(corners2[0], axis), max2 = min2;
        for (int j = 1; j < 4; j++)
        {
            float proj = dotProduct(corners2[j], axis);
            min2 = std::min(min2, proj);
            max2 = std::max(max2, proj);
        }

        float overlap = std::min(max1, max2) - std::max(min1, min2);
        if (overlap <= 0.f)
        {
            result.hit = false;
            return result;
        }
        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            translationAxis = axis;
        }
    }

    // contact point / index, purely for the impulse — unchanged from before
    int hitCornerIndex = 0;
    float closestDist = std::numeric_limits<float>::max();

    // Updated edge pairings: {FL, FR}, {RR, RL}, {FL, RL}, {FR, RR}
    std::vector<std::pair<int, int>> edges = {{0, 1}, {2, 3}, {0, 3}, {1, 2}};

    for (int i = 0; i < 4; i++)
    {
        for (auto &edge : edges)
        {
            float dist = distancePointToSegment(corners1[i], corners2[edge.first], corners2[edge.second]);
            if (dist < closestDist)
            {
                closestDist = dist;
                hitCornerIndex = i;
                if (edge == std::pair<int, int>{0, 1})
                    result.car2Index = 4; // Front
                else if (edge == std::pair<int, int>{2, 3})
                    result.car2Index = 5; // Rear
                else if (edge == std::pair<int, int>{0, 3})
                    result.car2Index = 6; // Left (Updated: was {0, 2})
                else if (edge == std::pair<int, int>{1, 2})
                    result.car2Index = 7; // Right (Updated: was {1, 3})
            }
        }
    }

    result.hit = true;
    result.car1Index = hitCornerIndex;
    result.minOverlap = minOverlap;
    result.translationAxis = translationAxis;
    return result;
}

void CollisionHandler::resolveCarOverlap(Car *car1, Car *car2, sf::Vector2f &pos1, const CarCollisionResult &result)
{
    sf::Vector2f translationAxis = result.translationAxis;

    sf::Vector2f centerToCenter = pos1 - car2->getPosition();
    float directionCheck = dotProduct(centerToCenter, translationAxis);
    if (directionCheck < 0.f)
        translationAxis = -translationAxis;

    sf::Vector2f separation = translationAxis * (result.minOverlap * overlapPushFactor);

    pos1 += separation;
    car1->setPosition(pos1);
    car2->setPosition(car2->getPosition() - separation);
}