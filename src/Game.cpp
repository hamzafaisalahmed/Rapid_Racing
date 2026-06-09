#include "Game.h"
#include "Utils.h"
#include <algorithm>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <stdexcept>
#include <fstream>

void Game::init()
{
    std::srand((unsigned)std::time(0));

    window.create(sf::VideoMode(1200, 800), "Racing", sf::Style::Default);
    window.setFramerateLimit(60);

    // Load track first
    track.LoadTrack("assets/textures/track1.png");
    // Load player
    player.load("assets/textures/car1.png");

    if (!font.loadFromFile("assets/fonts/ProFontWindows.ttf"))
        throw std::runtime_error("Font not found");

    timerText.setFont(font);
    timerText.setCharacterSize(16);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition(10.f, 10.f);

    lapText.setFont(font);
    lapText.setCharacterSize(20);
    lapText.setFillColor(sf::Color::Red);
    lapText.setPosition(10.f, 30.f);

    speedometer.setFont(font);
    speedometer.setCharacterSize(35);
    speedometer.setFillColor(sf::Color::White);
    speedometer.setPosition(10.f, 700.f);

    if (!engineAudio.openFromFile("assets/audio/engine.ogg"))
        throw std::runtime_error("Engine sound not found\n");
    engineAudio.setLoop(true);

    if (!endscreen.openFromFile("assets/audio/endscreen.ogg"))
        throw std::runtime_error("Endscreen music file not found\n");
    endscreen.setLoop(true);
    endscreen.setVolume(70.f);

    resetLevel();

    hudView = sf::View(sf::FloatRect(0.f, 0.f, 1200.f, 800.f));

    // Set up game view to show a reasonable portion of the track
    // View should be smaller than track to allow zooming on player
    gameView = sf::View(sf::FloatRect(0.f, 0.f, 300.f, 200.f));
    gameView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    window.setView(gameView);
    stateStack.push(GameState::Playing);
}

void Game::run()
{
    sf::Clock clock;
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f)
            dt = 0.05f;
        handleEvents();
        GameState current = stateStack.top();
        if (current == GameState::Playing)
        {
            update(dt);
        }
        render();
        window.display();
    }
}

void Game::handleEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            if (lapData.laps == totalLaps)
                saveLapTime(lapData);
            engineAudio.stop();
            endscreen.stop();
            window.close();
        }
        if (event.type == sf::Event::MouseButtonPressed)
        {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            if (stateStack.top() == GameState::LevelComplete)
            {
                if (levelCompleteButtons[0].contains(mousePos))
                {
                    saveLapTime(lapData);
                    lapData = LapTime();
                    resetLevel();
                    stateStack.pop();
                }
                else if (levelCompleteButtons[1].contains(mousePos))
                {
                    saveLapTime(lapData);
                    endscreen.stop();
                    window.close();
                }
            }
        }
    }
}

