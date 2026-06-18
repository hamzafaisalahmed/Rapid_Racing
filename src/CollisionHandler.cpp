#include "CollisionHandler.h"
#include <algorithm>
#include <cmath>

// Explicit structure setup assuming your collision detection passes down an info/result object
// struct CollisionResult { int car1Index; int car2Index; };

void resolveCarOverlap(Car *car1, Car *car2, sf::Vector2f &pos1)
{
    auto corners1 = car1->getCorners(pos1, car1->getAngle());
    auto corners2 = car2->getCorners(car2->getPosition(), car2->getAngle());

    // 1. Define the 4 unique axes of the two rectangular cars (2 from each car)
    // We get these by taking two perpendicular edges from each bounding box
    sf::Vector2f axes[4];
    axes[0] = corners1[1] - corners1[0]; // Car 1 Width Axis
    axes[1] = corners1[2] - corners1[0]; // Car 1 Length Axis
    axes[2] = corners2[1] - corners2[0]; // Car 2 Width Axis
    axes[3] = corners2[2] - corners2[0]; // Car 2 Length Axis

    float minOverlap = std::numeric_limits<float>::max();
    sf::Vector2f translationAxis;

    // 2. Test the projection of both cars along all 4 axes
    for (int i = 0; i < 4; i++)
    {
        // Normalize the axis
        float len = std::sqrt(axes[i].x * axes[i].x + axes[i].y * axes[i].y);
        if (len < 0.001f)
            continue;
        sf::Vector2f axis = axes[i] / len;

        // Project all 4 corners of Car 1 onto this axis to find its bounds
        float min1 = (corners1[0].x * axis.x) + (corners1[0].y * axis.y);
        float max1 = min1;
        for (int j = 1; j < 4; j++)
        {
            float proj = (corners1[j].x * axis.x) + (corners1[j].y * axis.y);
            min1 = std::min(min1, proj);
            max1 = std::max(max1, proj);
        }

        // Project all 4 corners of Car 2 onto this axis
        float min2 = (corners2[0].x * axis.x) + (corners2[0].y * axis.y);
        float max2 = min2;
        for (int j = 1; j < 4; j++)
        {
            float proj = (corners2[j].x * axis.x) + (corners2[j].y * axis.y);
            min2 = std::min(min2, proj);
            max2 = std::max(max2, proj);
        }

        // Calculate how much the two projections overlap
        float overlap = std::min(max1, max2) - std::max(min1, min2);

        // If there is no overlap on even ONE axis, they are not colliding at all
        if (overlap <= 0.f)
            return;

        // Track the axis with the absolute smallest overlap (the path of least resistance)
        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            translationAxis = axis;
        }
    }

    // 3. Ensure our final displacement vector points from Car 2 toward Car 1
    sf::Vector2f centerToCenter = pos1 - car2->getPosition();
    float directionCheck = (centerToCenter.x * translationAxis.x) + (centerToCenter.y * translationAxis.y);
    if (directionCheck < 0.f)
    {
        translationAxis = -translationAxis; // Flip it to point outward
    }

    // 4. Push them apart symmetrically along the clean translation axis
    sf::Vector2f separation = translationAxis * (minOverlap * 0.51f);

    pos1 += separation;
    car1->setPosition(pos1);
    car2->setPosition(car2->getPosition() - separation);
}

CollisionHandler::CollisionHandler(Track &track, std::vector<Car *> &cars)
    : track(track), cars(cars)
{
}

