#pragma once
#include <vector>
#include "Car.h"
#include "Utils.h"

class Car;

class AIController
{
    AIConsts constants;
    Car *car;
    const std::vector<Waypoint> &waypoints;
    carInput horizontalInput = carInput::None;
    carInput verticalInput = carInput::None;
    TargetSide defaultSide;

    void calculateVerticalInput(const std::vector<Car *> &cars);
    void calculateHorizontalInput(const std::vector<Car *> &cars);

    void targetOvertake(const std::vector<Car *> &cars);

    float getOvertakeDetectionRadius();

    Car *getOvertakeTarget(const std::vector<Car *> &cars, float range);

public:
    AIController(Car *c, const std::vector<Waypoint> &w) : car(c), waypoints(w)
    {
        defaultSide = static_cast<TargetSide>(std::rand() % 3);
    }

    void update(const std::vector<Car *> &cars, float dt);

    carInput getHorizontalInput() const { return horizontalInput; }
    carInput getVerticalInput() const { return verticalInput; }

    ~AIController() = default;
};