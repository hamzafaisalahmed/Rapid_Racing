#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Utils.h"
#include "Car.h"
#include "Track.h"

class Graphics
{
    sf::Font font;
    sf::RenderWindow &window;
    sf::View &gameView;
    sf::View &hudView;

    sf::Text timerText;
    sf::Text lapText;
    sf::Text speedometer;

    sf::Sprite homeBackground;
    sf::Texture homeTexture;

    sf::Sprite pause;
    sf::Texture pauseTexture;
    sf::FloatRect pauseButton;
    std::vector<sf::FloatRect> pauseButtons;
    std::vector<sf::FloatRect> levelCompleteButtons;
    std::vector<sf::FloatRect> homeButtons;
    const sf::Vector2f homeBtnSize{240.f, 75.f};
    Track &track;
    Player &player;

public:
    Graphics(sf::RenderWindow &w, sf::View &v, sf::View &hud, Track &t, Player &p) : window(w), gameView(v), hudView(hud), track(t), player(p) {}

    void init();
    void drawTextCentered(const std::string &str, float x, float y, int size, sf::Color col);
    void renderHUD(float elapsed, int currentLap, int totalLaps, float currentLapTime, const LapTime &lapData);
    void renderHomeScreen();
    void renderLevelComplete(const LapTime &lapData, const std::vector<LapTime> &allTimes);
    void renderGamePlay();
    void renderPauseScreen();
    const std::vector<sf::FloatRect> &getHomeButtons() const { return homeButtons; }
    const std::vector<sf::FloatRect> &getLevelCompleteButtons() const { return levelCompleteButtons; }
    const std::vector<sf::FloatRect> &getPauseScreenButtons() const { return pauseButtons; }
    const sf::FloatRect &getPauseButton() const { return pauseButton; }
};
