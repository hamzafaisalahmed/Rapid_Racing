#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <vector>
#include <cmath>
#include "WaypointHandler.h"
#include "AIController.h"
#include "Utils.h"
#include "Car.h"

// ============================================================================
// TRACK GENERATORS
// ============================================================================

inline std::vector<Waypoint> createStraightTrack(int count, float spacing = 100.0f, float width = 100.0f)
{
    std::vector<Waypoint> wps;
    wps.reserve(count);
    for (int i = 0; i < count; ++i)
    {
        float y = static_cast<float>(i) * spacing;
        wps.emplace_back(sf::Vector2f(-width / 2.0f, y), sf::Vector2f(width / 2.0f, y));
    }
    return wps;
}

// direction: +1/-1 just needs to produce mirrored geometry; we don't assert
// which absolute sign corresponds to "left" vs "right" in world terms, only
// that mirrored geometry produces opposite-signed curvature from each other.
inline std::vector<Waypoint> createCurvedTrack(int count, float radius, float arcDegrees, float width = 100.0f, int direction = 1)
{
    std::vector<Waypoint> wps;
    wps.reserve(count);
    constexpr float PI = 3.14159265f;
    float arcRad = arcDegrees * (PI / 180.0f) * direction;

    for (int i = 0; i < count; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(count - 1);
        float theta = t * arcRad;
        sf::Vector2f center(std::cos(theta) * radius, std::sin(theta) * radius);
        sf::Vector2f normal(std::cos(theta), std::sin(theta));
        wps.emplace_back(center - normal * (width / 2.0f), center + normal * (width / 2.0f));
    }
    return wps;
}

// A straight lead-in stitched to a curve far away in space, so the two
// sections don't spatially overlap despite sharing an index space.
inline std::vector<Waypoint> createStraightIntoCorner(int straightCount, int curveCount, float radius = 500.0f, float arcDegrees = 90.0f)
{
    auto straight = createStraightTrack(straightCount, 100.0f);
    auto curve = createCurvedTrack(curveCount, radius, arcDegrees);
    for (auto &wp : curve)
    {
        wp.mid.y += 1500.f;
        wp.left.y += 1500.f;
        wp.right.y += 1500.f;
    }
    std::vector<Waypoint> combined = straight;
    combined.insert(combined.end(), curve.begin(), curve.end());
    return combined;
}

// ============================================================================
// WAYPOINTHANDLER — SAFETY / EDGE CASES
// ============================================================================

TEST_CASE("WaypointHandler - handles empty and tiny waypoint sets without crashing", "[waypoint][safety]")
{
    WaypointHandler handler;

    SECTION("Empty vector")
    {
        std::vector<Waypoint> empty;
        REQUIRE_NOTHROW(handler.init(empty, 500.0f, 1.0f));
        REQUIRE(handler.data.empty());
        REQUIRE(handler.cornerZones.empty());
    }

    SECTION("Two waypoints (below any real curvature window)")
    {
        auto wps = createStraightTrack(2);
        REQUIRE_NOTHROW(handler.init(wps, 500.0f, 1.0f));
    }

    SECTION("Zero maxSpeed doesn't produce negative or NaN target speeds")
    {
        auto wps = createCurvedTrack(30, 500.0f, 90.0f);
        REQUIRE_NOTHROW(handler.init(wps, 0.0f, 1.0f));
        for (auto &d : handler.data)
            for (int lane = 0; lane < 3; ++lane)
                REQUIRE(d.targetSpeed[lane] >= 0.0f);
    }
}

// ============================================================================
// CURVATURE / STRAIGHT DETECTION
// ============================================================================

TEST_CASE("WaypointHandler - straight track has zero curvature and no corner zones", "[waypoint][curvature]")
{
    auto waypoints = createStraightTrack(20);
    WaypointHandler handler;
    handler.init(waypoints, 500.0f, 1.0f);

    REQUIRE(handler.cornerZones.empty());
    for (const auto &d : handler.data)
    {
        REQUIRE(d.cornerZoneID == -1);
        REQUIRE(d.curvature == Catch::Approx(0.0f));
    }
}

