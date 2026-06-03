#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>
#include <stdexcept>
#include "Utils.h"
using namespace std;

class Track
{
    sf::Texture trackTexture;
    sf::Image trackImage;
    sf::Sprite trackSprite;

    float frictionFactor;

public:
    void LoadTrack(const std::string &dir)
    {
        try
        {
            if (!trackTexture.loadFromFile(dir))
            {
                throw std::runtime_error("Texture not found");
            }
            else
            {
                trackImage = trackTexture.copyToImage();
                trackSprite.setTexture(trackTexture);
                trackSprite.setPosition(0.f, 0.f);
            }
        }
        catch (const std::runtime_error &e)
        {
            std::cout << "Track unable to load..." << e.what() << std::endl;
            throw;
        }
    }

    sf::Vector2u getSize() const
    {
        return trackTexture.getSize();
    }

    sf::Sprite &getSprite()
    {
        return trackSprite;
    }

    void draw(sf::RenderWindow &window, sf::VideoMode desktop)
    {
        window.draw(trackSprite);
    }
    bool isOnRoad(sf::Vector2f pos)
    {
        sf::Color pixel = trackImage.getPixel((unsigned)pos.x, (unsigned)pos.y);
        return (pixel.r > 100 && pixel.r < 200 &&
                pixel.g > 100 && pixel.g < 200 &&
                pixel.b > 100 && pixel.b < 200);
    }

    float getFriction()
    {
        return frictionFactor;
    }
    Track() : frictionFactor(0.5f) {}
    ~Track() = default;
};