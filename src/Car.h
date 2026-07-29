#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cmath>
#include "Utils.h"

class AIController;

class Car
{
protected:
    sf::Texture baseTexture;
    sf::Texture detailTexture;
    sf::Sprite baseSprite;
    sf::Sprite detailSprite;

    LapTime lapData;

    std::string title;
    sf::Vector2f position;
    sf::Vector2f dimensions;
    float angle;
    float speed;
    float maxSpeed;
    float maxReverseSpeed;
    float acc;
    float maxTurnSpeed;
    float angularVelocity = 0.f;
    float turnAngularVelocity = 0.f;
    const float standardTurnFactor = 500.f;
    std::function<bool(Car *, sf::Vector2f, float, sf::Vector2f, float, float)> collisionChecker;
    int currWaypointIndex;
    int currLap;
    int racePos;
    float stuckTime;
    bool isActive;
    const float maxStuckTime = 2.f;
    float iTime;
    const float maxITime = 2.f;
    const float maxTurnAngularVelocity = 360.f;

    void syncSprites(sf::Vector2f p, float a);
    bool finishedRace = false;

    bool isAccelerating = false;

public:
    Car(float xp, float yp, float a, float s, float ms, float mrs, float ac);
    void setCollisionFunction(std::function<bool(Car *, sf::Vector2f, float, sf::Vector2f, float, float)> cc) { collisionChecker = cc; }
    void load(const std::string &baseDir, const std::string &detailDir);
    void setBodyColor(sf::Color color) { baseSprite.setColor(color); }

    bool getAccelerating() const { return isAccelerating; }

    sf::Vector2f getDimensions() const { return dimensions; }
    std::vector<sf::Vector2f> getCorners(sf::Vector2f pos, float angle);
    std::vector<sf::Vector2f> getCorners();
    bool isFinishedRace() const { return finishedRace; }
    void setFinishedRace(bool b) { finishedRace = b; }
    void accelerate(float dt);
    void decelerate(float dt);

    void draw(sf::RenderWindow &window) const;

    void setPosition(sf::Vector2f pos);
    void setAngle(float a);

    sf::Vector2f getPosition() const { return position; }
    void setTitle(std::string s)
    {
        title = s;
        lapData.title = s;
    }
    std::string getTitle() const { return title; }
    float getAngle() const { return angle; }
    float getMaxSpeed() const { return maxSpeed; }
    float getCurrSpeed() const { return speed; }
    void setAcc(float a) { acc = a; }
    float getAcc() const { return acc; }
    void setCurrSpeed(float s) { speed = s; }
    void setMaxReverseSpeed(float mrs) { maxReverseSpeed = mrs; }
    float getMaxReverseSpeed() const { return maxReverseSpeed; }
    float getMaxTurnSpeed() const { return maxTurnSpeed; }
    void setMaxSpeed(float ms)
    {
        maxSpeed = ms;
        maxTurnSpeed = ms * 0.9f;
    }

    float getTurnFactor() const;
    void handleMovement(float dt, carInput xIn, carInput yIn, float friction);

    bool updateWaypoint(const std::vector<Waypoint> &waypoints);
    int getCurrWaypointIndex() const { return currWaypointIndex; }
    void resetWaypointIndex() { currWaypointIndex = 0; }
    int getCurrLap() const { return currLap; }
    void setCurrLap(int c) { currLap = c; }
    void updateStuckTime(float dt);
    bool isStuck() const { return stuckTime > maxStuckTime; }

    void resetPosition(const std::vector<Waypoint> &waypoints);

    void incrementLaps() { currLap++; }

    int getRacePos() const { return racePos; }
    void setRacePos(int i) { racePos = i; }
    bool getActive() const { return isActive; }
    void setActive(bool b) { isActive = b; }

    void setITime(float duration = 1.f) { iTime = duration; }
    void updateITime(float dt);
    float isInvincible() const { return iTime > 0.f; }
    float getITime() const { return iTime; }

    void setAngularVelocity(float a) { angularVelocity = a; }
    float getAngularVelocity() const { return angularVelocity; }
    void addTurnAngularVelocity(float a) { turnAngularVelocity = clamp(a, -maxTurnAngularVelocity, maxTurnAngularVelocity); }
    sf::Vector2f getDirectionVector() const;
    sf::Vector2f getPerpendicularVector() const;

    virtual bool isAI() const { return false; }

    void setTrackID(int id) { lapData.trackID = id; }
    float getBestLapTime() const { return lapData.bestLap; }
    void updateCurrentLapTime(float dt) { lapData.currentLapTime += dt; }
    void resetCurrentLapTime()
    {
        if (lapData.currentLapTime < lapData.bestLap)
            lapData.bestLap = lapData.currentLapTime;
        lapData.currentLapTime = 0;
    }
    void resetAllLapTime()
    {
        lapData.currentLapTime = 0;
        lapData.bestLap = BESTLAP_INIT_VAL;
    }
    float getCurrentLapTime() const { return lapData.currentLapTime; }
    void updateCarTimes(float dt)
    {
        updateITime(dt);
        updateCurrentLapTime(dt);
        updateStuckTime(dt);
    }
    LapTime getLapData() const { return lapData; }
    bool checkCollision(sf::Vector2f pos, float angle)
    {
        if (!collisionChecker)
            return false;
        return collisionChecker(this, pos, angle, position, angle, 0.f);
    }
    virtual TargetSide getLaneSide(const std::vector<Waypoint> &waypoints, int waypointIdx) const;

    virtual AIController *getAIController() const { return nullptr; }
    virtual ~Car() = default;
};

class Player : public Car
{
public:
    ~Player() {}
    Player(float xp, float yp, float a, float s, float ms, float mrs, float ac);
    Player();
};

class AI : public Car
{
public:
    std::unique_ptr<AIController> aiController;
    AI(float xp, float yp, float a, float s, float ms, float mrs, float ac);
    AI();
    void handleAIMovement(float dt, float friction);
    bool isAI() const override { return true; }
    TargetSide getLaneSide(const std::vector<Waypoint> &waypoints, int waypointIdx) const override;
    AIController *getAIController() const override;
};