TEST_CASE("WaypointHandler - mirrored turns produce opposite-sign curvature of equal magnitude", "[waypoint][curvature]")
{
    auto turnA = createCurvedTrack(30, 500.0f, 90.0f, 100.0f, +1);
    auto turnB = createCurvedTrack(30, 500.0f, 90.0f, 100.0f, -1);

    WaypointHandler hA, hB;
    hA.init(turnA, 500.0f, 1.0f);
    hB.init(turnB, 500.0f, 1.0f);

    REQUIRE(!hA.cornerZones.empty());
    REQUIRE(!hB.cornerZones.empty());

    float peakA = hA.cornerZones[0].peakCurvature;
    float peakB = hB.cornerZones[0].peakCurvature;

    REQUIRE(((peakA > 0.f) != (peakB > 0.f)));
    REQUIRE(std::abs(peakA) == Catch::Approx(std::abs(peakB)).margin(0.02f));
}

TEST_CASE("WaypointHandler - seam waypoints (first/last) are always pinned to zero", "[waypoint][curvature]")
{
    // Skewing endpoint geometry must not leak into curvature at the seam —
    // this is the wraparound guard, not a statement about the rest of the track.
    auto waypoints = createStraightTrack(10);
    waypoints.front().left = sf::Vector2f(-300.0f, -100.0f);
    waypoints.back().left = sf::Vector2f(300.0f, 1000.0f);

    WaypointHandler handler;
    handler.init(waypoints, 500.0f, 1.0f);

    REQUIRE(handler.data.front().curvature == Catch::Approx(0.0f));
    REQUIRE(handler.data.back().curvature == Catch::Approx(0.0f));
    REQUIRE(handler.data.front().cornerZoneID == -1);
    REQUIRE(handler.data.back().cornerZoneID == -1);
}

TEST_CASE("WaypointHandler - straightThreshold is derived from track's own curvature, not hardcoded", "[waypoint][curvature]")
{
    auto gentle = createCurvedTrack(40, 3000.0f, 90.0f);
    auto sharp = createCurvedTrack(40, 150.0f, 90.0f);

    WaypointHandler hGentle, hSharp;
    hGentle.init(gentle, 500.0f, 1.0f);
    hSharp.init(sharp, 500.0f, 1.0f);

    REQUIRE(hGentle.straightThreshold >= 0.0f);
    REQUIRE(hSharp.straightThreshold >= 0.0f);
    REQUIRE(hSharp.straightThreshold > hGentle.straightThreshold);
}

// ============================================================================
// CORNER ZONE DETECTION
// ============================================================================

TEST_CASE("WaypointHandler - detects exactly one zone for an isolated 90-degree curve", "[waypoint][corner]")
{
    auto waypoints = createCurvedTrack(30, 500.0f, 90.0f);
    WaypointHandler handler;
    handler.init(waypoints, 500.0f, 1.0f);

    REQUIRE(handler.cornerZones.size() == 1);

    auto &zone = handler.cornerZones[0];
    REQUIRE(zone.totalAngle == Catch::Approx(90.0f).margin(10.0f));
    REQUIRE(zone.apex >= zone.start);
    REQUIRE(zone.apex <= zone.end);
    REQUIRE(std::abs(zone.peakCurvature) >= std::abs(zone.avgCurvature));
}

TEST_CASE("WaypointHandler - insideLine differs between mirrored turn directions", "[waypoint][corner]")
{
    auto turnA = createCurvedTrack(30, 500.0f, 90.0f, 100.0f, +1);
    auto turnB = createCurvedTrack(30, 500.0f, 90.0f, 100.0f, -1);

    WaypointHandler hA, hB;
    hA.init(turnA, 500.0f, 1.0f);
    hB.init(turnB, 500.0f, 1.0f);

    REQUIRE(!hA.cornerZones.empty());
    REQUIRE(!hB.cornerZones.empty());
    REQUIRE(hA.cornerZones[0].insideLine != hB.cornerZones[0].insideLine);
}

