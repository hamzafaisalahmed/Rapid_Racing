#include "Car.h"
#include "AIController.h"
#include <iostream>
#include <stdexcept>
#include <cmath>

Car::Car(float xp, float yp, float a, float s, float ms, float mrs, float ac)
    : position(xp, yp),
      angle(a),
      speed(s),
      maxSpeed(ms),
      maxReverseSpeed(mrs),
      acc(ac),
      maxTurnSpeed(ms * 0.93f),
      currWaypointIndex(0),
      currLap(0),
      racePos(0),
      stuckTime(0.f),
      isActive(false),
      iTime(0.f)
{
}

void Car::setCollisionFunction(std::function<bool(Car *, sf::Vector2f, float, sf::Vector2f, float, float)> cc)
{
    collisionChecker = cc;
}

void Car::load(const std::string &dir)
{
    try
    {
        if (!texture.loadFromFile(dir))
        {
            throw std::runtime_error("Texture not found");
        }
        sprite.setTexture(texture);
        sprite.setScale(0.08f, 0.08f);
        sprite.setOrigin(texture.getSize().x / 2.f, texture.getSize().y * 0.65f);
        sprite.setPosition(position);
        dimensions = sf::Vector2f(
            texture.getSize().x * 0.08f,
            texture.getSize().y * 0.08f);
    }
    catch (const std::runtime_error &e)
    {
        std::cout << "Texture unable to load..." << e.what() << std::endl;
        throw;
    }
}

sf::Vector2f Car::getDimensions() const
{
    return dimensions;
}

std::vector<sf::Vector2f> Car::getCorners(sf::Vector2f pos, float angle)
{
    sf::Transform t;
    t.translate(pos);
    t.rotate(angle);

    sf::FloatRect bounds = sprite.getLocalBounds();
    float w = (bounds.width * sprite.getScale().x) / 2.2f;
    float hr = (bounds.height * sprite.getScale().y) * 0.3f;
    float hf = (bounds.height * sprite.getScale().y) * 0.6f;

    return {
        t.transformPoint(-w, -hf),
        t.transformPoint(w, -hf),
        t.transformPoint(-w, hr),
        t.transformPoint(w, hr)};
}

std::vector<sf::Vector2f> Car::getCorners()
{
    sf::Transform t;
    t.translate(position);
    t.rotate(angle);

    sf::FloatRect bounds = sprite.getLocalBounds();
    float w = (bounds.width * sprite.getScale().x) / 2.2f;
    float hf = (bounds.height * sprite.getScale().y) * 0.3f;
    float hr = (bounds.height * sprite.getScale().y) * 0.6f;

    return {
        t.transformPoint(-w, hf),
        t.transformPoint(w, hf),
        t.transformPoint(-w, -hr),
        t.transformPoint(w, -hr)};
}

void Car::accelerate(float dt)
{
    speed += acc * dt;
    if (speed > maxSpeed)
        speed = maxSpeed;
}

void Car::decelerate(float dt)
{
    speed -= acc * 2 * dt;
    if (speed < maxReverseSpeed)
        speed = maxReverseSpeed;
}

void Car::draw(sf::RenderWindow &window) const
{
    window.draw(sprite);
}

sf::Vector2f Car::getPosition() const
{
    return position;
}

void Car::setPosition(sf::Vector2f pos)
{
    position = pos;
    sprite.setPosition(position);
}

float Car::getAngle() const
{
    return angle;
}

float Car::getMaxSpeed() const
{
    return maxSpeed;
}

float Car::getCurrSpeed() const
{
    return speed;
}

void Car::setMaxSpeed(float ms)
{
    maxSpeed = ms;
    maxTurnSpeed = ms * 0.9f;
}

void Car::setAcc(float a)
{
    acc = a;
}

float Car::getAcc() const
{
    return acc;
}

void Car::setAngle(float a)
{
    angle = a;
    sprite.setRotation(angle);
}

void Car::setCurrSpeed(float s)
{
    speed = s;
}

void Car::setMaxReverseSpeed(float mrs)
{
    maxReverseSpeed = mrs;
}

float Car::getMaxReverseSpeed() const
{
    return maxReverseSpeed;
}

float Car::getMaxTurnSpeed() const
{
    return maxTurnSpeed;
}

float Car::getTurnFactor() const
{
    if (std::abs(speed) < 1.f)
        return 0.f;

    float ratio = 1 - (std::abs(speed) / maxTurnSpeed);
    if (ratio >= 0.8f)
        return 0.8f;
    if (ratio <= 0.3f)
        return 0.3f;
    return ratio;
}

