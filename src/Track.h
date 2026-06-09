#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>
#include <stdexcept>

class Track
{
    sf::Texture trackTexture;
    sf::Image trackImage;
    sf::Sprite trackSprite;

    float frictionFactor;
    float lapCooldown;

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
        // Bounds check to avoid memory crashes, suggested by AI
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
    bool isFinishLine(const std::vector<sf::Vector2f> &positions, float dt)
    {
        if (lapCooldown > 0.f)
        {
            lapCooldown -= dt;
            return false;
        }
        for (const auto &pos : positions)
        {
            if (pos.x < 0 || pos.y < 0 || pos.x >= trackImage.getSize().x || pos.y >= trackImage.getSize().y)
                continue;
            sf::Color pixel = trackImage.getPixel((unsigned int)pos.x, (unsigned int)pos.y);
            if (pixel.r <= 255 && pixel.r >= 250 && pixel.g <= 250 && pixel.g >= 240 && pixel.b >= 0 && pixel.b <= 5)
            {
                lapCooldown = 10.f;
                return true;
            }
        }
        return false;
    }
    void resetCooldown()
    {
        lapCooldown = 0.0f;
    }
    Track() : frictionFactor(0.998f), lapCooldown(0.0f) {}
    ~Track() = default;
};