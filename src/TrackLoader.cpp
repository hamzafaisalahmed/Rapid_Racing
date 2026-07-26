#include "TrackLoader.h"
using json = nlohmann::json;
#include <fstream>

void trackLoader(Track &track, const std::string &jsonFilePath)
{
    std::ifstream file(jsonFilePath);
    if (!file.is_open())
        throw std::runtime_error("Could not open file from path: " + jsonFilePath);

    json j;
    file >> j;
    std::string imagePath = j.value("image", "");
    if (imagePath.empty())
        throw std::runtime_error("Missing 'image' in track: " + jsonFilePath);
    track.loadTrackImage(imagePath);

    if (j.contains("friction"))
    {
        track.setFriction(j["friction"].get<float>());
    }

    if (j.contains("wallColor") && j["wallColor"].is_array() && j["wallColor"].size() >= 3)
    {
        sf::Color col(
            j["wallColor"][0].get<sf::Uint8>(),
            j["wallColor"][1].get<sf::Uint8>(),
            j["wallColor"][2].get<sf::Uint8>());
        if (j.contains("wallTolerance"))
        {
            track.setWallColor(col, j["wallTolerance"].get<int>());
        }
        else
        {
            track.setWallColor(col); // uses default 2 from the method signature
        }
    }
    std::vector<std::vector<float>> coords;
    for (auto &wp : j["waypoints"])
    {
        if (!wp.is_array() || wp.size() < 4)
        {
            throw std::runtime_error("Invalid waypoint format (need [x1,y1,x2,y2]) " + jsonFilePath);
        }
        coords.push_back({wp[0].get<float>(),
                          wp[1].get<float>(),
                          wp[2].get<float>(),
                          wp[3].get<float>()});
    }
    track.populateWaypoints(coords);

    std::string minimapImage = j.value("minimapImage", "");
    if (minimapImage.empty())
        throw std::runtime_error("Missing 'minimapImage' in track: " + jsonFilePath);
    track.setMinimapImagePath(minimapImage);
}