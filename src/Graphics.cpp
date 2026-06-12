#include "Graphics.h"

void Graphics::init()
{
    if (!homeTexture.loadFromFile("assets/textures/homescreen.png"))
        throw std::runtime_error("Home background texture not found");
    homeBackground.setTexture(homeTexture);
    if (!font.loadFromFile("assets/fonts/ProFontWindows.ttf"))
        throw std::runtime_error("Font not found");
    if (!pauseTexture.loadFromFile("assets/textures/pause.png"))
        throw std::runtime_error("Pause texture not found");
    pause.setTexture(pauseTexture);

    pauseButton = sf::FloatRect(1140.f, 15.f, 45.f, 45.f);
    pause.setPosition(pauseButton.left, pauseButton.top);
    float scaleX = pauseButton.width / pauseTexture.getSize().x;
    float scaleY = pauseButton.height / pauseTexture.getSize().y;
    pause.setScale(scaleX, scaleY);

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

    homeButtons.clear();
    float positionsY[] = {100.f, 275.f, 450.f, 625.f};
    float buttonX = 35.f;

    for (int i = 0; i < 4; ++i)
    {
        homeButtons.push_back(sf::FloatRect(buttonX, positionsY[i], homeBtnSize.x, homeBtnSize.y));
    }

    pauseButtons.clear();
    float btnWidth = 280.f;
    float btnHeight = 80.f;
    float centerX = (1200.f / 2.f) - (btnWidth / 2.f);

    pauseButtons.push_back(sf::FloatRect(centerX, 350.f, btnWidth, btnHeight)); // Continue
    pauseButtons.push_back(sf::FloatRect(centerX, 480.f, btnWidth, btnHeight)); // Exit
}
void Graphics::drawTextCentered(const std::string &str, float x, float y, int size, sf::Color col)
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
void Graphics::renderHUD(float elapsed, int currentLap, int totalLaps, float currentLapTime, const LapTime &lapData)
{
    // Switch to HUD view so elements stay locked to your 1200x800 window
    window.setView(hudView);

    // --- TELEMETRY CALCULATIONS ---
    int minutes = (int)(elapsed / 60.f);
    int seconds = (int)(elapsed) % 60;
    int millis = (int)((elapsed - (int)elapsed) * 100);

    // Format the time string nicely with leading zeros
    std::string timeStr = std::to_string(minutes) + ":" +
                          (seconds < 10 ? "0" : "") + std::to_string(seconds) + "." +
                          (millis < 10 ? "0" : "") + std::to_string(millis);

    int displaySpeed = (int)(std::abs(player.getCurrSpeed()));

    // ========================================================
    // 1. TOP-LEFT TIMING & LAP PODIUM
    // ========================================================

    // Background plate for the lap/timer stats so track textures don't hide the text
    sf::RectangleShape topPodium(sf::Vector2f(240.f, 75.f));
    topPodium.setPosition(15.f, 15.f);
    topPodium.setFillColor(sf::Color(30, 8, 10, 200)); // Deep dark burgundy translucent
    topPodium.setOutlineColor(sf::Color(140, 30, 40)); // Clean red border line
    topPodium.setOutlineThickness(1.5f);
    window.draw(topPodium);

    // Dynamic strings
    timerText.setString("TIME " + timeStr);
    lapText.setString("LAP  " + std::to_string(currentLap) + " / " + std::to_string(totalLaps));

    // Refined Styling
    timerText.setCharacterSize(16);
    timerText.setFillColor(sf::Color(255, 230, 230)); // Pinkish off-white digits
    timerText.setPosition(25.f, 23.f);

    lapText.setCharacterSize(16);
    lapText.setFillColor(sf::Color(255, 70, 80)); // Pops out in aggressive red
    lapText.setPosition(25.f, 48.f);

    window.draw(timerText);
    window.draw(lapText);

    // ========================================================
    // 2. BOTTOM-RIGHT RACING SPEEDOMETER POD
    // ========================================================

    // Positioned in the bottom right corner (Width: 1200, Height: 800)
    float speedX = 960.f;
    float speedY = 700.f;

    // Dark background banner for speed readings
    sf::RectangleShape speedPodium(sf::Vector2f(220.f, 85.f));
    speedPodium.setPosition(speedX, speedY);
    speedPodium.setFillColor(sf::Color(20, 5, 8, 220));
    speedPodium.setOutlineColor(sf::Color(210, 35, 45)); // High-visibility glowing red border
    speedPodium.setOutlineThickness(2.f);
    window.draw(speedPodium);

    // Subtle "KM/H" layout design
    speedometer.setString(std::to_string(displaySpeed));
    speedometer.setCharacterSize(45); // Made it massive for clear racing glance value
    speedometer.setFillColor(sf::Color::White);
    speedometer.setPosition(speedX + 20.f, speedY + 10.f);
    window.draw(speedometer);

    // Small decorative unit label next to or below the big numbers
    sf::Text unitText;
    unitText.setFont(*timerText.getFont()); // Re-use loaded font
    unitText.setString("KM/H");
    unitText.setCharacterSize(14);
    unitText.setFillColor(sf::Color(160, 160, 170));
    unitText.setPosition(speedX + 160.f, speedY + 45.f); // Pushed right alongside the numbers
    window.draw(unitText);

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

    // Just change the color tint based on hover state
    if (pauseButton.contains(mappedMousePos))
    {
        pause.setColor(sf::Color(255, 100, 100)); // Hover: Flash racing red
    }
    else
    {
        pause.setColor(sf::Color(255, 255, 255, 220)); // Normal: Semi-translucent white
    }

    // Draw the pre-configured member sprite
    window.draw(pause);

    // Switch back to viewport game view so camera physics tracking continues smoothly
    window.setView(gameView);
}

