#pragma once
#include "Utils.h"
#include <fstream>
#include <vector>

class Leaderboard
{
public:
    void saveLapTime(const LapTime &lt)
    {
        std::ofstream file("scores.txt", std::ios::app);
        if (file.is_open())
        {
            file << lt.id << " " << lt.laps << " "
                 << lt.bestLap << " " << lt.totalTime << "\n";
            file.close();
        }
    }

    std::vector<LapTime> loadLapTimes()
    {
        std::vector<LapTime> times;
        std::ifstream file("scores.txt");
        if (file.is_open())
        {
            LapTime lt;
            while (file >> lt.id >> lt.laps >> lt.bestLap >> lt.totalTime)
                times.push_back(lt);
            file.close();
        }
        return times;
    }
};