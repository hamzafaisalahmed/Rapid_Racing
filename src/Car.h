#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

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
    vector<sf::Vector2f> corners;

public:
    Car(float xp, float yp, float a, float s, float ms, float mrs, float ac) : position(xp, yp), angle(a), speed(s), maxSpeed(ms), maxReverseSpeed(mrs), acc(ac)
    {
        corners.clear();
        maxTurnSpeed = ms * 0.9f;
    }
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
                sprite.setOrigin(texture.getSize().x / 2.f, texture.getSize().y / 2.f);
                sprite.setPosition(position);
                // corners
                sf::Transform t = sprite.getTransform();
                corners.clear();
                corners.push_back(t.transformPoint(0.f, 0.f));
                corners.push_back(t.transformPoint(texture.getSize().x, 0.f));
                corners.push_back(t.transformPoint(texture.getSize().x, texture.getSize().y));
                corners.push_back(t.transformPoint(0.f, texture.getSize().y));
            }
        }
        catch (const std::runtime_error &e)
        {
            std::cout << "Texture unable to load..." << e.what() << std::endl;
            throw;
        }
    }

    void accelerate(float dt)
    {
        speed += acc * dt;
        if (speed > maxSpeed)
            speed = maxSpeed;
    }

    void decelerate(float dt)
    {
        speed -= acc * dt;
        if (speed < maxReverseSpeed)
            speed = maxReverseSpeed;
    }

    void draw(sf::RenderWindow &window) { window.draw(sprite); }

    sf::Vector2f getPosition() { return position; }
    void setPosition(sf::Vector2f pos)
    {
        position = pos;
        sprite.setPosition(position);
    }
    // handle friction in game
    float getAngle() { return angle; }
    float getMaxSpeed() { return maxSpeed; }
    float getCurrSpeed() { return speed; }
    void setMaxSpeed(float ms)
    {
        maxSpeed = ms;
        maxTurnSpeed = ms * 0.8f;
    }
    void setAcc(float a) { acc = a; }
    float getAcc() { return acc; }
    void setAngle(float a)
    {
        angle = a;
        sprite.setRotation(angle);
    }
    void setCurrSpeed(float s) { speed = s; }
    void setMaxReverseSpeed(float mrs) { maxReverseSpeed = mrs; }
    float getMaxReverseSpeed() { return maxReverseSpeed; }
    float getMaxTurnSpeed() { return maxTurnSpeed; }
    float getTurnSpeed() { return speed / maxTurnSpeed; }
    ~Car() {}
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