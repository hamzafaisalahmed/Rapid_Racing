// WaypointHandler.h
#pragma once
#include <vector>
#include "Utils.h"

struct CornerZone
{
    int start = -1, apex = -1, end = -1;
    float peakCurvature = 0.f;
    float avgCurvature = 0.f;
    float totalAngle = 0.f;
    float brakeDistCache[3] = {0.f, 0.f, 0.f}; // NEW
    TargetSide insideLine = TargetSide::Mid;   // Left or Right only, Mid unused here
};

struct WaypointAIData
{
    float curvature = 0.f; // signed, smoothed
    int cornerZoneID = -1; // index into cornerZones, -1 = straight
    float turnAngle = 0.f;
    bool brakeZone[3] = {false, false, false}; // indexed by TargetSide
    float targetSpeed[3] = {0.f, 0.f, 0.f};    // indexed by TargetSide
};

struct WaypointHandler
{
    std::vector<WaypointAIData> data;
    std::vector<CornerZone> cornerZones;
    float straightThreshold = 0.f; // derived, not hardcoded

    void init(const std::vector<Waypoint> &waypoints, float maxSpeed);

private:
    void computeCurvature(const std::vector<Waypoint> &waypoints);
    void smoothCurvature(const std::vector<Waypoint> &waypoints);
    void detectCornerZones();
    void computeBrakeZones(const std::vector<Waypoint> &waypoints, float maxSpeed);
};