#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "Utils.h"
#include <functional>
#include <cmath>

class Car
{
protected:
    sf::Texture texture;
    sf::Sprite sprite;

    sf::Vector2f position;
    float angle;
    float speed;
    float maxSpeed;
    float maxReverseSpeed;
    float acc;
    float maxTurnSpeed;
    const float standardTurnFactor = 500.f;
    std::function<bool(sf::Vector2f, float)> collisionChecker;
    int currWaypointIndex;
    int currLap;

public:
    Car(float xp, float yp, float a, float s, float ms, float mrs, float ac) : position(xp, yp), angle(a), speed(s), maxSpeed(ms), maxReverseSpeed(mrs), acc(ac), maxTurnSpeed(ms * 0.9f), currWaypointIndex(0), currLap(0) {}
    void setCollisionFunction(std::function<bool(sf::Vector2f, float)> cc) { collisionChecker = cc; }
    void load(const std::string &dir)
    {
        try
        {
            if (!texture.loadFromFile(dir))
            {
                throw std::runtime_error("Texture not found");
            }
            else
            {
                sprite.setTexture(texture);
                sprite.setScale(0.08f, 0.08f);
                sprite.setOrigin(texture.getSize().x / 2.f, texture.getSize().y * 0.65f);
                sprite.setPosition(position);
            }
        }
        catch (const std::runtime_error &e)
        {
            std::cout << "Texture unable to load..." << e.what() << std::endl;
            throw;
        }
    }

    std::vector<sf::Vector2f> getCorners(sf::Vector2f pos, float angle)
    {
        // corners
        sf::Transform t;
        t.translate(pos);
        t.rotate(angle);

        sf::FloatRect bounds = sprite.getLocalBounds();
        float w = (bounds.width * sprite.getScale().x) / 2.2f;
        float hf = (bounds.height * sprite.getScale().y) * 0.3f;
        float hr = (bounds.height * sprite.getScale().y) * 0.6f; // might have to make custom for all cars
        return {
            t.transformPoint(-w, hf), t.transformPoint(w, hf), t.transformPoint(-w, -hr), t.transformPoint(w, -hr)};
    }
    void accelerate(float dt)
    {
        speed += acc * dt;
        if (speed > maxSpeed)
            speed = maxSpeed;
    }

    void decelerate(float dt)
    {
        speed -= acc * 2 * dt;
        if (speed < maxReverseSpeed)
            speed = maxReverseSpeed;
    }

    void draw(sf::RenderWindow &window) { window.draw(sprite); }

    sf::Vector2f getPosition() const { return position; }
    void setPosition(sf::Vector2f pos)
    {
        position = pos;
        sprite.setPosition(position);
    }
    // handle friction in game
    float getAngle() const { return angle; }
    float getMaxSpeed() const { return maxSpeed; }
    float getCurrSpeed() const { return speed; }
    void setMaxSpeed(float ms)
    {
        maxSpeed = ms;
        maxTurnSpeed = ms * 0.9f;
    }
    void setAcc(float a) { acc = a; }
    float getAcc() const { return acc; }
    void setAngle(float a)
    {
        angle = a;
        sprite.setRotation(angle);
    }
    void setCurrSpeed(float s) { speed = s; }
    void setMaxReverseSpeed(float mrs) { maxReverseSpeed = mrs; }
    float getMaxReverseSpeed() const { return maxReverseSpeed; }
    float getMaxTurnSpeed() const { return maxTurnSpeed; }
    float getTurnSpeed() const
    {
        if (std::abs(speed) < 1.f)
            return 0.f;
        float ratio = 1 - (std::abs(speed) / maxTurnSpeed);
        if (ratio >= 0.8f)
            return 0.8f;
        if (ratio <= 0.3f)
            return 0.3f;
        return ratio;
    }
    float handleMovement(float dt, carInput xIn, carInput yIn, float friction)
    {
        sf::Vector2f oldPosition = getPosition();
        float angle = getAngle();
        float oldAngle = angle;
        float turnFactor = 0.f;
        bool dec = false;
        if (xIn == carInput::Left)
        {
            turnFactor = -1.f * standardTurnFactor;
        }
        else if (xIn == carInput::Right)
        {
            turnFactor = standardTurnFactor;
        }
        angle += turnFactor * getTurnSpeed() * dt;

        if (yIn == carInput::Up)
        {
            accelerate(dt);
        }
        else if (yIn == carInput::Down)
        {
            decelerate(dt);
            dec = true;
        }
        else
        {
            dec = true;
            if (std::abs(getCurrSpeed()) < 10.f)
                setCurrSpeed(0.f);
            else if (std::abs(getCurrSpeed()) < 60.f)
                setCurrSpeed(getCurrSpeed() * friction * 0.985f);
            else
                setCurrSpeed(getCurrSpeed() * friction);
        }

        float speed = getCurrSpeed();
        sf::Vector2f position = getPosition();
        position.x += std::cos((angle - 90.f) * (3.14159f / 180.f)) * speed * dt;
        position.y += std::sin((angle - 90.f) * (3.14159f / 180.f)) * speed * dt;
        if (collisionChecker(position, angle))
        {
            setPosition(oldPosition);
            setCurrSpeed(speed * -0.3f);
            setAngle(oldAngle);
        }
        else
        {
            setPosition(position);
            setCurrSpeed(speed);
            setAngle(lerp(oldAngle, angle, 0.6f));
        }
        float targetVolume = 0;
        if (dec)
            targetVolume = 0;
        else
            targetVolume = std::abs(getCurrSpeed() / getMaxSpeed()) * 100.f;
        return targetVolume;
    }

    bool updateWaypoint(const std::vector<Waypoint> &waypoints)
    {
        if (distance(waypoints[currWaypointIndex].mid, getPosition()) < (69.f * 69.f)) // squared distance so square highest width 138/2
        {
            currWaypointIndex++;
            if ((size_t)currWaypointIndex >= waypoints.size())
            {
                currWaypointIndex = 0;
                return true;
            }
        }
        return false;
    }
    int getCurrWaypointIndex() const { return currWaypointIndex; }
    int getCurrLap() const { return currLap; }
    void setCurrLap(int c) { currLap = c; }
    ~Car() = default;
};
class Player : public Car
{

public:
    ~Player() {}
    Player(float xp, float yp, float a, float s, float ms, float mrs, float ac) : Car(xp, yp, a, s, ms, mrs, ac) {}
    Player() : Car(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f) {}
};

class AI : public Car
{
public:
    AI(float xp, float yp, float a, float s, float ms, float mrs, float ac) : Car(xp, yp, a, s, ms, mrs, ac) {}
};