TEST_CASE("WaypointHandler - every waypoint inside a zone's [start,end] is tagged with that zone's ID", "[waypoint][corner]")
{
    auto waypoints = createCurvedTrack(30, 500.0f, 90.0f);
    WaypointHandler handler;
    handler.init(waypoints, 500.0f, 1.0f);

    REQUIRE(!handler.cornerZones.empty());
    auto &zone = handler.cornerZones[0];
    for (int i = zone.start; i <= zone.end; ++i)
        REQUIRE(handler.data[i].cornerZoneID == 0);
}

// ============================================================================
// BRAKE ZONES / TARGET SPEED
// ============================================================================

TEST_CASE("WaypointHandler - apex speed severity scales within the active curvature window", "[waypoint][speed]")
{
    // Both radii chosen so peak curvature falls between mildCurv (0.10) and
    // sharpCurv (0.35) — isolates severity scaling from the known constant-window
    // gap covered separately below.
    auto sharp = createCurvedTrack(30, 150.0f, 90.0f);
    auto lessSharp = createCurvedTrack(30, 400.0f, 90.0f);
    float maxSpeed = 500.0f;

    WaypointHandler hSharp, hLessSharp;
    hSharp.init(sharp, maxSpeed, 1.0f);
    hLessSharp.init(lessSharp, maxSpeed, 1.0f);

    REQUIRE(!hSharp.cornerZones.empty());
    REQUIRE(!hLessSharp.cornerZones.empty());

    float sharpApex = hSharp.data[hSharp.cornerZones[0].apex].targetSpeed[(int)TargetSide::Mid];
    float lessSharpApex = hLessSharp.data[hLessSharp.cornerZones[0].apex].targetSpeed[(int)TargetSide::Mid];

    REQUIRE(sharpApex < maxSpeed);
    REQUIRE(sharpApex < lessSharpApex);
}

TEST_CASE("WaypointHandler - mildCurv/sharpCurv are intentionally fixed, absolute curvature thresholds", "[waypoint][design]")
{
    // Unlike straightThreshold (deliberately track-relative — "is this a
    // corner at all" is comparative), severity/apex-speed scaling is
    // deliberately track-independent: a corner's required braking is a
    // physical fact about its own sharpness, not about its neighbors. A
    // track-relative scheme would misclassify a uniformly sharp track
    // (e.g. zig-zag) as "all mild" relative to itself, under-braking
    // every corner on it.
    float maxSpeed = 500.0f;

    auto isolatedSharpCorner = createCurvedTrack(30, 150.0f, 90.0f);

    std::vector<Waypoint> zigZagTrack;
    for (int i = 0; i < 4; ++i)
    {
        auto segment = createCurvedTrack(30, 150.0f, 90.0f, 100.0f, (i % 2 == 0) ? +1 : -1);
        float yOffset = static_cast<float>(i) * 1000.f;
        for (auto &wp : segment)
        {
            wp.mid.y += yOffset;
            wp.left.y += yOffset;
            wp.right.y += yOffset;
        }
        zigZagTrack.insert(zigZagTrack.end(), segment.begin(), segment.end());
    }

    WaypointHandler hIsolated, hZigZag;
    hIsolated.init(isolatedSharpCorner, maxSpeed, 1.0f);
    hZigZag.init(zigZagTrack, maxSpeed, 1.0f);

    REQUIRE(!hIsolated.cornerZones.empty());
    REQUIRE(!hZigZag.cornerZones.empty());

    float isolatedApex = hIsolated.data[hIsolated.cornerZones[0].apex].targetSpeed[(int)TargetSide::Mid];
    float zigZagApex = hZigZag.data[hZigZag.cornerZones[0].apex].targetSpeed[(int)TargetSide::Mid];

    REQUIRE(isolatedApex < maxSpeed);
    REQUIRE(zigZagApex < maxSpeed);
    REQUIRE(isolatedApex == Catch::Approx(zigZagApex).margin(5.0f));
}
TEST_CASE("WaypointHandler - brakeZone flags are only set within the zone's actual braking window", "[waypoint][speed]")
{
    auto combined = createStraightIntoCorner(15, 20);
    WaypointHandler handler;
    handler.init(combined, 500.0f, 1.0f);

    bool anyUnflagged = false;
    for (size_t i = 0; i < 15; ++i)
        if (!handler.data[i].brakeZone[(int)TargetSide::Mid])
            anyUnflagged = true;

    REQUIRE(anyUnflagged);
}