bool CollisionHandler::handleCollision(Car *car, sf::Vector2f pos, float angle, sf::Vector2f oldPos, float oldAngle, float dt)
{
    bool flag = false;

    // 1. WALL COLLISIONS (Handled sequentially first)
    int wallIndex = checkWallCollisions(car, pos, angle);
    if (wallIndex != -1)
    {
        impactCarryover result = handleCollisionResponse(wallIndex, car, pos, angle, oldPos, oldAngle, dt);
        car->setCurrSpeed(result.speed);
        car->setAngle(result.angle);
        car->setAngularVelocity(result.angularVelocity);

        pos = oldPos + result.pos;
        car->setPosition(pos);
        flag = true;
    }

    // Sort cars to process trailing vehicles first (natural chain reaction logic)
    std::sort(cars.begin(), cars.end(), [](Car *a, Car *b)
              { return a->getRacePos() > b->getRacePos(); });

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

CarCollisionResult CollisionHandler::checkCarCollisions(Car *car1, Car *car2, sf::Vector2f pos, float angle) const
{
    CarCollisionResult result;
    auto corners1 = car1->getCorners(pos, angle);
    auto corners2 = car2->getCorners();

    float touchThreshold = 5.f; // Slightly generous threshold to capture close overlaps
    int hitCornerIndex = -1;
    float closestDist = touchThreshold * touchThreshold;

    for (int i = 0; i < 4; i++)
    {
        std::vector<std::pair<int, int>> edges = {{0, 1}, {2, 3}, {0, 2}, {1, 3}};

        for (auto &edge : edges)
        {
            sf::Vector2f p1 = corners2[edge.first];
            sf::Vector2f p2 = corners2[edge.second];
            float dist = distancePointToSegment(corners1[i], p1, p2);

            if (dist < closestDist)
            {
                closestDist = dist;
                hitCornerIndex = i;
                if (edge == std::pair<int, int>{0, 1})
                {
                    result.car2Index = 4; // Front
                }
                else if (edge == std::pair<int, int>{2, 3})
                {
                    result.car2Index = 5; // Rear
                }
                else if (edge == std::pair<int, int>{0, 2})
                {
                    result.car2Index = 6; // Left Side
                }
                else if (edge == std::pair<int, int>{1, 3})
                {
                    result.car2Index = 7; // Right Side
                }
            }
        }
    }

    if (hitCornerIndex == -1)
    {
        result.hit = false;
        return result;
    }

    result.hit = true;
    result.car1Index = hitCornerIndex;
    return result;
}

int CollisionHandler::checkWallCollisions(Car *car, sf::Vector2f pos, float angle) const
{
    auto corners = car->getCorners(pos, angle);

    bool frontHit = !track.isOnRoad(corners[0]) && !track.isOnRoad(corners[1]);
    bool rearHit = !track.isOnRoad(corners[2]) && !track.isOnRoad(corners[3]);
    bool leftHit = !track.isOnRoad(corners[0]) && !track.isOnRoad(corners[2]);
    bool rightHit = !track.isOnRoad(corners[1]) && !track.isOnRoad(corners[3]);

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
    float impact = std::abs(speed) * 1.1f;
    float spinDirection = 0.f;
    sf::Vector2f pushDir = car->getPerpendicularVector();

    if (index == 3)
    {
        spinDirection = 1.0f;
        pushDir *= -1.f;
    }
    else if (index == 2)
    {
        spinDirection = -1.0f;
    }
    else if (index == 1)
    {
        spinDirection = -1.0f;
        pushDir *= -1.f;
    }
    else if (index == 0)
    {
        spinDirection = 1.0f;
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
        bool flag = false;
        for (int i = 0; i < 5; i++)
        {
            sf::Vector2f test = oldPos + ((i + 1) / 5.f) * (pos - oldPos);
            float testAngle = oldAngle + ((i + 1) / 5.f) * (angle - oldAngle);
            if (checkWallCollisions(car, test, testAngle) == -1)
            {
                returnValue.pos = ((i + 1) / 5.f) * (pos - oldPos);
                car->setAngle(testAngle);
                flag = true;
                break;
            }
        }
        if (!flag)
        {
            car->setAngle(oldAngle);
            car->setCurrSpeed(0.f);
        }
    }

    if (index >= 6 || index <= 3)
        speed *= 0.5f;
    else if (index == 4 || index == 5)
        speed *= -0.7f;

    returnValue.dir = car->getDirectionVector();
    returnValue.speed = speed;
    float av = impact * spinDirection;
    returnValue.angularVelocity = av;
    float afterSpin = av * dt;
    if (checkWallCollisions(car, oldPos + returnValue.pos, car->getAngle() + afterSpin) != -1)
    {
        returnValue.angularVelocity = 0.f;
        returnValue.pos = pushDir * 2.f;
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
    if (dist < 0.0001f)
        return;

    sf::Vector2f normal(delta.x / dist, delta.y / dist);
    sf::Vector2f v1 = car1->getDirectionVector() * car1->getCurrSpeed();
    sf::Vector2f v2 = car2->getDirectionVector() * car2->getCurrSpeed();
    sf::Vector2f relativeVelocity = v1 - v2;

    float velocityAlongNormal = (relativeVelocity.x * normal.x) + (relativeVelocity.y * normal.y);

    float restitution = 0.35f;
    if (std::abs(velocityAlongNormal) < 15.f)
    {
        restitution = 0.0f;
    }

    if (velocityAlongNormal < 0)
    {
        float j = -(1.0f + restitution) * velocityAlongNormal;
        j /= 2.0f;

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

        // 4. Commit angular velocities scaled by a tuning factor (0.1f balances out high velocities)
        float angularInertiaScalar = 0.5f;
        car1->setAngularVelocity(car1->getAngularVelocity() + torque1 * angularInertiaScalar);
        car2->setAngularVelocity(car2->getAngularVelocity() + torque2 * angularInertiaScalar);
    }

    // ================================================
    // OVERLAP RESOLUTION
    // ================================================
    resolveCarOverlap(car1, car2, pos1);
}
float CollisionHandler::distancePointToSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) const
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float lengthSquared = dx * dx + dy * dy;

    if (lengthSquared < 0.0001f)
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