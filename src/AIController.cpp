#include "AIController.h"
#include "Utils.h"
#include <vector>
using namespace std;

void AIController::calculateVerticalInput(const std::vector<Car *> &cars)
{
    int currIdx = car->getCurrWaypointIndex();

    // Bounds checking
    if (waypoints.empty())
    {
        verticalInput = carInput::Up;
        return;
    }

    int nextIdx = (currIdx + 1) % waypoints.size();
    int nextNextIdx = (currIdx + 2) % waypoints.size();

    // Sequential gate vectors
    sf::Vector2f carPos = car->getPosition();
    sf::Vector2f currGate = waypoints[currIdx].mid;
    sf::Vector2f nextGate = waypoints[nextIdx].mid;
    sf::Vector2f nextNextGate = waypoints[nextNextIdx].mid;

    // V1: Current path (current gate to next gate)
    sf::Vector2f v1 = normalize(nextGate - currGate);

    // V2: Next path (next gate to gate after)
    sf::Vector2f v2 = normalize(nextNextGate - nextGate);

    // Curvature sensor
    float curvature = dotProduct(v1, v2);

    // Distance to next gate
    float distToNextGate = magnitude(nextGate - carPos);

    // Physics braking distance
    float currentSpeed = std::abs(car->getCurrSpeed());
    float decelerationRate = car->getAcc() * 2.0f;
    float safetyBuffer = 1.5f;
    float brakingZone = (currentSpeed * currentSpeed) / (2.0f * decelerationRate) * safetyBuffer;

    // Decision logic
    if (curvature >= 0.85f)
    {
        verticalInput = carInput::Up;
    }
    else
    {
        if (distToNextGate > brakingZone)
        {
            verticalInput = carInput::Up;
        }
        else
        {
            verticalInput = carInput::Down;
        }
    }
}

float AIController::getOvertakeDetectionRadius()
{
    int currIdx = car->getCurrWaypointIndex();
    int nextIdx = (currIdx + 1) % waypoints.size();

    sf::Vector2f currGate = waypoints[currIdx].mid;
    sf::Vector2f nextGate = waypoints[nextIdx].mid;

    sf::Vector2f path = normalize(nextGate - currGate);
    sf::Vector2f carDir = car->getDirectionVector();

    float curvature = dotProduct(path, carDir);

    // Straight: large detection radius, Corner: small detection radius
    if (curvature >= constants.CORNER_THRESHOLD)
    {
        return constants.OVERTAKE_DETECTION_STRAIGHT;
    }
    else
    {
        return constants.OVERTAKE_DETECTION_CORNER;
    }
}

Car *AIController::getOvertakeTarget(const std::vector<Car *> &cars, float range)
{
    Car *nearest = nullptr;
    float maxDist = range * range;
    for (Car *other : cars)
    {
        if (!other->getActive() || other == car)
            continue;
        float dist = distance(other->getPosition(), car->getPosition());
        if (dist >= maxDist)
            continue;
        sf::Vector2f carPos = car->getPosition();
        sf::Vector2f otherPos = other->getPosition();
        sf::Vector2f carDir = car->getDirectionVector();
        sf::Vector2f toOther = otherPos - carPos;
        float dot = dotProduct(carDir, toOther);

        if (dot <= constants.IN_FRONT_THRESHOLD)
            continue; // Car is not in front

        maxDist = dist;
        nearest = other;
    }

    return nearest;
}

void AIController::targetOvertake(const std::vector<Car *> &cars)
{
    float detectionRadius = getOvertakeDetectionRadius();
    Car *rival = getOvertakeTarget(cars, detectionRadius);

    if (rival == nullptr)
    {
        AI *aiCar = static_cast<AI *>(car);
        aiCar->targetSide = defaultSide;
        return;
    }

    // Rival detected and in front
    static_cast<AI *>(car)->targetSide = TargetSide::Mid;
    int currIdx = car->getCurrWaypointIndex();
    const Waypoint &currWaypoint = waypoints[currIdx];

    sf::Vector2f rivalPos = rival->getPosition();

    // Distance from rival to left and right sides of gate
    float distToLeft = magnitude(rivalPos - currWaypoint.left);
    float distToRight = magnitude(rivalPos - currWaypoint.right);

    AI *aiCar = static_cast<AI *>(car);
    if (distToLeft < distToRight)
    {
        aiCar->targetSide = TargetSide::Right;
    }
    else
    {
        aiCar->targetSide = TargetSide::Left;
    }
}

void AIController::calculateHorizontalInput(const std::vector<Car *> &cars)
{
    targetOvertake(cars);

    int currIdx = car->getCurrWaypointIndex();
    if (waypoints.empty())
    {
        horizontalInput = carInput::None;
        return;
    }

    sf::Vector2f target;
    AI *aiCar = static_cast<AI *>(car);
    sf::Vector2f dim = aiCar->getDimensions();
    float margin = std::max(dim.x, dim.y) / 2 + constants.SAFE_MARGIN;
    sf::Vector2f rad = margin * car->getPerpendicularVector();
    const Waypoint &currWp = waypoints[currIdx];
    switch (aiCar->targetSide)
    {
    case TargetSide::Left:
        target = currWp.left + rad;
        break;
    case TargetSide::Right:
        target = currWp.right - rad;
        break;
    default:
        target = currWp.mid;
    }

    sf::Vector2f toTarget = normalize(target - car->getPosition());
    float cross = crossProduct(car->getDirectionVector(), toTarget);

    if (cross > constants.STEERING_DEADZONE)
    {
        horizontalInput = carInput::Right;
    }
    else if (cross < -constants.STEERING_DEADZONE)
    {
        horizontalInput = carInput::Left;
    }
    else
    {
        horizontalInput = carInput::None;
    }
}

void AIController::update(const std::vector<Car *> &cars, float dt)
{
    calculateVerticalInput(cars);
    calculateHorizontalInput(cars);
}