#include "Track.h"
#include "CollisionHandler.h"
#include "Car.h"
#include "Utils.h"
#include "AIController.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <vector>
#include <cmath>

// ============================================================
// Friend functions for Track (declared as friends in Track.h)
// ============================================================
void setTrackTestImage(Track &track, unsigned int w, unsigned int h, sf::Color color)
{
    track.trackImage.create(w, h, color);
}

void paintTrackRegion(Track &track, sf::IntRect rect, sf::Color color)
{
    for (int y = rect.top; y < rect.top + rect.height; ++y)
        for (int x = rect.left; x < rect.left + rect.width; ++x)
            if (x >= 0 && y >= 0 &&
                (unsigned)x < track.trackImage.getSize().x &&
                (unsigned)y < track.trackImage.getSize().y)
                track.trackImage.setPixel((unsigned)x, (unsigned)y, color);
}

// ============================================================
// Friend helper for CollisionHandler::checkCarCollisions
// (declared as friend in CollisionHandler.h)
// ============================================================
CarCollisionResult callCheckCarCollisions(const CollisionHandler &handler,
                                          Car *car1, Car *car2,
                                          sf::Vector2f pos, float angle)
{
    return handler.checkCarCollisions(car1, car2, pos, angle);
}

// ============================================================
// GEOMETRY NOTES (derived from Car::getCorners(pos,angle), current fixed version)
//
// boundsWidth=60, boundsHeight=100 (placeholder, no sprite loaded)
// w = 60/2.6 ≈ 23.08, hf (front) = 60, hr (rear) = 30
// At angle=0: corners = {(-w,-hf), (w,-hf), (-w,hr), (w,hr)}
//           = front-left, front-right, rear-left, rear-right (world = pos + local, no rotation)
//
// Front-left/rear-left SHARE the same x (-w) and only differ in y.
// Front-right/rear-right SHARE the same x (+w) and only differ in y.
// => A vertical wall boundary (x-based) can only ever split LEFT vs RIGHT
//    corners from each other — it can never isolate front from rear, since
//    front/rear pairs share identical x. Front/rear hits require a
//    HORIZONTAL wall boundary (y-based) instead.
// => checkWallCollisions checks frontHit -> rearHit -> leftHit -> rightHit
//    in that order, each requiring BOTH corners on that side off-road.
//
// getDirectionVector() at angle=0 points (0,-1) — i.e. "up"/-y. So a car at
// angle=0 driving into a wall at smaller y is a head-on hit against its front.
// ============================================================

// ============================================================
// PART 1: WALL DETECTION (isOnRoad / checkWallCollisions surface only)
// ============================================================
TEST_CASE("CollisionHandler - wall detection across road, wall, tolerance, and out-of-bounds cases", "[collision][wall]")
{
    Track track;
    setTrackTestImage(track, 1000, 1000, sf::Color::White);
    paintTrackRegion(track, sf::IntRect(600, 0, 400, 1000), sf::Color::Black); // vertical wall band, x in [600,1000)

    std::vector<Car *> cars;
    CollisionHandler handler(track, cars);

    AI car(0.f, 0.f, 0.f, 100.f, 500.f, -50.f, 200.f);
    cars.push_back(&car);

    SECTION("Car fully in the road panel produces no collision")
    {
        car.setPosition(sf::Vector2f(300.0f, 500.0f));
        car.setAngle(0.0f);
        bool collided = handler.handleCollision(&car, car.getPosition(), car.getAngle(),
                                                car.getPosition(), car.getAngle(), 0.016f);
        REQUIRE_FALSE(collided);
    }

    SECTION("Car driven fully into the wall panel triggers a collision")
    {
        car.setPosition(sf::Vector2f(700.0f, 500.0f));
        car.setAngle(0.0f);
        bool collided = handler.handleCollision(&car, car.getPosition(), car.getAngle(),
                                                car.getPosition(), car.getAngle(), 0.016f);
        REQUIRE(collided);
    }

    SECTION("Position outside image bounds is treated as off-road")
    {
        REQUIRE_FALSE(track.isOnRoad(sf::Vector2f(-10.0f, 500.0f)));
        REQUIRE_FALSE(track.isOnRoad(sf::Vector2f(500.0f, 2000.0f)));
    }

    SECTION("Custom wall color and tolerance are respected")
    {
        Track customTrack;
        sf::Color customWall(200, 50, 50);
        setTrackTestImage(customTrack, 200, 200, sf::Color::White);
        paintTrackRegion(customTrack, sf::IntRect(0, 0, 200, 200), customWall);
        customTrack.setWallColor(customWall, 5);
        REQUIRE_FALSE(customTrack.isOnRoad(sf::Vector2f(100.0f, 100.0f)));
    }

    SECTION("Car rotated 45 degrees against the wall boundary still detects a collision")
    {
        car.setPosition(sf::Vector2f(590.f, 500.f));
        car.setAngle(45.f);
        bool collided = handler.handleCollision(&car, car.getPosition(), car.getAngle(),
                                                car.getPosition(), car.getAngle(), 0.016f);
        REQUIRE(collided);
    }
}

