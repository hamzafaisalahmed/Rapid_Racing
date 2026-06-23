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

    sf::Texture minimapTexture;
    sf::Sprite minimapSprite;
    const float minimapScale = 0.05f;

    sf::Sprite pause;
    sf::Texture pauseTexture;
    sf::FloatRect pauseButton;
    sf::FloatRect resetButton;
    std::vector<sf::FloatRect> pauseButtons;
    std::vector<sf::FloatRect> levelCompleteButtons;
    std::vector<sf::FloatRect> homeButtons;
    std::vector<sf::FloatRect> settingsButtons;
    std::vector<sf::FloatRect> resetButtonPVP;
    const sf::Vector2f homeBtnSize{240.f, 75.f};
    Track &track;
    Player &player;

    const std::vector<sf::Color> carColors = {
        // Extracted from your provided car textures
        sf::Color(255, 215, 0), // Gold (car1.png)
        sf::Color(220, 20, 60), // Race Red (car2.png)

        // 7 Additional Unique Colors
        sf::Color(0, 100, 255), // Electric Blue
        sf::Color(50, 205, 50), // Lime Green
        sf::Color(148, 0, 211), // Dark Violet
        sf::Color(255, 140, 0), // Dark Orange
        sf::Color(0, 255, 255), // Cyan
        sf::Color(255, 0, 255), // Magenta
        sf::Color(25, 25, 112)  // Midnight Blue
    };

public:
    Graphics(sf::RenderWindow &w, sf::View &v, sf::View &hud, Track &t, Player &p) : window(w), gameView(v), hudView(hud), track(t), player(p) {}

    void init();
    void drawTextCentered(const std::string &str, float x, float y, int size, sf::Color col);
    void renderHUD(float elapsed, int currentLap, int totalLaps, float currentLapTime, const LapTime &lapData);
    void renderHomeScreen();
    void renderLevelComplete(const LapTime &lapData, const std::vector<LapTime> &allTimes);
    void renderGamePlay(const std::vector<Car *> cars);
    void renderPauseScreen();
    void debugPlayDisplay(Car *car);
    const std::vector<sf::FloatRect> &getHomeButtons() const { return homeButtons; }
    const std::vector<sf::FloatRect> &getLevelCompleteButtons() const { return levelCompleteButtons; }
    const std::vector<sf::FloatRect> &getPauseScreenButtons() const { return pauseButtons; }
    const std::vector<sf::FloatRect> &getSettingsButtons() const { return settingsButtons; }
    const std::vector<sf::FloatRect> &getResetButtonsPVP() const { return resetButtonPVP; }

    const sf::FloatRect &getPauseButton() const { return pauseButton; }
    const sf::FloatRect &getResetButton() const { return resetButton; }
    void renderResetButton(Gamemode mode, bool p1 = false, bool p2 = false);
    void renderSettingsScreen(int currentLevel, int currentLaps, bool isMuted);
    void setCarColors(const std::vector<Car *> cars);
    void renderPVPGameplay(Player &player2);
    void renderPVPHUD(const Player &player2, int totalLaps, float totalRaceTime);
    void renderPVPLvlComplete(const Player &player2, const std::vector<Car *> &podium);
    void renderMinimap(const std::vector<Car *> &cars, Gamemode mode);
    void renderAILvlComplete(Car &player, LapTime &lapData, const std::vector<Car *> &podium);
    void renderAIHUD(const std::vector<Car *> &raceLeaderboard, int totalLaps, float elapsed);
};