void Game::resetLevel()
{
    player.setPosition(sf::Vector2f(1620.f, 2550.f));
    player.setAngle(90.f);
    player.setCurrSpeed(0.f);
    player.setMaxSpeed(300.f);
    player.setAcc(50.f);
    player.setMaxReverseSpeed(-100.f);

    currentLap = 0;
    totalLaps = 2;
    currentLapTime = 0.f;
    track.resetCooldown();
    raceTimer.restart();
    endscreen.stop();
    engineAudio.setVolume(0.f);
    engineAudio.play();
}
void Game::handlePlayerMovement(float dt)
{
    if (window.hasFocus())
    {
        sf::Vector2f oldPosition = player.getPosition();
        float angle = player.getAngle();
        float oldAngle = angle;
        float turnFactor = 0.f;
        bool dec = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            turnFactor = -1.f * standardTurnFactor;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            turnFactor = standardTurnFactor;
        }
        angle += turnFactor * player.getTurnSpeed() * dt;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            player.accelerate(dt);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            player.decelerate(dt);
            dec = true;
        }
        else
        {
            dec = true;
            if (std::abs(player.getCurrSpeed()) < 10.f)
                player.setCurrSpeed(0.f);
            else if (std::abs(player.getCurrSpeed()) < 60.f)
                player.setCurrSpeed(player.getCurrSpeed() * track.getFriction() * 0.985f);
            else
                player.setCurrSpeed(player.getCurrSpeed() * track.getFriction());
        }

        float speed = player.getCurrSpeed();
        sf::Vector2f position = player.getPosition();
        position.x += std::cos((angle - 90.f) * (3.14159f / 180.f)) * speed * dt;
        position.y += std::sin((angle - 90.f) * (3.14159f / 180.f)) * speed * dt;
        if (checkCollisions(position, angle))
        {
            player.setPosition(oldPosition);
            player.setCurrSpeed(speed * -0.3f);
            player.setAngle(oldAngle);
        }
        else
        {
            player.setPosition(position);
            player.setCurrSpeed(speed);
            player.setAngle(lerp(oldAngle, angle, 0.6f));
        }
        float targetVolume = 0;
        if (dec)
            targetVolume = 0;
        else
            targetVolume = std::abs(player.getCurrSpeed() / player.getMaxSpeed()) * 100.f;
        engineAudio.setVolume(lerp(engineAudio.getVolume(), targetVolume, 10.f * dt));
    }
}

void Game::update(float dt)
{
    if (stateStack.top() == GameState::Playing)
    {
        handlePlayerMovement(dt);
        currentLapTime += dt;
        if (track.isFinishLine(player.getCorners(player.getPosition(), player.getAngle()), dt))
        {
            currentLap++;
            if (currentLap > 1)
            {
                lapData.totalTime += currentLapTime;
                if (currentLapTime < lapData.bestLap)
                    lapData.bestLap = currentLapTime;
                currentLapTime = 0.0f;
                lapData.laps++;
            }

            if (currentLap > totalLaps)
            {
                engineAudio.pause();
                endscreen.play();
                stateStack.push(GameState::LevelComplete);
            }
        }
    }
    if (stateStack.top() == GameState::LevelComplete)
    {
    }
}

void Game::render()
{
    window.clear(sf::Color::Black);
    if (stateStack.top() == GameState::Playing)
    {
        renderGamePlay();
    }
    else if (stateStack.top() == GameState::LevelComplete)
    {
        renderLevelComplete();
    }
}

void Game::renderGamePlay()
{
    // Get track dimensions
    sf::Vector2u trackSize = track.getSize();

    // Get player position
    sf::Vector2f playerPos = player.getPosition();

    // Center view on player, constrained within track bounds
    sf::Vector2f viewCenter = playerPos;

    // Constrain view to not go out of track bounds
    float viewWidth = gameView.getSize().x;
    float viewHeight = gameView.getSize().y;

    // Keep view centered on player, but within track bounds
    if (viewCenter.x - viewWidth / 2.f < 0.f)
        viewCenter.x = viewWidth / 2.f;
    if (viewCenter.x + viewWidth / 2.f > trackSize.x)
        viewCenter.x = trackSize.x - viewWidth / 2.f;
    if (viewCenter.y - viewHeight / 2.f < 0.f)
        viewCenter.y = viewHeight / 2.f;
    if (viewCenter.y + viewHeight / 2.f > trackSize.y)
        viewCenter.y = trackSize.y - viewHeight / 2.f;

    gameView.setCenter(viewCenter);
    window.setView(gameView);

    track.draw(window, sf::VideoMode::getDesktopMode());
    player.draw(window);
    renderHUD();
}

