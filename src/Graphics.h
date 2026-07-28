#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Utils.h"
#include "Car.h"
#include "Track.h"
#include "WaypointHandler.h"
#include "AIController.h"
#include <algorithm>

class Graphics
{
    sf::Font font;
    sf::RenderWindow &window;
    sf::View &gameView;
    sf::View &hudView;

    sf::Text timerText;
    sf::Text speedometer;

    sf::Sprite homeBackground;
    sf::Texture homeTexture;

    sf::Texture minimapTexture;
    sf::Sprite minimapSprite;
    const float minimapScale = 0.05f;

    sf::Color standardFill;
    sf::Color standardOutline;
    sf::Color standardText;
    sf::Color hoverFill;
    sf::Color hoverOutline;
    sf::Color hoverText;

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
    const std::vector<sf::Color> carColors = {
        // Extracted from your provided car textures
        sf::Color(198, 136, 1), // Gold (car1.png)
        sf::Color(151, 0, 30),  // Race Red (car2.png)

        // 7 AI colors
        sf::Color(151, 0, 30),    // red again
        sf::Color(5, 8, 171),     // blue
        sf::Color(13, 96, 6),     // green
        sf::Color(0, 0, 0),       // black
        sf::Color(255, 255, 255), // white
        sf::Color(102, 29, 87),   // purple
        sf::Color(54, 60, 80)     // idk silver blue
    };
    std::vector<sf::FloatRect> aiSetupButtons;
    std::vector<sf::FloatRect> trackSelectButtons;
    std::vector<sf::Texture> trackTextures;
    std::vector<sf::Sprite> trackSprites;

public:
    Graphics(sf::RenderWindow &w, sf::View &v, sf::View &hud, Track &t) : window(w), gameView(v), hudView(hud), track(t) {}

    void init();
    void drawTextCentered(const std::string &str, float x, float y, int size, sf::Color col);
    void renderTTHUD(Player &player, float elapsed, int currentLap, int totalLaps);
    void renderHomeScreen();
    void renderLevelComplete(const LapTime &lapData, const std::vector<LapTime> &allTimes);
    // void renderGamePlay(const std::vector<Car *> cars);
    void loadMinimap();
    void renderPauseScreen();
    void debugPlayDisplay(Car *car);
    const std::vector<sf::FloatRect> &getHomeButtons() const { return homeButtons; }
    const std::vector<sf::FloatRect> &getLevelCompleteButtons() const { return levelCompleteButtons; }
    const std::vector<sf::FloatRect> &getPauseScreenButtons() const { return pauseButtons; }
    const std::vector<sf::FloatRect> &getSettingsButtons() const { return settingsButtons; }
    const std::vector<sf::FloatRect> &getResetButtonsPVP() const { return resetButtonPVP; }
    const std::vector<sf::FloatRect> &getAISetupButtons() const { return aiSetupButtons; }

    const sf::FloatRect &getPauseButton() const { return pauseButton; }
    const sf::FloatRect &getResetButton() const { return resetButton; }
    void renderResetButton(Gamemode mode, bool p1 = false, bool p2 = false);
    void setCarColors(const std::vector<Car *> cars);
    void renderPVPGameplay(Player &player1, Player &player2, bool debugDisplay);
    void renderPVPHUD(const Player &player1, const Player &player2, int totalLaps, float totalRaceTime);
    void renderPVPLvlComplete(const Player &player1, const Player &player2, const std::vector<Car *> &podium);
    void renderMinimap(const std::vector<Car *> &cars, Gamemode mode);
    void renderAILvlComplete(Car &player, const LapTime &lapData, const std::vector<Car *> &podium);
    void renderAIHUD(Player &player, const std::vector<Car *> &raceLeaderboard, int totalLaps, float elapsed, bool spectatorMode = false);
    void renderLevelCompleteButtons();
    void renderCountdown(float countdownTimer);

    void debugAITarget(const std::vector<Car *> &cars);

    void debugWaypointAI(const WaypointHandler &wpHandler, const std::vector<Waypoint> &waypoints);

    void renderSettingsScreen(int currentLevel, int currentLaps, bool isMuted, bool debugDisplay);
    void renderGamePlay(const std::vector<Car *> cars, Car *focusCar, bool debugDisplay);

    void renderAISetupScreen(int currentAiCount, bool isSpectator, int currentDifficulty);

    void initTrackSelect(const std::vector<std::string> &trackPaths);
    void renderTrackSelectScreen(int currentSelectedTrackIndex);
    const std::vector<sf::FloatRect> &getTrackSelectButtons() const { return trackSelectButtons; }
};
