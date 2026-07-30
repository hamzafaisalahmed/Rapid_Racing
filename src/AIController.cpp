// AIController.cpp
#include "AIController.h"
#include "Car.h"
#include <cmath>
#include <random>

AIController::AIController(AI *car, const std::vector<Waypoint> &waypoints, const WaypointHandler &wpHandler, int gridSlot)
    : car(car), waypoints(waypoints), wpHandler(wpHandler), gridSlot(gridSlot)
{
    std::mt19937 rng(static_cast<unsigned int>(gridSlot) * 2654435761u + 12345u);
    std::uniform_real_distribution<float> dist(0.15f, 0.45f);
    minAggro = dist(rng);
    aggro = minAggro;
}

void AIController::reset(float scaleFactor)
{
    this->scaleFactor = scaleFactor;
    currentLane = TargetSide::Right;
    horizontalInput = carInput::None;
    verticalInput = carInput::None;
    baseAcc = car->getAcc();
}

sf::Vector2f AIController::getLaneTarget(int waypointIdx, TargetSide lane) const
{
    const Waypoint &wp = waypoints[waypointIdx];
    switch (lane)
    {
    case TargetSide::Left:
        return wp.left + (wp.mid - wp.left) * 0.5f; // biased toward left, not on the rail
    case TargetSide::Right:
        return wp.right + (wp.mid - wp.right) * 0.5f;
    case TargetSide::Mid:
    default:
        return wp.mid;
    }
}

void AIController::updateSteering()
{
    int currIdx = car->getCurrWaypointIndex();
    int n = (int)waypoints.size();
    int prevIdx = (currIdx == 0) ? n - 1 : currIdx - 1;

    bool onStraight = (wpHandler.data[prevIdx].cornerZoneID == -1);

    if (state == AIState::Passing && onStraight)
        debugTargetPoint = getPassingOverrideTarget(currentLane);
    else
        debugTargetPoint = getLaneTarget(currIdx, currentLane);

    sf::Vector2f toTarget = normalize(debugTargetPoint - car->getPosition());
    sf::Vector2f heading = car->getDirectionVector();

    float cross = crossProduct(heading, toTarget);
    float dot = clamp(dotProduct(heading, toTarget), -1.f, 1.f);
    float angleDeg = std::atan2(cross, dot) * (180.f / 3.14159f);

    if (angleDeg > steerToleranceDeg)
        horizontalInput = carInput::Right;
    else if (angleDeg < -steerToleranceDeg)
        horizontalInput = carInput::Left;
    else
        horizontalInput = carInput::None;
}
void AIController::updateSpeed()
{
    int currIdx = car->getCurrWaypointIndex();
    int laneIdx = (int)currentLane;
    int n = (int)waypoints.size();

    int prevIdx = (currIdx == 0) ? n - 1 : currIdx - 1;
    int effectiveZone = wpHandler.data[prevIdx].cornerZoneID;

    float targetSpeed;
    float distToCornerStart = -1.f; // for debug only
    float laneBrakeDist = -1.f;     // for debug only

    if (effectiveZone != -1)
    {
        targetSpeed = wpHandler.data[currIdx].targetSpeed[laneIdx];
        if (targetSpeed <= 0.f)
            targetSpeed = car->getMaxSpeed();
    }
    else
    {
        int nextCornerStart = -1;
        for (int step = 1; step <= n; ++step)
        {
            int idx = (currIdx + step) % n;
            if (wpHandler.data[idx].cornerZoneID != -1)
            {
                nextCornerStart = idx;
                break;
            }
        }

        if (nextCornerStart == -1)
        {
            targetSpeed = car->getMaxSpeed();
        }
        else
        {
            int nextZoneID = wpHandler.data[nextCornerStart].cornerZoneID;
            float brakeStartSpeed = wpHandler.data[nextCornerStart].targetSpeed[laneIdx];
            if (brakeStartSpeed <= 0.f)
                brakeStartSpeed = car->getMaxSpeed();

            laneBrakeDist = wpHandler.cornerZones[nextZoneID].brakeDistCache[laneIdx];
            distToCornerStart = magnitude(waypoints[nextCornerStart].mid - car->getPosition());

            if (laneBrakeDist <= 0.0001f || distToCornerStart > laneBrakeDist)
                targetSpeed = car->getMaxSpeed();
            else
            {
                float t = clamp(distToCornerStart / laneBrakeDist, 0.f, 1.f);
                targetSpeed = lerp(brakeStartSpeed, car->getMaxSpeed(), t);
            }
        }
    }

    float currSpeed = car->getCurrSpeed();

    if (trafficSpeedCap >= 0.f)
    {
        targetSpeed = std::min(targetSpeed, trafficSpeedCap);
    }

    if (currSpeed < targetSpeed - 5.f)
        verticalInput = carInput::Up;
    else if (currSpeed > targetSpeed + 5.f)
        verticalInput = carInput::Down;
    else
        verticalInput = carInput::None;
}

