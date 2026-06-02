#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
using namespace std;

class Player
{
    sf::Texture texture;
    sf::Sprite sprite;

    sf::Vector2f position;
    float angle;
    float speed;
    float maxSpeed;
    float acc;

public:
    Player() : position(0.f, 0.f), angle(0.f), speed(0.f), maxSpeed(200.f), acc(50.f) {}
    ~Player() {}

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
                sprite.setPosition(0.f, 0.f);
            }
        }
        catch (const std::runtime_error &e)
        {
            std::cout << "Player unable to load..." << e.what() << std::endl;
            throw;
        }
    }

    void draw(sf::RenderWindow &window) { window.draw(sprite); }

    float getAngle() { return angle; }
    float getMaxSpeed() { return maxSpeed; }
    float getCurrSpeed() { return speed; }
    float setAngle(float a) { angle = a; }
    float setCurrSpeed(float s) { speed = s; }
};