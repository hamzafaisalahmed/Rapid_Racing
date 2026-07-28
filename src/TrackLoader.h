#pragma once
#include "Track.h"
#include <nlohmann/json.hpp>

void trackLoader(Track &track, const std::string &jsonFilePath);

std::string getTrackMinimapPath(const std::string &jsonFilePath);