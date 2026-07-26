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

    std::string minimapImagePath;

public:
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
        // vector<vector<float>> waypointCoords = {
        //     {1754, 2500, 1754, 2625},
        //     {3990, 2501, 3992, 2623},
        //     {4042, 2495, 4067, 2612},
        //     {4089, 2476, 4141, 2587},
        //     {4139, 2448, 4214, 2541},
        //     {4176, 2405, 4275, 2485},
        //     {4204, 2355, 4330, 2389},
        //     {4212, 2300, 4349, 2300},
        //     {4211, 2257, 4343, 2240},
        //     {4200, 2219, 4321, 2168},
        //     {4173, 2165, 4284, 2104},
        //     {4143, 2130, 4232, 2046},
        //     {4038, 2032, 4145, 1961},
        //     {3983, 1951, 4101, 1895},
        //     {3945, 1834, 4080, 1821},
        //     {3953, 1702, 4081, 1722},
        //     {3989, 1606, 4102, 1667},
        //     {4056, 1519, 4150, 1597},
        //     {4142, 1439, 4228, 1523},
        //     {4255, 1329, 4352, 1412},
        //     {4316, 1233, 4432, 1291},
        //     {4351, 1098, 4480, 1100},
        //     {4344, 1013, 4476, 999},
        //     {4324, 932, 4452, 892},
        //     {4272, 829, 4374, 754},
        //     {4189, 738, 4270, 648},
        //     {4099, 676, 4160, 574},
        //     {3999, 637, 4040, 525},
        //     {3917, 617, 3932, 501},
        //     {3873, 616, 3874, 495},
        //     {2983, 614, 2982, 495},
        //     {2900, 627, 2869, 513},
        //     {2827, 657, 2776, 548},
        //     {2762, 704, 2688, 604},
        //     {2716, 757, 2620, 680},
        //     {2683, 839, 2562, 791},
        //     {2676, 909, 2544, 913},
        //     {2691, 994, 2568, 1038},
        //     {2747, 1082, 2647, 1157},
        //     {2839, 1143, 2774, 1245},
        //     {2942, 1174, 2919, 1287},
        //     {3070, 1184, 3043, 1298},
        //     {3153, 1213, 3098, 1320},
        //     {3226, 1265, 3138, 1354},
        //     {3284, 1353, 3160, 1401},
        //     {3297, 1465, 3162, 1452},
        //     {3265, 1548, 3148, 1495},
        //     {3208, 1615, 3113, 1539},
        //     {3149, 1654, 3074, 1562},
        //     {3035, 1709, 3000, 1596},
        //     {2914, 1734, 2904, 1616},
        //     {2825, 1734, 2836, 1617},
        //     {2721, 1713, 2767, 1603},
        //     {2619, 1666, 2695, 1573},
        //     {1845, 991, 1915, 898},
        //     {1748, 934, 1806, 833},
        //     {1564, 848, 1617, 740},
        //     {1370, 776, 1405, 666},
        //     {1126, 723, 1146, 609},
        //     {915, 705, 923, 587},
        //     {746, 700, 745, 585},
        //     {602, 710, 583, 597},
        //     {479, 758, 427, 652},
        //     {414, 820, 319, 744},
        //     {365, 897, 249, 842},
        //     {347, 979, 215, 970},
        //     {348, 1048, 221, 1066},
        //     {372, 1130, 261, 1184},
        //     {424, 1204, 331, 1284},
        //     {505, 1261, 447, 1367},
        //     {582, 1290, 550, 1403},
        //     {659, 1302, 659, 1418},
        //     {1606, 1303, 1606, 1422},
        //     {1725, 1319, 1700, 1436},
        //     {1832, 1353, 1787, 1456},
        //     {1952, 1436, 1854, 1511},
        //     {2015, 1537, 1906, 1588},
        //     {2036, 1715, 1911, 1693},
        //     {2000, 1830, 1885, 1777},
        //     {1935, 1922, 1835, 1850},
        //     {1841, 2001, 1774, 1905},
        //     {1710, 2061, 1677, 1954},
        //     {1587, 2085, 1587, 1970},
        //     {479, 2085, 480, 1970},
        //     {370, 2123, 320, 2017},
        //     {321, 2169, 226, 2089},
        //     {277, 2243, 159, 2204},
        //     {276, 2300, 148, 2311},
        //     {293, 2380, 194, 2457},
        //     {351, 2451, 280, 2548},
        //     {435, 2495, 389, 2605},
        //     {511, 2505, 511, 2624},
        //     {1754, 2500, 1754, 2625}};
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
    ~Track() = default;
};