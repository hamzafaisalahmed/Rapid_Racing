#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Utils.h"
#include "WaypointHandler.h"

class Car;
class AI;

enum class AIState
{
    Cruising,
    Following,
    Passing
};

class AIController
{
public:
    AIController(AI *car, const std::vector<Waypoint> &waypoints, const WaypointHandler &wpHandler, int gridSlot);

    void update(const std::vector<Car *> &cars, float dt);
    void reset();

    carInput getHorizontalInput() const { return horizontalInput; }
    carInput getVerticalInput() const { return verticalInput; }
    sf::Vector2f getDebugTargetPoint() const { return debugTargetPoint; }

private:
    float laneBlockedTime = 0.f;
    const float laneBlockedTimeThreshold = 0.25f;
    float followingHoldTime = 0.f;
    const float followingHoldDuration = 0.4f;
    float passingHoldTime = 0.f;
    const float passingHoldDuration = 0.3f;

    AI *car;
    const std::vector<Waypoint> &waypoints;
    const WaypointHandler &wpHandler;
    int gridSlot;

    TargetSide preferredLane = TargetSide::Mid;
    TargetSide currentLane = TargetSide::Mid;

    carInput horizontalInput = carInput::None;
    carInput verticalInput = carInput::None;
    sf::Vector2f debugTargetPoint;

    // steering
    void updateSteering();
    sf::Vector2f getLaneTarget(int waypointIdx, TargetSide lane) const;
    float steerToleranceDeg = 2.f;

    // speed
    void updateSpeed();

    // traffic awareness (pure detection, no state decisions)
    void updateTrafficAwareness(const std::vector<Car *> &cars);
    float trafficSpeedCap = -1.f;
    const float trafficCheckDist = 500.f;
    bool isLaneClear(TargetSide lane, const std::vector<Car *> &cars) const;
    const float sideCorridorHalfLen = 80.f; // how far fwd/back of me counts as "beside me"
    sf::Vector2f getPassingOverrideTarget(TargetSide lane) const;
    const float minPassingLookahead = 60.f;
    // FSM
    AIState state = AIState::Cruising;
    void updateState(const std::vector<Car *> &cars, float dt);
    bool hasPassed(Car *rival) const;

    Car *carAhead = nullptr; // whichever car is currently blocking, if any
    Car *passingRival = nullptr;

    bool hasPassed(Car *rival, const std::vector<Car *> &leaderboard) const;
    int passingEntryIdx = -1;
    const int maxPassingIndexSpan = 18; // abort safety net, tuned by feel

    // stuck-detection for Following -> Passing trigger
    int followingStartIdx = -1;
    const int stuckIndexWindow = 6; // how many waypoints of no progress before triggering a pass

    float baseAcc = 0.f;
    const float passBoostMul = 1.15f;

    // AIController.h
    sf::Vector2f passingLastPos;
    float passingStuckTime = 0.f;
    const float maxPassingStuckTime = 3.f; // seconds of near-zero movement before force-abort

    sf::Vector2f getRailOffset(int waypointIdx, TargetSide lane) const;
};