float Car::handleMovement(float dt, carInput xIn, carInput yIn, float friction)
{
    sf::Vector2f oldPosition = getPosition();
    float angle = getAngle();
    float oldAngle = angle;
    float turnFactor = 0.f;
    float targetVolume = 0.f;

    if (xIn == carInput::Left)
    {
        turnFactor = -1.f * standardTurnFactor;
    }
    else if (xIn == carInput::Right)
    {
        turnFactor = standardTurnFactor;
    }

    addTurnAngularVelocity(turnFactor * getTurnFactor());
    angle += turnAngularVelocity * dt;
    turnAngularVelocity *= 0.9f;

    if (yIn == carInput::Up)
    {
        accelerate(dt);
        targetVolume = std::abs(getCurrSpeed() / getMaxSpeed()) * 100.f;
    }
    else if (yIn == carInput::Down)
    {
        decelerate(dt);
    }
    else
    {
        if (std::abs(getCurrSpeed()) < 10.f)
            setCurrSpeed(0.f);
        else if (std::abs(getCurrSpeed()) < 60.f)
            setCurrSpeed(getCurrSpeed() * friction * 0.985f);
        else
            setCurrSpeed(getCurrSpeed() * friction);
    }

    float speed = getCurrSpeed();
    sf::Vector2f position = getPosition();
    position.x += std::cos((angle - 90.f) * (3.14159f / 180.f)) * speed * dt;
    position.y += std::sin((angle - 90.f) * (3.14159f / 180.f)) * speed * dt;

    if (!collisionChecker(this, position, angle, oldPosition, oldAngle, dt))
    {
        setPosition(position);
        setCurrSpeed(speed);
    }

    angle += angularVelocity * dt;
    angularVelocity *= 0.95f;
    if (std::abs(angularVelocity) < 1.f)
        angularVelocity = 0.f;

    setAngle(lerp(oldAngle, angle, 0.6f));
    return targetVolume;
}

bool Car::updateWaypoint(const std::vector<Waypoint> &waypoints)
{
    if (distance(waypoints[currWaypointIndex].mid, getPosition()) < (69.f * 69.f))
    {
        currWaypointIndex++;
        if ((size_t)currWaypointIndex >= waypoints.size())
        {
            currWaypointIndex = 0;
            return true;
        }
    }
    return false;
}

int Car::getCurrWaypointIndex() const
{
    return currWaypointIndex;
}

void Car::resetWaypointIndex()
{
    currWaypointIndex = 0;
}

int Car::getCurrLap() const
{
    return currLap;
}

void Car::setCurrLap(int c)
{
    currLap = c;
}

void Car::updateStuckTime(float dt)
{
    if (std::abs(getCurrSpeed()) < 10.f)
        stuckTime += dt;
    else
        stuckTime = 0.f;
}

bool Car::isStuck() const
{
    return stuckTime > maxStuckTime;
}

void Car::resetPosition(const std::vector<Waypoint> &waypoints)
{
    if (!isStuck())
        return;

    int index = 0;
    if (currWaypointIndex != 0)
        index = currWaypointIndex - 1;

    setPosition(waypoints[index].mid);
    setCurrSpeed(0.f);
    setAngularVelocity(0.f);
    stuckTime = 0.f;
    setITime();
}

void Car::incrementLaps()
{
    currLap++;
}

int Car::getRacePos() const
{
    return racePos;
}

void Car::setRacePos(int i)
{
    racePos = i;
}

bool Car::getActive() const
{
    return isActive;
}

void Car::setActive(bool b)
{
    isActive = b;
}

void Car::setITime(float duration)
{
    iTime = duration;
}

void Car::updateITime(float dt)
{
    if (iTime > 0.f)
        iTime -= dt;
}

float Car::isInvincible() const
{
    return iTime > 0.f;
}

float Car::getITime() const
{
    return iTime;
}

void Car::setAngularVelocity(float a)
{
    angularVelocity = a;
}

float Car::getAngularVelocity() const
{
    return angularVelocity;
}

void Car::addTurnAngularVelocity(float a)
{
    turnAngularVelocity = clamp(a, -maxTurnAngularVelocity, maxTurnAngularVelocity);
}

sf::Vector2f Car::getDirectionVector() const
{
    float rad = (angle - 90.f) * (3.14159f / 180.f);
    return sf::Vector2f(std::cos(rad), std::sin(rad));
}

sf::Vector2f Car::getPerpendicularVector() const
{
    sf::Vector2f dir = getDirectionVector();
    return sf::Vector2f(-dir.y, dir.x);
}

Player::Player(float xp, float yp, float a, float s, float ms, float mrs, float ac)
    : Car(xp, yp, a, s, ms, mrs, ac)
{
}

Player::Player()
    : Car(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f)
{
}

AI::AI(float xp, float yp, float a, float s, float ms, float mrs, float ac)
    : Car(xp, yp, a, s, ms, mrs, ac)
{
}

AI::AI()
    : Car(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f)
{
}

float AI::handleAIMovement(float dt, float friction)
{
    if (!aiController)
    {
        std::cerr << "ERROR: aiController is null\n";
        return 0.f;
    }
    return Car::handleMovement(dt, aiController->getHorizontalInput(), aiController->getVerticalInput(), friction);
}