// ============================================================
// PART 2: SAT CAR-CAR COLLISION DETECTION
//
// Both cars use the getCorners() placeholder (60 wide, no texture loaded),
// so half-width ~= 23.08 each -> cars stop overlapping only past a gap of
// roughly 46.16. Test separations are chosen with margin around that value.
// ============================================================
TEST_CASE("CollisionHandler - SAT car-car collision detection", "[collision][car-car]")
{
    Track dummyTrack;
    std::vector<Car *> emptyCars;
    CollisionHandler handler(dummyTrack, emptyCars);

    Car car1(0.f, 0.f, 0.f, 0.f, 100.f, -50.f, 50.f);
    Car car2(0.f, 0.f, 0.f, 0.f, 100.f, -50.f, 50.f);
    car1.setActive(true);
    car2.setActive(true);

    SECTION("No overlap, far apart")
    {
        car1.setPosition({0.f, 0.f});
        car1.setAngle(0.f);
        car2.setPosition({500.f, 0.f});
        car2.setAngle(0.f);
        auto result = callCheckCarCollisions(handler, &car1, &car2, car1.getPosition(), car1.getAngle());
        REQUIRE_FALSE(result.hit);
        REQUIRE(result.minOverlap == 0.f);
    }

    SECTION("Overlap, centre to centre")
    {
        car1.setPosition({0.f, 0.f});
        car1.setAngle(0.f);
        car2.setPosition({15.f, 0.f});
        car2.setAngle(0.f);
        auto result = callCheckCarCollisions(handler, &car1, &car2, car1.getPosition(), car1.getAngle());
        REQUIRE(result.hit);
        REQUIRE(result.minOverlap > 0.f);
        REQUIRE(result.car1Index >= 0);
        REQUIRE(result.car2Index >= 0);
    }

    SECTION("Overlap, rotated 45 degrees")
    {
        car1.setPosition({0.f, 0.f});
        car1.setAngle(45.f);
        car2.setPosition({15.f, 0.f});
        car2.setAngle(0.f);
        auto result = callCheckCarCollisions(handler, &car1, &car2, car1.getPosition(), car1.getAngle());
        REQUIRE(result.hit);
        REQUIRE(result.minOverlap > 0.f);
    }

    SECTION("No overlap, side by side with a genuine gap")
    {
        // Half-widths sum to ~46.16 — 80 leaves clear separation with margin.
        car1.setPosition({0.f, 0.f});
        car1.setAngle(0.f);
        car2.setPosition({80.f, 0.f});
        car2.setAngle(0.f);
        auto result = callCheckCarCollisions(handler, &car1, &car2, car1.getPosition(), car1.getAngle());
        REQUIRE_FALSE(result.hit);
    }

    SECTION("No overlap, vertical offset beyond car length")
    {
        car1.setPosition({0.f, 0.f});
        car1.setAngle(0.f);
        car2.setPosition({0.f, 250.f});
        car2.setAngle(0.f);
        auto result = callCheckCarCollisions(handler, &car1, &car2, car1.getPosition(), car1.getAngle());
        REQUIRE_FALSE(result.hit);
    }
}

