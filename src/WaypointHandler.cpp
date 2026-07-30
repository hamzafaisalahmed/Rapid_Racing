// WaypointHandler.cpp
#include "WaypointHandler.h"
#include <cmath>
#include <algorithm>

void WaypointHandler::init(const std::vector<Waypoint> &waypoints, float maxSpeed, float scaleFactor)
{
    data.assign(waypoints.size(), WaypointAIData());
    cornerZones.clear();

    computeCurvature(waypoints);
    smoothCurvature(waypoints, scaleFactor);
    detectCornerZones();
    computeBrakeZones(waypoints, maxSpeed, scaleFactor);
}

void WaypointHandler::computeCurvature(const std::vector<Waypoint> &waypoints)
{
    size_t n = waypoints.size();
    for (size_t i = 0; i < n; ++i)
    {
        // Treat first/last waypoint as a duplicate seam (track loop closes on itself).
        // Skip real wraparound math there — it produces a garbage spike otherwise.
        if (i == 0 || i == n - 1)
        {
            data[i].curvature = 0.f;
            data[i].turnAngle = 0.f;
            continue;
        }

        size_t prev = i - 1;
        size_t next = i + 1;

        sf::Vector2f inVec = waypoints[i].mid - waypoints[prev].mid;
        sf::Vector2f outVec = waypoints[next].mid - waypoints[i].mid;

        float inLen = magnitude(inVec);
        float outLen = magnitude(outVec);
        if (inLen < 0.0001f || outLen < 0.0001f)
        {
            data[i].curvature = 0.f;
            data[i].turnAngle = 0.f;
            continue;
        }

        sf::Vector2f inDir = normalize(inVec);
        sf::Vector2f outDir = normalize(outVec);

        float cross = crossProduct(inDir, outDir);
        float dot = clamp(dotProduct(inDir, outDir), -1.f, 1.f);
        float turnAngleRad = std::atan2(cross, dot);
        float turnAngleDeg = turnAngleRad * (180.f / 3.14159f);

        float segLen = (inLen + outLen) * 0.5f;
        data[i].curvature = turnAngleDeg / segLen;
        data[i].turnAngle = turnAngleDeg;
    }
}

void WaypointHandler::smoothCurvature(const std::vector<Waypoint> &waypoints, float scaleFactor)
{
    size_t n = waypoints.size();
    const float smoothRadius = 80.f * scaleFactor;
    std::vector<float> smoothed(n, 0.f);

    for (size_t i = 0; i < n; ++i)
    {
        // Seam waypoints (first/last) are pinned to zero and never smoothed —
        // they're a duplicate point where the loop closes, not real track shape.
        if (i == 0 || i == n - 1)
        {
            smoothed[i] = 0.f;
            continue;
        }

        float sum = data[i].curvature;
        int count = 1;

        // walk forward, stop hard at the seam instead of wrapping through it
        float accumDist = 0.f;
        size_t j = i;
        while (accumDist < smoothRadius)
        {
            if (j == n - 1)
                break;
            size_t next = j + 1;
            accumDist += magnitude(waypoints[next].mid - waypoints[j].mid);
            sum += data[next].curvature;
            count++;
            j = next;
        }

        // walk backward, stop hard at the seam
        accumDist = 0.f;
        j = i;
        while (accumDist < smoothRadius)
        {
            if (j == 0)
                break;
            size_t prev = j - 1;
            accumDist += magnitude(waypoints[j].mid - waypoints[prev].mid);
            sum += data[prev].curvature;
            count++;
            j = prev;
        }

        smoothed[i] = sum / (float)count;
    }

    for (size_t i = 0; i < n; ++i)
        data[i].curvature = smoothed[i];

    // Derive straight threshold from this track's own curvature distribution,
    // excluding the seam waypoints (always 0, would skew the average down).
    float avgAbs = 0.f;
    size_t sampleCount = 0;
    for (size_t i = 0; i < n; ++i)
    {
        if (i == 0 || i == n - 1)
            continue;
        avgAbs += std::abs(data[i].curvature);
        sampleCount++;
    }
    avgAbs = (sampleCount > 0) ? avgAbs / (float)sampleCount : 0.f;

    straightThreshold = avgAbs * 0.5f;
}

void WaypointHandler::detectCornerZones()
{
    size_t n = data.size();
    std::vector<bool> visited(n, false);

    for (size_t i = 0; i < n; ++i)
    {
        if (visited[i] || std::abs(data[i].curvature) <= straightThreshold)
            continue;

        // start of a new corner zone
        int sign = (data[i].curvature > 0.f) ? 1 : -1;
        size_t startIdx = i;
        size_t endIdx = i;

        size_t j = i;
        while (true)
        {
            size_t next = (j + 1) % n;
            if (next == i)
                break; // wrapped
            bool sameSign = (data[next].curvature > 0.f ? 1 : -1) == sign;
            bool aboveThreshold = std::abs(data[next].curvature) > straightThreshold;
            if (!sameSign || !aboveThreshold)
                break;
            endIdx = next;
            j = next;
        }

        CornerZone zone;
        zone.start = (int)startIdx;
        zone.end = (int)endIdx;

        // find apex (max abs curvature) and accumulate severity stats
        float peak = 0.f;
        float sumAngle = 0.f; // was: float sum = 0.f;  (curvature sum)
        float sumCurv = 0.f;  // keep separately if you still want avgCurvature from curvature
        int count = 0;
        size_t k = startIdx;
        while (true)
        {
            float c = data[k].curvature;
            float a = data[k].turnAngle;
            sumCurv += c;
            sumAngle += a;
            count++;
            if (std::abs(c) > std::abs(peak))
            {
                peak = c;
                zone.apex = (int)k;
            }
            visited[k] = true;
            if (k == endIdx)
                break;
            k = (k + 1) % n;
        }
        zone.peakCurvature = peak;
        zone.avgCurvature = sumCurv / (float)count;
        zone.totalAngle = std::abs(sumAngle); // now real degrees, not inflated
        zone.insideLine = (peak > 0.f) ? TargetSide::Right : TargetSide::Left;

        int zoneID = (int)cornerZones.size();
        cornerZones.push_back(zone);

        k = startIdx;
        while (true)
        {
            data[k].cornerZoneID = zoneID;
            if (k == endIdx)
                break;
            k = (k + 1) % n;
        }
    }
}

