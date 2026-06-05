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
        return sf::Vector2u(trackTexture.getSize().x, trackTexture.getSize().y);
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

        // Bounds check to avoid memory crashes
        if (pos.x >= trackImage.getSize().x || pos.y >= trackImage.getSize().y)
            return false;

        sf::Color pixel = trackImage.getPixel((unsigned int)pos.x, (unsigned int)pos.y);

        if (pixel.r <= 2 && pixel.r >= 0 && pixel.g <= 2 && pixel.g >= 0 && pixel.b <= 2 && pixel.b >= 0)
        {
            return false; // border collision
        }
        return true;
    }

    float getFriction() const
    {
        return frictionFactor;
    }
    Track() : frictionFactor(0.998f) {}
    ~Track() = default;
};