// ============================================================
// PART 3: COLLISION RESPONSE (wall-hit physics)
//
// Each SECTION builds its own dedicated wall geometry, since front/rear vs
// left/right vs single-corner hits each require a different wall shape to
// isolate cleanly — reusing one shared band produced ambiguous results.
// ============================================================
TEST_CASE("CollisionHandler - wall collision response physics", "[collision][response]")
{
    SECTION("Head-on hit (index 4) applies headCollisionDamping, no spin")
    {
        // Horizontal wall band at y < 400; car at angle=0 (heading -y) placed
        // so its front corners (y = pos.y - 60) fall inside the band while
        // its rear corners (y = pos.y + 30) stay on the road.
        Track track;
        setTrackTestImage(track, 1000, 1000, sf::Color::White);
        paintTrackRegion(track, sf::IntRect(0, 0, 1000, 400), sf::Color::Black);

        std::vector<Car *> cars;
        CollisionHandler handler(track, cars);

        AI car(0.f, 0.f, 0.f, 100.f, 500.f, -50.f, 200.f);
        cars.push_back(&car);
        sf::Vector2f oldPos(500.f, 460.f);           // clear of the wall
        car.setPosition(sf::Vector2f(500.f, 430.f)); // front y=370 (wall), rear y=460 (road)
        car.setAngle(0.f);

        handler.handleCollision(&car, car.getPosition(), car.getAngle(),
                                oldPos, car.getAngle(), 0.016f);

        // headCollisionDamping = -0.7f, initial speed 100 -> ~ -70
        REQUIRE(car.getCurrSpeed() == Catch::Approx(-70.f).margin(1.f));
        REQUIRE(car.getAngularVelocity() == Catch::Approx(0.f).margin(0.1f)); // indices 4/5 never set spin
    }

    SECTION("Rear hit (index 5) applies headCollisionDamping symmetrically")
    {
        // Mirror of the front case: wall band above, car facing away from it.
        Track track;
        setTrackTestImage(track, 1000, 1000, sf::Color::White);
        paintTrackRegion(track, sf::IntRect(0, 600, 1000, 400), sf::Color::Black);

        std::vector<Car *> cars;
        CollisionHandler handler(track, cars);

        AI car(0.f, 0.f, 0.f, 100.f, 500.f, -50.f, 200.f);
        cars.push_back(&car);
        sf::Vector2f oldPos(500.f, 540.f);           // clear of the wall
        car.setPosition(sf::Vector2f(500.f, 570.f)); // rear y=600 (wall boundary), front y=510 (road)
        car.setAngle(0.f);

        handler.handleCollision(&car, car.getPosition(), car.getAngle(),
                                oldPos, car.getAngle(), 0.016f);

        REQUIRE(car.getCurrSpeed() == Catch::Approx(-70.f).margin(1.f));
    }

    SECTION("Paired left-side hit (index 6) applies sideCollisionDamping, no spin")
    {
        // Vertical wall on the LEFT (x < 400); car straddles the boundary so
        // both left corners (shared x = pos.x - 23.08) go off-road while both
        // right corners stay on-road -> leftHit, index 6 (has an explicit
        // branch: pushDir flips, but spinDirection is never set for index 6).
        Track track;
        setTrackTestImage(track, 1000, 1000, sf::Color::White);
        paintTrackRegion(track, sf::IntRect(0, 0, 400, 1000), sf::Color::Black);

        std::vector<Car *> cars;
        CollisionHandler handler(track, cars);

        AI car(0.f, 0.f, 0.f, 100.f, 500.f, -50.f, 200.f);
        cars.push_back(&car);
        sf::Vector2f oldPos(440.f, 500.f);           // clear of the wall (both corners on road at x=440)
        car.setPosition(sf::Vector2f(410.f, 500.f)); // left corners x≈386.9 (wall), right x≈433.1 (road)
        car.setAngle(0.f);

        handler.handleCollision(&car, car.getPosition(), car.getAngle(),
                                oldPos, car.getAngle(), 0.016f);

        REQUIRE(car.getCurrSpeed() == Catch::Approx(30.f).margin(1.f)); // sideCollisionDamping = 0.3
        REQUIRE(car.getAngularVelocity() == Catch::Approx(0.f).margin(0.1f));
    }

    SECTION("Corner hit against an unresolvable obstruction forces a full stop (speed and spin both zeroed)")
    {
        // The initial resolution nudge is too small to clear this obstruction,
        // so handleCollisionResponse's trailing safety re-check overrides both
        // speed and angular velocity to 0 rather than let the car spin while
        // still embedded in it.
        Track track;
        setTrackTestImage(track, 1000, 1000, sf::Color::White);

        std::vector<Car *> cars;
        CollisionHandler handler(track, cars);

        AI car(0.f, 0.f, 0.f, 100.f, 500.f, -50.f, 200.f);
        cars.push_back(&car);
        sf::Vector2f oldPos(500.f, 500.f);
        car.setPosition(sf::Vector2f(500.f, 500.f));
        car.setAngle(0.f);
        // Front-left corner world pos ≈ (476.92, 440).
        paintTrackRegion(track, sf::IntRect(465, 430, 25, 25), sf::Color::Black);

        handler.handleCollision(&car, car.getPosition(), car.getAngle(),
                                oldPos, car.getAngle(), 0.016f);

        REQUIRE(car.getCurrSpeed() == Catch::Approx(0.f));
        REQUIRE(car.getAngularVelocity() == Catch::Approx(0.f));
    }

    SECTION("Deep penetration (already colliding at oldPos) forces speed and angle to reset")
    {
        // oldPos is itself inside the wall region, so checkWallCollisions at
        // oldPos is already non-(-1) -> triggers the internal setAngle(oldAngle)
        // path, and the subsequent still-colliding safety check overrides
        // speed to exactly 0 regardless of damping math already computed.
        Track track;
        setTrackTestImage(track, 1000, 1000, sf::Color::White);
        paintTrackRegion(track, sf::IntRect(600, 0, 400, 1000), sf::Color::Black);

        std::vector<Car *> cars;
        CollisionHandler handler(track, cars);

        AI car(0.f, 0.f, 0.f, 200.f, 500.f, -50.f, 200.f);
        cars.push_back(&car);
        sf::Vector2f oldPos(800.f, 500.f); // deep inside the wall region
        car.setPosition(oldPos);
        car.setAngle(45.f);

        handler.handleCollision(&car, car.getPosition(), car.getAngle(), oldPos, 90.f, 0.016f);

        REQUIRE(car.getCurrSpeed() == Catch::Approx(0.f));
        REQUIRE(car.getAngle() == Catch::Approx(90.f)); // set directly inside handleCollisionResponse
    }
}
// ============================================================
// PART 4: CAR-CAR COLLISION RESPONSE (impulse, overlap, invincibility)
// ============================================================
TEST_CASE("CollisionHandler - car-car collision response", "[collision][response][car-car]")
{
    Track roadTrack;
    setTrackTestImage(roadTrack, 1000, 1000, sf::Color::White); // all-road, wall never interferes

    SECTION("Overlapping active cars are pushed apart and damped")
    {
        std::vector<Car *> cars;
        CollisionHandler handler(roadTrack, cars);

        AI car1(0.f, 0.f, 0.f, 50.f, 100.f, -50.f, 50.f);
        AI car2(0.f, 0.f, 0.f, 0.f, 100.f, -50.f, 50.f);
        car1.setActive(true);
        car2.setActive(true);
        cars.push_back(&car1);
        cars.push_back(&car2);

        car1.setPosition(sf::Vector2f(495.f, 500.f));
        car1.setAngle(90.f);                          // faces +x, i.e. directly toward car2 -> genuine closing velocity along the collision normal
        car2.setPosition(sf::Vector2f(505.f, 500.f)); // 10 units apart, well within combined half-widths (~46) -> real overlap
        car2.setAngle(90.f);

        sf::Vector2f oldPos1 = car1.getPosition();
        sf::Vector2f oldPos2 = car2.getPosition();
        float oldSpeed1 = car1.getCurrSpeed();

        bool flag = handler.handleCollision(&car1, car1.getPosition(), car1.getAngle(),
                                            car1.getPosition(), car1.getAngle(), 0.016f);

        REQUIRE(car1.getPosition() != oldPos1);
        REQUIRE(car2.getPosition() != oldPos2);
        REQUIRE(car1.getCurrSpeed() < oldSpeed1);
        REQUIRE(flag);
    }

    SECTION("Invincible car is skipped in car-car collision")
    {
        std::vector<Car *> cars;
        CollisionHandler handler(roadTrack, cars);

        AI car1(0.f, 0.f, 0.f, 50.f, 100.f, -50.f, 50.f);
        AI car2(0.f, 0.f, 0.f, 0.f, 100.f, -50.f, 50.f);
        car1.setActive(true);
        car2.setActive(true);
        cars.push_back(&car1);
        cars.push_back(&car2);

        car1.setPosition(sf::Vector2f(500.f, 500.f));
        car2.setPosition(sf::Vector2f(500.f, 500.f));
        car2.setITime(1.0f); // invincible -> skipped by handleCollision's own filter

        sf::Vector2f oldPos1 = car1.getPosition();
        float oldSpeed1 = car1.getCurrSpeed();

        bool collided = handler.handleCollision(&car1, car1.getPosition(), car1.getAngle(),
                                                car1.getPosition(), car1.getAngle(), 0.016f);
        REQUIRE_FALSE(collided);
        REQUIRE(car1.getPosition() == oldPos1);
        REQUIRE(car1.getCurrSpeed() == oldSpeed1);
    }

    SECTION("Inactive car is not treated as a collision candidate")
    {
        std::vector<Car *> cars;
        CollisionHandler handler(roadTrack, cars);

        AI car1(0.f, 0.f, 0.f, 50.f, 100.f, -50.f, 50.f);
        AI car2(0.f, 0.f, 0.f, 0.f, 100.f, -50.f, 50.f);
        car1.setActive(true);
        car2.setActive(false); // inactive -> skipped by handleCollision's own filter
        cars.push_back(&car1);
        cars.push_back(&car2);

        car1.setPosition(sf::Vector2f(500.f, 500.f));
        car2.setPosition(sf::Vector2f(500.f, 500.f));

        sf::Vector2f oldPos1 = car1.getPosition();
        bool collided = handler.handleCollision(&car1, car1.getPosition(), car1.getAngle(),
                                                car1.getPosition(), car1.getAngle(), 0.016f);
        REQUIRE_FALSE(collided);
        REQUIRE(car1.getPosition() == oldPos1);
    }
}