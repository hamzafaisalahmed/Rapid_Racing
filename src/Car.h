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

public:
    Car(float xp, float yp, float a, float s, float ms, float mrs, float ac) : position(xp, yp), angle(a), speed(s), maxSpeed(ms), maxReverseSpeed(mrs), acc(ac)
    {
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

    vector<sf::Vector2f> getCorners(sf::Vector2f pos, float angle)
    {
        // corners
        sf::Transform t;
        t.translate(pos);
        t.rotate(angle);

        sf::FloatRect bounds = sprite.getLocalBounds();
        float w = (bounds.width * sprite.getScale().x) / 2.f;
        float hf = (bounds.height * sprite.getScale().y) * 0.65f;
        float hr = (bounds.height * sprite.getScale().y) - hf;
        return {
            t.transformPoint(-w, hf), t.transformPoint(w, hf), t.transformPoint(w, -hr), t.transformPoint(-w, -hr)};
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
    float getTurnSpeed()
    {
        float ratio = 1 - (speed / maxTurnSpeed);
        if (ratio < 0.3)
            return 0.3;
        return ratio;
    }
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