void WaypointHandler::computeBrakeZones(const std::vector<Waypoint> &waypoints, float maxSpeed, float scaleFactor)
{
    size_t n = waypoints.size();
    const float baseDistance = 60.f * scaleFactor;
    const float angleScale = 4.f;
    const float minApexSpeedFrac = 0.35f;

    float sharpestPeak = 0.f;
    for (auto &z : cornerZones)
        sharpestPeak = std::max(sharpestPeak, std::abs(z.peakCurvature));

    for (size_t zi = 0; zi < cornerZones.size(); ++zi)
    {
        CornerZone &zone = cornerZones[zi];
        float brakeDistance = baseDistance + angleScale * zone.totalAngle;

        float absPeak = std::abs(zone.peakCurvature);
        const float mildCurv = 0.10f;
        const float sharpCurv = 0.35f;
        float severity = clamp((absPeak - mildCurv) / (sharpCurv - mildCurv), 0.f, 1.f);
        float apexTargetSpeed = maxSpeed * (1.f - severity * (1.f - minApexSpeedFrac));

        // previous zone in track order, wrapping around the loop
        size_t prevZoneIdx = (zi == 0) ? cornerZones.size() - 1 : zi - 1;
        int prevZoneEnd = cornerZones[prevZoneIdx].end;

        for (int laneIdx = 0; laneIdx < 3; ++laneIdx)
        {
            TargetSide lane = (TargetSide)laneIdx;
            float laneMul = (lane == zone.insideLine) ? 0.85f : (lane == TargetSide::Mid ? 1.f : 1.15f);
            float laneBrakeDist = brakeDistance * laneMul;
            zone.brakeDistCache[laneIdx] = laneBrakeDist; // NEW — persist for runtime distance-based lookup
            float laneTargetSpeed = apexTargetSpeed * (lane == zone.insideLine ? 0.95f : (lane == TargetSide::Mid ? 1.f : 1.1f));
            laneTargetSpeed = std::min(laneTargetSpeed, maxSpeed);

            float accum = 0.f;
            size_t idx = (size_t)zone.start;
            while (accum < laneBrakeDist)
            {
                size_t prev = (idx == 0) ? n - 1 : idx - 1;
                if ((int)prev == prevZoneEnd)
                    break;
                accum += magnitude(waypoints[idx].mid - waypoints[prev].mid);
                idx = prev;
                if (idx == (size_t)zone.start)
                    break;
            }

            float usedBrakeDist = std::min(laneBrakeDist, accum); // NEW: clamp to what we actually walked

            size_t walk = idx;
            float distFromStart = 0.f;
            while (true)
            {
                data[walk].brakeZone[laneIdx] = true;
                size_t nxt = (walk + 1) % n;
                float t = (usedBrakeDist > 0.0001f) ? clamp(distFromStart / usedBrakeDist, 0.f, 1.f) : 1.f; // was laneBrakeDist
                data[walk].targetSpeed[laneIdx] = lerp(maxSpeed, laneTargetSpeed, t);
                if (walk == (size_t)zone.apex)
                    break;
                distFromStart += magnitude(waypoints[nxt].mid - waypoints[walk].mid);
                walk = nxt;
                if (walk == idx)
                    break;
            }
        }
    }
}

void WaypointHandler::debugWaypointData(const std::vector<Waypoint> &waypoints, float scaleFactor)
{
    const float angleScale = 4.f * scaleFactor; // mirror computeBrakeZones' actual scaled constants
    const float baseDistance = 60.f * scaleFactor;

    for (size_t i = 0; i < cornerZones.size(); ++i)
    {
        const auto &z = cornerZones[i];
        float totalStraightLen = 0.f; // rough: distance from prev zone end to this zone start
        size_t prevEnd = (i == 0) ? cornerZones.back().end : cornerZones[i - 1].end;
        size_t idx = (size_t)z.start;
        while (idx != prevEnd)
        {
            size_t p = (idx == 0) ? waypoints.size() - 1 : idx - 1;
            totalStraightLen += magnitude(waypoints[idx].mid - waypoints[p].mid);
            idx = p;
        }
        float brakeDist = baseDistance + angleScale * z.totalAngle;
        std::cout << "Zone " << i << ": start=" << z.start << " apex=" << z.apex << " end=" << z.end
                  << " totalAngle=" << z.totalAngle << " peakCurv=" << z.peakCurvature
                  << " brakeDist=" << brakeDist << " availableStraight=" << totalStraightLen << "\n";
    }
    std::cout << "straightThreshold=" << straightThreshold << " scaleFactor=" << scaleFactor << "\n";

    // Print a representative sample around the middle of the track instead of a fixed range
    size_t n = data.size();
    size_t sampleCount = std::min<size_t>(9, n);
    size_t sampleStart = (n > sampleCount) ? (n / 2) - (sampleCount / 2) : 0;
    for (size_t i = sampleStart; i < sampleStart + sampleCount; ++i)
        std::cout << "idx=" << i << " curvature=" << data[i].curvature
                  << " zoneID=" << data[i].cornerZoneID << "\n";
}