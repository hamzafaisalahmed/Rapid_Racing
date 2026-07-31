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

    float frictionFactor = 0.998f;
    std::vector<Waypoint> waypoints;

    sf::Color wallColor = sf::Color::Black;
    int wallTolerance = 2;
    float startPosA;
    float startPosB1;
    float startPosB2;
    float startRowSpacing;
    float startAngle;
    std::string minimapImagePath;
    float minimapScale;
    int id;
    float scaleFactor = 1.f;

public:
    Track() : startPosA(0.f), startPosB1(0.f), startPosB2(0.f), startRowSpacing(0.f), startAngle(0.f), minimapScale(0.f)
    {
        minimapImagePath = "";
    }
    void loadTrackImage(const std::string &dir)
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
                trackSprite.setTexture(trackTexture, true);
                trackSprite.setPosition(0.f, 0.f);
                computeScaleFactor();
            }
        }
        catch (const std::runtime_error &e)
        {
            std::cout << "Track unable to load..." << e.what() << std::endl;
            throw;
        }
    }

    void setFriction(float friction)
    {
        frictionFactor = friction;
    }

    std::vector<Waypoint> getWaypoints() const { return waypoints; }

    void setWallColor(sf::Color c, int tolerance = 2)
    {
        wallColor = c;
        wallTolerance = tolerance;
    }

    void populateWaypoints(std::vector<std::vector<float>> waypointCoords)
    {
        waypoints.clear();
        for (size_t i = 0; i < waypointCoords.size(); i++)
        {
            waypoints.push_back(Waypoint(sf::Vector2f(waypointCoords[i][0], waypointCoords[i][1]), sf::Vector2f(waypointCoords[i][2], waypointCoords[i][3])));
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
        if (pos.x >= trackImage.getSize().x || pos.y >= trackImage.getSize().y || pos.x < 0 || pos.y < 0)
            return false;

        sf::Color pixel = trackImage.getPixel((unsigned int)pos.x, (unsigned int)pos.y);

        if (std::abs((int)pixel.r - (int)wallColor.r) <= wallTolerance &&
            std::abs((int)pixel.g - (int)wallColor.g) <= wallTolerance &&
            std::abs((int)pixel.b - (int)wallColor.b) <= wallTolerance)
        {
            return false;
        }
        return true;
    }

    float getFriction() const
    {
        return frictionFactor;
    }

    void setMinimapImagePath(const std::string &img) { minimapImagePath = img; }
    std::string getMinimapImagePath() const { return minimapImagePath; }

    void setStartPosA(float pos) { startPosA = pos; }
    void setStartPosB1(float pos) { startPosB1 = pos; }
    void setStartPosB2(float pos) { startPosB2 = pos; }
    void setStartRowSpacing(float spacing) { startRowSpacing = spacing; }
    float getStartPosA() const { return startPosA; }
    float getStartPosB1() const { return startPosB1; }
    float getStartPosB2() const { return startPosB2; }
    float getStartRowSpacing() const { return startRowSpacing; }
    void setStartAngle(float angle) { startAngle = angle; }
    float getStartAngle() const { return startAngle; }
    void setMinimapScale(float s) { minimapScale = s; }
    float getMinimapScale() const { return minimapScale; }
    void setID(int ID) { id = ID; }
    float getID() const { return id; }
    void computeScaleFactor() { scaleFactor = computeTrackScaleFactor(getSize()); }
    float getScaleFactor() const { return scaleFactor; }
    ~Track() = default;
};