void AIController::updateTrafficAwareness(const std::vector<Car *> &leaderboard)
{
    trafficSpeedCap = -1.f;
    carAhead = nullptr;

    std::vector<sf::Vector2f> corners = car->getCorners();
    sf::Vector2f frontLeft = corners[0];
    sf::Vector2f frontRight = corners[1];
    sf::Vector2f heading = car->getDirectionVector();
    sf::Vector2f perp = car->getPerpendicularVector();

    float projectDist = std::max(trafficCheckDist * scaleFactor, car->getCurrSpeed() * 0.6f);

    // Added a 20% safety margin to the width so it doesn't clip corners
    float halfWidth = magnitude(frontRight - frontLeft) * 0.5f * 1.2f;
    sf::Vector2f frontMid = (frontLeft + frontRight) * 0.5f;

    float closestDist = projectDist;

    for (Car *other : leaderboard)
    {
        if (other == car)
            continue;

        std::vector<sf::Vector2f> otherCorners = other->getCorners();
        bool inPath = false;
        float minAlongDist = projectDist;

        // 1. Check all 4 corners of the other car
        for (const auto &otherCorner : otherCorners)
        {
            sf::Vector2f toP = otherCorner - frontMid;
            float alongDist = dotProduct(toP, heading);
            float sideDist = dotProduct(toP, perp);

            if (alongDist > 0.f && alongDist < projectDist && std::abs(sideDist) < halfWidth)
            {
                inPath = true;
                if (alongDist < minAlongDist)
                    minAlongDist = alongDist; // Track the closest point of intersection
            }
        }

        // 2. Also check the center just in case (e.g. if the car is very large)
        sf::Vector2f toCenter = other->getPosition() - frontMid;
        float centerAlong = dotProduct(toCenter, heading);
        float centerSide = dotProduct(toCenter, perp);

        if (centerAlong > 0.f && centerAlong < projectDist && std::abs(centerSide) < halfWidth)
        {
            inPath = true;
            if (centerAlong < minAlongDist)
                minAlongDist = centerAlong;
        }

        // 3. Register the closest car found in our path
        if (inPath && minAlongDist < closestDist)
        {
            closestDist = minAlongDist;
            carAhead = other;
        }
    }

    if (carAhead)
    {
        bool exemptFromCap = (state == AIState::Passing && carAhead == passingRival && isLaneClear(currentLane, leaderboard));

        if (!exemptFromCap)
            trafficSpeedCap = carAhead->getCurrSpeed();
    }
}

bool AIController::isLaneClear(TargetSide lane, const std::vector<Car *> &leaderboard) const
{
    int currIdx = car->getCurrWaypointIndex();
    sf::Vector2f laneTarget = getLaneTarget(currIdx, lane);
    sf::Vector2f heading = car->getDirectionVector();
    sf::Vector2f perp = car->getPerpendicularVector();
    sf::Vector2f myPos = car->getPosition();

    // Lateral offset of the target lane, relative to my current position
    float laneLateralOffset = dotProduct(laneTarget - myPos, perp);

    // Since we are checking the exact physical corners of the other car,
    // the collision width is just the space OUR car will take up (with a 20% safety buffer).
    float myHalfWidth = car->getDimensions().x * 0.5f * 1.2f;

    for (Car *other : leaderboard)
    {
        if (other == car)
            continue;

        // Grab the 4 corners, and add the center point just to be completely safe
        std::vector<sf::Vector2f> otherPoints = other->getCorners();
        otherPoints.push_back(other->getPosition());

        for (const auto &point : otherPoints)
        {
            sf::Vector2f toPoint = point - myPos;
            float alongDist = dotProduct(toPoint, heading);

            // 1. Is this point within our forward/backward danger zone?
            if (std::abs(alongDist) <= sideCorridorHalfLen * scaleFactor)
            {
                float pointLateral = dotProduct(toPoint, perp);

                // 2. Is this point physically inside the lateral space we need for the target lane?
                if (std::abs(pointLateral - laneLateralOffset) < myHalfWidth)
                {
                    return false; // The lane is occupied
                }
            }
        }
    }

    return true; // No corners found in our target lane space
}