void Graphics::renderHomeScreen()
{
    window.setView(hudView);
    window.clear(sf::Color::Black);
    window.draw(homeBackground);

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

    // Red theme styles
    sf::Color standardFill(45, 10, 15, 230);
    sf::Color standardOutline(110, 30, 35);
    sf::Color standardText(255, 220, 220);

    sf::Color hoverFill(180, 25, 35);
    sf::Color hoverOutline(255, 60, 70);
    sf::Color hoverText(255, 255, 255);

    std::string menuItems[] = {"VS AI", "PVP", "TIME TRIAL", "SETTINGS"};
    unsigned int fontSizes[] = {18, 20, 18, 22};

    for (size_t i = 0; i < homeButtons.size(); ++i)
    {
        sf::FloatRect rect = homeButtons[i];

        sf::RectangleShape button(sf::Vector2f(rect.width, rect.height));
        button.setPosition(rect.left, rect.top);

        bool isHovered = rect.contains(mappedMousePos);

        if (isHovered)
        {
            button.setFillColor(hoverFill);
            button.setOutlineColor(hoverOutline);
            button.setOutlineThickness(3.f);
            window.draw(button);

            drawTextCentered(menuItems[i], rect.left + (rect.width / 2.f), rect.top + (rect.height / 2.f), fontSizes[i], hoverText);
        }
        else
        {
            button.setFillColor(standardFill);
            button.setOutlineColor(standardOutline);
            button.setOutlineThickness(2.f);
            window.draw(button);

            drawTextCentered(menuItems[i], rect.left + (rect.width / 2.f), rect.top + (rect.height / 2.f), fontSizes[i], standardText);
        }
    }
}

void Graphics::renderLevelComplete(const LapTime &lapData, const std::vector<LapTime> &allTimes)
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
    std::vector<LapTime> sortedTimes = allTimes;
    std::sort(sortedTimes.begin(), sortedTimes.end(),
              [](const LapTime &a, const LapTime &b)
              { return a.bestLap < b.bestLap; });

    std::string text = "GLOBAL TOP 3:\n\n";
    for (int i = 0; i < std::min(3, (int)sortedTimes.size()); i++)
    {
        float time = sortedTimes[i].bestLap;
        int m = (int)(time / 60.f);
        int s = (int)(time) % 60;
        int milli = (int)((time - (int)time) * 1000);

        text += std::to_string(i + 1) + ". ID:" + std::to_string(sortedTimes[i].id) + " | " +
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

void Graphics::renderGamePlay()
{
    sf::Vector2u trackSize = track.getSize();
    sf::Vector2f playerPos = player.getPosition();
    sf::Vector2f viewCenter = playerPos;
    float viewWidth = gameView.getSize().x;
    float viewHeight = gameView.getSize().y;
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
    debugPlayDisplay();
}

void Graphics::renderPauseScreen()
{
    window.setView(hudView);
    window.draw(pause); // Draw your pause texture overlay

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

    // Re-use your reddish theme colors
    sf::Color standardFill(45, 10, 15, 230);
    sf::Color hoverFill(180, 25, 35);
    sf::Color standardText(255, 220, 220);

    std::string labels[] = {"CONTINUE", "EXIT TO MENU"};

    for (size_t i = 0; i < pauseButtons.size(); ++i)
    {
        sf::FloatRect rect = pauseButtons[i];
        sf::RectangleShape button(sf::Vector2f(rect.width, rect.height));
        button.setPosition(rect.left, rect.top);

        bool isHovered = rect.contains(mappedMousePos);
        button.setFillColor(isHovered ? hoverFill : standardFill);
        button.setOutlineColor(isHovered ? sf::Color::White : sf::Color(110, 30, 35));
        button.setOutlineThickness(2.f);

        window.draw(button);
        drawTextCentered(labels[i], rect.left + rect.width / 2.f, rect.top + rect.height / 2.f, 22, isHovered ? sf::Color::White : standardText);
    }
}

void Graphics::debugPlayDisplay()
{
    for (auto &corner : player.getCorners(player.getPosition(), player.getAngle()))
    {
        sf::CircleShape dot(1.f);
        dot.setFillColor(sf::Color::Red);
        dot.setPosition(corner.x - 1.f, corner.y - 1.f);
        window.draw(dot);
    }
    std::vector<Waypoint> waypoints = track.getWaypoints();
    for (const auto &waypoint : waypoints)
    {
        sf::CircleShape dotL(1.f);
        dotL.setFillColor(sf::Color::Red);
        sf::CircleShape dotR(1.f);
        dotR.setFillColor(sf::Color::Green);
        dotL.setPosition(waypoint.left.x - 1.f, waypoint.left.y - 1.f);
        dotR.setPosition(waypoint.right.x - 1.f, waypoint.right.y - 1.f);
        window.draw(dotL);
        window.draw(dotR);
    }
}