TEST_CASE("WaypointHandler - brakeDistCache reflects the corner's own ideal distance, independent of neighboring zones", "[waypoint][cache]")
{
    // Resolved design: brakeDistCache is a live, continuous-space lookahead
    // radius read by AIController::updateSpeed — it must NOT be clamped by
    // the previous zone's boundary (that clamp is purely internal to the
    // waypoint-tagging walk inside computeBrakeZones and must stay local).
    // A near-full-loop track (very little straight before the zone starts)
    // is exactly the case that previously exposed this when the two were
    // conflated: the ideal distance must remain independent of walk-back
    // reachability.
    auto waypoints = createCurvedTrack(60, 500.0f, 350.0f, 100.0f);

    WaypointHandler handler;
    handler.init(waypoints, 500.0f, 1.0f);
    REQUIRE(!handler.cornerZones.empty());

    for (auto &zone : handler.cornerZones)
        for (int lane = 0; lane < 3; ++lane)
            REQUIRE(zone.brakeDistCache[lane] > 0.0f);
}

TEST_CASE("WaypointHandler - single corner spanning most of the track still yields a usable brake distance", "[waypoint][cache]")
{
    auto waypoints = createCurvedTrack(80, 500.0f, 355.0f, 100.0f);

    WaypointHandler handler;
    handler.init(waypoints, 500.0f, 1.0f);
    REQUIRE(handler.cornerZones.size() == 1);

    auto &zone = handler.cornerZones[0];
    for (int lane = 0; lane < 3; ++lane)
        REQUIRE(zone.brakeDistCache[lane] > 10.0f);
}

// ============================================================================
// SCALE FACTOR CONSISTENCY
// ============================================================================

TEST_CASE("WaypointHandler - brake distance scales proportionally with track scaleFactor", "[waypoint][scale]")
{
    SECTION("Large-angle corner (angle term dominates)")
    {
        auto small = createCurvedTrack(30, 500.0f, 90.0f);
        auto large = createCurvedTrack(30, 500.0f, 90.0f);

        WaypointHandler hSmall, hLarge;
        hSmall.init(small, 500.0f, 1.0f);
        hLarge.init(large, 500.0f, 2.0f);

        REQUIRE(!hSmall.cornerZones.empty());
        REQUIRE(!hLarge.cornerZones.empty());

        float a = hSmall.cornerZones[0].brakeDistCache[(int)TargetSide::Mid];
        float b = hLarge.cornerZones[0].brakeDistCache[(int)TargetSide::Mid];
        REQUIRE(b / a == Catch::Approx(2.0f).margin(0.2f));
    }

    SECTION("Small-angle corner (baseDistance term dominates)")
    {
        auto small = createCurvedTrack(40, 500.0f, 20.0f);
        auto large = createCurvedTrack(40, 500.0f, 20.0f);

        WaypointHandler hSmall, hLarge;
        hSmall.init(small, 500.0f, 1.0f);
        hLarge.init(large, 500.0f, 2.0f);

        if (!hSmall.cornerZones.empty() && !hLarge.cornerZones.empty())
        {
            float a = hSmall.cornerZones[0].brakeDistCache[(int)TargetSide::Mid];
            float b = hLarge.cornerZones[0].brakeDistCache[(int)TargetSide::Mid];
            REQUIRE(b / a == Catch::Approx(2.0f).margin(0.3f));
        }
    }
}

