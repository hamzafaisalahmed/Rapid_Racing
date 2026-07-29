#pragma once
#include "Utils.h"
#include <fstream>
#include <vector>

class Leaderboard
{
public:
    void saveLapTime(const LapTime &lt)
    {
        try
        {
            if (lt.bestLap == BESTLAP_INIT_VAL)
                return;
            std::vector<LapTime> sortedTimes = loadLapTimes();
            sortedTimes.push_back(lt);
            std::sort(sortedTimes.begin(), sortedTimes.end(),
                      [](const LapTime &a, const LapTime &b)
                      { return a.bestLap < b.bestLap; });
            // Optional: Keep only top 10
            if (sortedTimes.size() > 10)
                sortedTimes.resize(10);

            // 4. Overwrite file
            std::ofstream file("scores.txt", std::ios::trunc); // Use trunc to overwrite
            if (file.is_open())
            {
                for (const auto &t : sortedTimes)
                {
                    file << t.title << " " << t.bestLap << " " << t.trackID << "\n";
                }
                file.close();
            }
        }
        catch (...)
        {
            throw std::runtime_error("Error in writing to scores file\n");
        }
    }

    std::vector<LapTime> loadLapTimes()
    {
        try
        {
            std::vector<LapTime> times;
            std::ifstream file("scores.txt");
            if (file.is_open())
            {
                LapTime lt;
                while (file >> lt.title >> lt.bestLap >> lt.trackID)
                    times.push_back(lt);
                file.close();
            }
            return times;
        }
        catch (...)
        {
            throw std::runtime_error("Error in reading scores file\n");
        }
    }
};