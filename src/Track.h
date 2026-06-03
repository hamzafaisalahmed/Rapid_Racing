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
                trackSprite.setScale(4.f, 4.f);
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
        return sf::Vector2u(trackTexture.getSize().x * 4.f, trackTexture.getSize().y * 4.f);
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
        // Divide by 4.f to match the scale factor applied to the sprite
        unsigned int x = static_cast<unsigned int>(pos.x / 4.f);
        unsigned int y = static_cast<unsigned int>(pos.y / 4.f);

        // Bounds check to avoid memory crashes
        if (x >= trackImage.getSize().x || y >= trackImage.getSize().y)
            return false;

        sf::Color pixel = trackImage.getPixel(x, y);
        return (pixel.r > 100 && pixel.r < 200 &&
                pixel.g > 100 && pixel.g < 200 &&
                pixel.b > 100 && pixel.b < 200);
    }

    float getFriction() const
    {
        return frictionFactor;
    }
    Track() : frictionFactor(0.98f) {}
    ~Track() = default;
};