bool AIController::hasPassed(Car *rival, const std::vector<Car *> &leaderboard) const
{
    if (!rival)
        return true;

    auto myIt = std::find(leaderboard.begin(), leaderboard.end(), car);
    auto rivalIt = std::find(leaderboard.begin(), leaderboard.end(), rival);

    if (myIt == leaderboard.end() || rivalIt == leaderboard.end())
        return true; // one of us isn't in the leaderboard (inactive/finished) -> treat as resolved

    return std::distance(leaderboard.begin(), myIt) < std::distance(leaderboard.begin(), rivalIt);
}

sf::Vector2f AIController::getPassingOverrideTarget(TargetSide lane) const
{
    int currIdx = car->getCurrWaypointIndex();
    int n = (int)waypoints.size();

    // The segment is ALWAYS from prevIdx to currIdx
    int prevIdx = (currIdx == 0) ? n - 1 : currIdx - 1;
    int dirIdx = (currIdx == 0) ? 1 : currIdx;

    sf::Vector2f segDir = normalize(waypoints[dirIdx].mid - waypoints[prevIdx].mid);
    sf::Vector2f segPerp(-segDir.y, segDir.x);

    sf::Vector2f toCar = car->getPosition() - waypoints[prevIdx].mid;
    float segLen = magnitude(waypoints[dirIdx].mid - waypoints[prevIdx].mid);
    float projT = (segLen > 0.0001f) ? clamp(dotProduct(toCar, segDir) / segLen, 0.f, 1.f) : 0.f;
    sf::Vector2f centerlinePoint = waypoints[prevIdx].mid + segDir * (projT * segLen);

    float forwardDist = std::max(minPassingLookahead * scaleFactor, car->getCurrSpeed() * 0.6f);

    sf::Vector2f fullLaneTarget = getRailOffset(currIdx, lane); // full rail, not halved
    sf::Vector2f fullMidTarget = getRailOffset(currIdx, TargetSide::Mid);
    float laneLateralOffset = dotProduct(fullLaneTarget - fullMidTarget, segPerp);

    return centerlinePoint + segDir * forwardDist + segPerp * laneLateralOffset;
}

sf::Vector2f AIController::getRailOffset(int waypointIdx, TargetSide lane) const // new helper, or inline
{
    const Waypoint &wp = waypoints[waypointIdx];
    switch (lane)
    {
    case TargetSide::Left:
        return wp.left;
    case TargetSide::Right:
        return wp.right;
    default:
        return wp.mid;
    }
}

void AIController::updateAggro(float dt)
{
    float decayRate = aggroDecayRate * ((carAhead == nullptr) ? aggroLeadDecayMul : 1.f);
    aggro -= decayRate * dt;

    if (state == AIState::Following)
        aggro += aggroStuckRate * dt;

    int currPos = car->getRacePos();
    if (lastRacePos != -1 && currPos > lastRacePos)
        aggro += aggroPassedBoost;
    lastRacePos = currPos;

    aggro = clamp(aggro, minAggro, maxAggro);

    followingHoldDuration = 0.6f - (0.6f - 0.25f) * aggro;
    passingHoldDuration = 0.35f - (0.35f - 0.2f) * aggro;
    passBoostMul = 1.05f + (1.5f - 1.05f) * aggro;
    maxPassingIndexSpan = (int)(14.f + (24.f - 14.f) * aggro);
    laneBlockedTimeThreshold = 0.18f + (0.35f - 0.18f) * aggro;
    defenseFatigueThreshold = 2.f + (6.f - 2.f) * aggro;
}