void Game::renderHUD()
{
    // switch to HUD view so text stays fixed on screen
    window.setView(hudView);

    float elapsed = raceTimer.getElapsedTime().asSeconds();
    int minutes = (int)(elapsed / 60.f);
    int seconds = (int)(elapsed) % 60;
    int millis = (int)((elapsed - (int)elapsed) * 100);

    timerText.setString("Time: " + std::to_string(minutes) + ":" +
                        (seconds < 10 ? "0" : "") + std::to_string(seconds) + "." +
                        (millis < 10 ? "0" : "") + std::to_string(millis));

    lapText.setString("Lap: " + std::to_string(currentLap) + "/" + std::to_string(totalLaps));

    window.draw(timerText);
    window.draw(lapText);

    int displaySpeed = (int)(std::abs(player.getCurrSpeed()));
    speedometer.setString("Speed: " + std::to_string(displaySpeed) + " km/h");
    window.draw(speedometer);
    // switch back to game view
    window.setView(gameView);
}
bool Game::checkCollisions(sf::Vector2f pos, float angle)
{
    for (auto &corner : player.getCorners(pos, angle))
        if (!track.isOnRoad(corner))
        {
            return true;
        }

    return false;
}

void Game::saveLapTime(const LapTime &lt)
{
    std::ofstream file("scores.txt", std::ios::app);
    if (file.is_open())
    {
        file << lt.id << " " << lt.laps << " "
             << lt.bestLap << " " << lt.totalTime << "\n";
        file.close();
    }
}

std::vector<LapTime> Game::loadLapTimes()
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

void Game::renderLevelComplete()
{
    window.setView(window.getDefaultView());
    window.clear(sf::Color::Black);

    sf::Text title;
    drawTextCentered("RACE COMPLETE!", 600.f, 60.f, 50, sf::Color::White);

    float fastest = lapData.bestLap;
    int mins = (int)(fastest / 60.f);
    int secs = (int)fastest % 60;
    int ms = (int)((fastest - (int)fastest) * 1000);

    std::string lapTime = "Your ID: " + std::to_string(lapData.id) + " | Best Lap: " + std::to_string(mins) + ":" +
                          (secs < 10 ? "0" : "") + std::to_string(secs) + "." +
                          (ms < 100 ? "0" : "") + (ms < 10 ? "0" : "") + std::to_string(ms);
    drawTextCentered(lapTime, 600.f, 150.f, 30, sf::Color::White);

    // Global top 3
    std::vector<LapTime> allTimes = loadLapTimes();
    std::sort(allTimes.begin(), allTimes.end(),
              [](const LapTime &a, const LapTime &b)
              { return a.bestLap < b.bestLap; });

    std::string text = "GLOBAL TOP 3:\n\n";
    for (int i = 0; i < std::min(3, (int)allTimes.size()); i++)
    {
        float time = allTimes[i].bestLap;
        int m = (int)(time / 60.f);
        int s = (int)(time) % 60;
        int milli = (int)((time - (int)time) * 1000);

        text += std::to_string(i + 1) + ". ID:" + std::to_string(allTimes[i].id) + " | " +
                std::to_string(m) + ":" +
                (s < 10 ? "0" : "") + std::to_string(s) + "." +
                (milli < 100 ? "0" : "") + (milli < 10 ? "0" : "") + std::to_string(milli) + "\n\n";
    }

    drawTextCentered(text, 600.f, 300.f, 20, sf::Color::White);

    // Buttons
    std::vector<std::string>
        buttonNames = {"RESTART", "EXIT"};
    levelCompleteButtons.clear();

    for (int i = 0; i < 2; i++)
    {
        float buttonX = 300.f + (i * 360.f);
        float buttonY = 550.f;

        sf::RectangleShape button(sf::Vector2f(120.f, 40.f));
        button.setPosition(buttonX, buttonY);
        button.setFillColor(sf::Color::White);
        window.draw(button);
        levelCompleteButtons.push_back(button.getGlobalBounds());

        drawTextCentered(buttonNames[i], buttonX + 60.f, buttonY + 20.f, 16, sf::Color::Black);
    }
}

void Game::drawTextCentered(const std::string &str, float x, float y, int size, sf::Color col)
{
    sf::Text text;
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(col);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    text.setPosition(x, y);
    window.draw(text);
}