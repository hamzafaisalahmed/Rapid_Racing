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
    sf::Texture texture;
    sf::Sprite sprite;

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

public:
    Car(float xp, float yp, float a, float s, float ms, float mrs, float ac);
    void setCollisionFunction(std::function<bool(Car *, sf::Vector2f, float, sf::Vector2f, float, float)> cc);
    void load(const std::string &dir);

    sf::Vector2f getDimensions() const;
    std::vector<sf::Vector2f> getCorners(sf::Vector2f pos, float angle);
    std::vector<sf::Vector2f> getCorners();

    void accelerate(float dt);
    void decelerate(float dt);

    void draw(sf::RenderWindow &window) const;

    sf::Vector2f getPosition() const;
    void setPosition(sf::Vector2f pos);
    void setTitle(std::string s) { title = s; }
    std::string getTitle() const { return title; }
    float getAngle() const;
    float getMaxSpeed() const;
    float getCurrSpeed() const;
    void setMaxSpeed(float ms);
    void setAcc(float a);
    float getAcc() const;
    void setAngle(float a);
    void setCurrSpeed(float s);
    void setMaxReverseSpeed(float mrs);
    float getMaxReverseSpeed() const;
    float getMaxTurnSpeed() const;
    float getTurnFactor() const;
    float handleMovement(float dt, carInput xIn, carInput yIn, float friction);

    bool updateWaypoint(const std::vector<Waypoint> &waypoints);
    int getCurrWaypointIndex() const;
    void resetWaypointIndex();
    int getCurrLap() const;
    void setCurrLap(int c);
    void updateStuckTime(float dt);
    bool isStuck() const;
    void resetPosition(const std::vector<Waypoint> &waypoints);
    void incrementLaps();

    int getRacePos() const;
    void setRacePos(int i);
    bool getActive() const;
    void setActive(bool b);

    void setITime(float duration = 1.f);
    void updateITime(float dt);
    float isInvincible() const;
    float getITime() const;

    void setAngularVelocity(float a);
    float getAngularVelocity() const;
    void addTurnAngularVelocity(float a);
    sf::Vector2f getDirectionVector() const;
    sf::Vector2f getPerpendicularVector() const;

    virtual bool isAI() const { return false; }
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
    TargetSide targetSide = TargetSide::Mid;
    AI(float xp, float yp, float a, float s, float ms, float mrs, float ac);
    AI();
    float handleAIMovement(float dt, float friction);
    bool isAI() const override { return true; }
};