void AIController::updateState(const std::vector<Car *> &leaderboard, float dt)
{
    int currIdx = car->getCurrWaypointIndex();
    bool blocked = (carAhead != nullptr);

    if (!blocked)
    {
        state = AIState::Cruising;
        passingRival = nullptr;

        TargetSide wantLane = preferredLane;

        if (targetedBy && targetedByTimeout > 0.f)
        {
            TargetSide attackerLane = targetedBy->getLaneSide(waypoints, currIdx);
            bool wantsToBlock = (aggro > 0.66f);
            bool wantsPartial = (aggro > 0.4f && aggro <= 0.66f);

            if (wantsToBlock)
                wantLane = attackerLane;
            else if (wantsPartial)
                wantLane = TargetSide::Mid;
            else
                wantLane = (attackerLane == TargetSide::Left) ? TargetSide::Right : TargetSide::Left;

            if (wantsToBlock && wantLane == attackerLane)
            {
                defenseFatigue += dt;
                if (defenseFatigue > defenseFatigueThreshold)
                {
                    wantLane = (attackerLane == TargetSide::Left) ? TargetSide::Right : TargetSide::Left;
                    defenseFatigue = 0.f;
                }
            }
            else
            {
                defenseFatigue = std::max(0.f, defenseFatigue - defenseFatigueDecay * dt);
            }
        }
        else
        {
            defenseFatigue = std::max(0.f, defenseFatigue - defenseFatigueDecay * dt);
        }

        if (currentLane != wantLane && isLaneClear(wantLane, leaderboard))
            currentLane = wantLane;

        return;
    }

    if (state == AIState::Cruising)
    {
        state = AIState::Following;
        followingHoldTime = 0.f;
    }

    if (state == AIState::Following)
    {
        followingHoldTime += dt;
        if (followingHoldTime < followingHoldDuration)
            return;

        TargetSide sideA = TargetSide::Left;
        TargetSide sideB = TargetSide::Right;

        bool aClear = isLaneClear(sideA, leaderboard);
        bool bClear = isLaneClear(sideB, leaderboard);

        TargetSide chosen = currentLane;
        bool found = false;

        if (aClear && currentLane != sideA)
        {
            chosen = sideA;
            found = true;
        }
        else if (bClear && currentLane != sideB)
        {
            chosen = sideB;
            found = true;
        }
        else if (aClear)
        {
            chosen = sideA;
            found = true;
        }
        else if (bClear)
        {
            chosen = sideB;
            found = true;
        }

        if (found)
        {
            currentLane = chosen;
            passingRival = carAhead;
            passingEntryIdx = currIdx;
            state = AIState::Passing;
            passingLastPos = car->getPosition();
            passingStuckTime = 0.f;
            laneBlockedTime = 0.f;
            passingHoldTime = 0.f;
        }
    }
    else if (state == AIState::Passing)
    {
        if (passingRival)
        {
            if (AIController *rivalAI = passingRival->getAIController())
                rivalAI->notifyTargeted(car);
        }

        passingHoldTime += dt;
        bool holdExpired = (passingHoldTime >= passingHoldDuration);

        if (holdExpired && carAhead != passingRival)
        {
            state = AIState::Following;
            passingRival = nullptr;
            followingHoldTime = 0.f;
            return;
        }

        if (holdExpired && hasPassed(passingRival, leaderboard))
        {
            state = AIState::Cruising;
            passingRival = nullptr;
            return;
        }

        if (!isLaneClear(currentLane, leaderboard))
        {
            laneBlockedTime += dt;
            if (holdExpired && laneBlockedTime > laneBlockedTimeThreshold)
            {
                state = AIState::Following;
                passingRival = nullptr;
                laneBlockedTime = 0.f;
                followingHoldTime = 0.f;
                return;
            }
        }
        else
        {
            laneBlockedTime = 0.f;
        }

        int n = (int)waypoints.size();
        int idxDelta = (currIdx - passingEntryIdx + n) % n;
        if (holdExpired && idxDelta >= maxPassingIndexSpan)
        {
            state = AIState::Following;
            passingRival = nullptr;
            followingHoldTime = 0.f;
        }

        float movedDist = magnitude(car->getPosition() - passingLastPos);
        if (movedDist < 5.f * scaleFactor)
            passingStuckTime += dt;
        else
        {
            passingStuckTime = 0.f;
            passingLastPos = car->getPosition();
        }

        if (holdExpired && passingStuckTime > maxPassingStuckTime)
        {
            state = AIState::Following;
            passingRival = nullptr;
            followingHoldTime = 0.f;
        }
    }
}

void AIController::update(const std::vector<Car *> &cars, float dt)
{
    updateTrafficAwareness(cars);

    updateAggro(dt);

    if (targetedBy)
    {
        targetedByTimeout -= dt;
        if (targetedByTimeout <= 0.f)
            targetedBy = nullptr;
    }

    updateState(cars, dt);
    updateSteering();
    updateSpeed();
    if (state == AIState::Passing && verticalInput == carInput::Up)
        car->setAcc(baseAcc * passBoostMul);
    else
        car->setAcc(baseAcc);
}