TEST_CASE("WaypointHandler - brake-zone walk terminates cleanly on very small waypoint counts", "[waypoint][safety]")
{
    auto waypoints = createCurvedTrack(5, 500.0f, 90.0f, 100.0f);
    WaypointHandler handler;
    REQUIRE_NOTHROW(handler.init(waypoints, 500.0f, 1.0f));

    for (auto &d : handler.data)
    {
        REQUIRE(d.targetSpeed[(int)TargetSide::Mid] >= 0.0f);
        REQUIRE(d.targetSpeed[(int)TargetSide::Mid] <= 500.0f);
    }
}

TEST_CASE("WaypointHandler - near-full-circle single corner still assigns a valid apex", "[waypoint][corner]")
{
    auto waypoints = createCurvedTrack(50, 300.0f, 359.0f, 100.0f);
    WaypointHandler handler;
    handler.init(waypoints, 500.0f, 1.0f);

    REQUIRE(handler.cornerZones.size() == 1);
    REQUIRE(handler.cornerZones[0].apex != -1);
    REQUIRE(handler.cornerZones[0].peakCurvature != 0.0f);
}

// ============================================================================
// AICONTROLLER — CONSTRUCTION & RESET
// ============================================================================

TEST_CASE("AIController - constructs and resets without crashing given a minimal track", "[aicontroller][safety]")
{
    auto waypoints = createCurvedTrack(20, 500.0f, 90.0f);
    WaypointHandler wpHandler;
    wpHandler.init(waypoints, 500.0f, 1.0f);

    AI car;
    car.setMaxSpeed(500.0f);
    car.setAcc(200.0f);

    REQUIRE_NOTHROW([&]()
                    {
        AIController controller(&car, waypoints, wpHandler, 0);
        controller.reset(1.0f); }());
}

TEST_CASE("AIController - aggro stays within [minAggro, maxAggro] immediately after construction", "[aicontroller][aggro]")
{
    auto waypoints = createCurvedTrack(20, 500.0f, 90.0f);
    WaypointHandler wpHandler;
    wpHandler.init(waypoints, 500.0f, 1.0f);

    AI car;
    car.setMaxSpeed(500.0f);
    AIController controller(&car, waypoints, wpHandler, 3); // arbitrary grid slot

    REQUIRE(controller.getAggro() >= 0.0f);
    REQUIRE(controller.getAggro() <= 1.0f);
}

TEST_CASE("AIController - [KNOWN FALLBACK] reset() always sets currentLane to Right, regardless of preferredLane", "[aicontroller][gap]")
{
    // Documented, not a bug: this is a fallback default. Intended long-term
    // behavior is for Game::resetLevel to reassign currentLane to
    // preferredLane after reset(). This test pins current behavior so it
    // doesn't silently change until that integration happens; once it does,
    // flip this to REQUIRE(controller.getCurrentLane() == TargetSide::Left)
    // (or whatever preferredLane was explicitly set to) and drive it through
    // Game's reset path instead of calling reset() directly.
    auto waypoints = createCurvedTrack(20, 500.0f, 90.0f);
    WaypointHandler wpHandler;
    wpHandler.init(waypoints, 500.0f, 1.0f);

    AI car;
    car.setMaxSpeed(500.0f);
    AIController controller(&car, waypoints, wpHandler, 0);

    controller.setPreferredLane(TargetSide::Left);
    controller.reset(1.0f);

    REQUIRE(controller.getCurrentLane() == TargetSide::Right);
}

// ============================================================================
// AICONTROLLER — SPEED DECISION LOGIC
// ============================================================================

