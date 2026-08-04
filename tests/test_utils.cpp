#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "Utils.h"

TEST_CASE("Math & Clamp Helpers", "[utils]")
{

    SECTION("clamp restricts values to min and max bounds")
    {
        REQUIRE(clamp(5.0f, 0.0f, 10.0f) == 5.0f);   // Inside range
        REQUIRE(clamp(-2.0f, 0.0f, 10.0f) == 0.0f);  // Below min
        REQUIRE(clamp(15.0f, 0.0f, 10.0f) == 10.0f); // Above max
    }

    SECTION("lerp linearly interpolates between two values")
    {
        REQUIRE(lerp(0.0f, 100.0f, 0.5f) == Catch::Approx(50.0f));
        REQUIRE(lerp(10.0f, 20.0f, 0.0f) == Catch::Approx(10.0f));
        REQUIRE(lerp(10.0f, 20.0f, 1.0f) == Catch::Approx(20.0f));
    }

    SECTION("distance calculates squared Euclidean distance")
    {
        sf::Vector2f p1(0.0f, 0.0f);
        sf::Vector2f p2(3.0f, 4.0f);
        // 3^2 + 4^2 = 25 (squared distance, no sqrt)
        REQUIRE(distance(p1, p2) == Catch::Approx(25.0f));
    }
}

TEST_CASE("Vector Operations", "[utils]")
{

    SECTION("dotProduct and crossProduct")
    {
        sf::Vector2f v1(1.0f, 0.0f);
        sf::Vector2f v2(0.0f, 1.0f);

        REQUIRE(dotProduct(v1, v2) == Catch::Approx(0.0f));   // Perpendicular vectors
        REQUIRE(crossProduct(v1, v2) == Catch::Approx(1.0f)); // 2D Z-component
    }

    SECTION("normalize handles normal vectors and zero vectors")
    {
        sf::Vector2f v(10.0f, 0.0f);
        sf::Vector2f norm = normalize(v);
        REQUIRE(norm.x == Catch::Approx(1.0f));
        REQUIRE(norm.y == Catch::Approx(0.0f));

        // Prevents division-by-zero crashes on zero vectors
        sf::Vector2f zeroVec(0.0f, 0.0f);
        sf::Vector2f normZero = normalize(zeroVec);
        REQUIRE(normZero.x == Catch::Approx(0.0f));
        REQUIRE(normZero.y == Catch::Approx(0.0f));
    }
}

TEST_CASE("Track Scaling & Formatting", "[utils]")
{

    SECTION("computeTrackScaleFactor relative to reference size")
    {
        sf::Vector2u refSize(4800, 3200);
        REQUIRE(computeTrackScaleFactor(refSize) == Catch::Approx(1.0f));

        sf::Vector2u doubleSize(9600, 6400);
        REQUIRE(computeTrackScaleFactor(doubleSize) == Catch::Approx(2.0f));
    }

    SECTION("formatRaceTime builds MM:SS.mm formatted strings")
    {
        REQUIRE(formatRaceTime(0.0f) == "0:00.00");
        REQUIRE(formatRaceTime(65.25f) == "1:05.25");
        REQUIRE(formatRaceTime(125.05f) == "2:05.05");
    }

    SECTION("Waypoint constructor automatically sets mid point")
    {
        sf::Vector2f left(-10.0f, 20.0f);
        sf::Vector2f right(30.0f, 20.0f);
        Waypoint wp(left, right);

        REQUIRE(wp.mid.x == Catch::Approx(10.0f)); // (-10 + 30) / 2
        REQUIRE(wp.mid.y == Catch::Approx(20.0f)); // (20 + 20) / 2
    }
}

#include "Utils.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("normalizeAngle", "[utils][math]")
{
    // Rounding
    REQUIRE(normalizeAngle(45.2f) == 45);
    REQUIRE(normalizeAngle(45.8f) == 46);
    REQUIRE(normalizeAngle(359.6f) == 0); // rounds to 360 -> 0

    // Positive angles
    REQUIRE(normalizeAngle(0.f) == 0);
    REQUIRE(normalizeAngle(90.f) == 90);
    REQUIRE(normalizeAngle(180.f) == 180);
    REQUIRE(normalizeAngle(270.f) == 270);
    REQUIRE(normalizeAngle(359.f) == 359);
    REQUIRE(normalizeAngle(360.f) == 0);
    REQUIRE(normalizeAngle(450.f) == 90);

    // Negative angles
    REQUIRE(normalizeAngle(-1.f) == 359);
    REQUIRE(normalizeAngle(-90.f) == 270);
    REQUIRE(normalizeAngle(-180.f) == 180);
    REQUIRE(normalizeAngle(-270.f) == 90);
    REQUIRE(normalizeAngle(-359.f) == 1);
    REQUIRE(normalizeAngle(-360.f) == 0);
    REQUIRE(normalizeAngle(-450.f) == 270);

    // Large magnitude
    REQUIRE(normalizeAngle(1000.f) == 280);
    REQUIRE(normalizeAngle(-1000.f) == 80);
}