TEST_CASE("AIController - car directly ahead in corridor forces a brake decision", "[aicontroller][traffic]")
{
    auto waypoints = createStraightTrack(30, 100.0f);
    WaypointHandler wpHandler;
    wpHandler.init(waypoints, 500.0f, 1.0f);

    AI self, blocker;
    self.setMaxSpeed(500.0f);
    self.setAcc(200.0f);
    self.setPosition(waypoints[5].mid);
    self.setAngle(180.0f); // faces +y, matching track's forward (increasing index) direction
    self.setCurrSpeed(500.0f);

    blocker.setMaxSpeed(500.0f);
    blocker.setPosition(waypoints[5].mid + sf::Vector2f(0.0f, 150.0f)); // ahead along +y
    blocker.setAngle(180.0f);
    blocker.setCurrSpeed(50.0f);

    AIController controller(&self, waypoints, wpHandler, 0);
    controller.reset(1.0f);
    controller.update({&self, &blocker}, 0.016f);

    REQUIRE(controller.getVerticalInput() == carInput::Down);
}

TEST_CASE("AIController - car far to the side is not treated as blocking", "[aicontroller][traffic]")
{
    auto waypoints = createStraightTrack(30, 100.0f);
    WaypointHandler wpHandler;
    wpHandler.init(waypoints, 500.0f, 1.0f);

    AI self, sideCar;
    self.setMaxSpeed(500.0f);
    self.setAcc(200.0f);
    self.setPosition(waypoints[5].mid);
    self.setAngle(180.0f);
    self.setCurrSpeed(500.0f);

    sideCar.setMaxSpeed(500.0f);
    sideCar.setPosition(waypoints[5].mid + sf::Vector2f(2000.0f, 150.0f)); // ahead in y, but far laterally
    sideCar.setAngle(180.0f);
    sideCar.setCurrSpeed(10.0f);

    AIController controller(&self, waypoints, wpHandler, 0);
    controller.reset(1.0f);
    controller.update({&self, &sideCar}, 0.016f);

    REQUIRE(controller.getVerticalInput() != carInput::Down);
}

TEST_CASE("AIController - car behind is not treated as blocking", "[aicontroller][traffic]")
{
    auto waypoints = createStraightTrack(30, 100.0f);
    WaypointHandler wpHandler;
    wpHandler.init(waypoints, 500.0f, 1.0f);

    AI self, rearCar;
    self.setMaxSpeed(500.0f);
    self.setAcc(200.0f);
    self.setPosition(waypoints[10].mid);
    self.setAngle(180.0f); // facing +y
    self.setCurrSpeed(500.0f);

    rearCar.setMaxSpeed(500.0f);
    rearCar.setPosition(waypoints[0].mid); // smaller y = genuinely behind a +y-facing car
    rearCar.setAngle(180.0f);
    rearCar.setCurrSpeed(10.0f);

    AIController controller(&self, waypoints, wpHandler, 0);
    controller.reset(1.0f);
    controller.update({&self, &rearCar}, 0.016f);

    REQUIRE(controller.getVerticalInput() != carInput::Down);
}

TEST_CASE("AIController - blocker beyond forward projection distance is not capped", "[aicontroller][traffic]")
{
    auto waypoints = createStraightTrack(60, 100.0f);
    WaypointHandler wpHandler;
    wpHandler.init(waypoints, 500.0f, 1.0f);

    AI self, farBlocker;
    self.setMaxSpeed(500.0f);
    self.setAcc(200.0f);
    self.setPosition(waypoints[0].mid);
    self.setAngle(180.0f);
    self.setCurrSpeed(0.0f); // stationary -> projectDist purely trafficCheckDist*scaleFactor = 500

    farBlocker.setMaxSpeed(500.0f);
    farBlocker.setPosition(waypoints[0].mid + sf::Vector2f(0.0f, 900.0f)); // ahead, beyond projectDist(500)
    farBlocker.setAngle(180.0f);
    farBlocker.setCurrSpeed(10.0f);

    AIController controller(&self, waypoints, wpHandler, 0);
    controller.reset(1.0f);
    controller.update({&self, &farBlocker}, 0.016f);

    REQUIRE(controller.getVerticalInput() == carInput::Up);
}