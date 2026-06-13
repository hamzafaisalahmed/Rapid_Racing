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

    if (!minimapTexture.loadFromFile("assets/textures/track1mini.png"))
        throw std::runtime_error("Minimap texture not found");

    minimapSprite.setTexture(minimapTexture);
    minimapSprite.setScale(minimapScale, minimapScale);

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

    resetButton = sf::FloatRect(500.f, 650.f, 200.f, 50.f);

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

    settingsButtons.clear();

    // Define all 9 button positions visually in a single vector
    std::vector<sf::Vector2f> settingsPos = {
        {220.f, 220.f}, {480.f, 220.f}, {740.f, 220.f}, // Indices 0-2: Levels (1, 2, 3)
        {90.f, 400.f},
        {350.f, 400.f},
        {610.f, 400.f},
        {870.f, 400.f}, // Indices 3-6: Laps (1, 2, 3, 5)
        {480.f, 550.f}, // Index 7: Mute
        {480.f, 650.f}  // Index 8: Return to Menu
    };

    // Populate the boundaries using the standard home button size
    for (const auto &pos : settingsPos)
    {
        settingsButtons.push_back(sf::FloatRect(pos.x, pos.y, homeBtnSize.x, homeBtnSize.y));
    }
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
    std::string timeStr = formatRaceTime(elapsed);
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
        buttonNames = {"RESTART", "BACK TO MENU"};
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
    // sf::Vector2u trackSize = track.getSize();
    // gameView.setSize(static_cast<float>(trackSize.x), static_cast<float>(trackSize.y));
    // gameView.setCenter(trackSize.x / 2.f, trackSize.y / 2.f);
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

void Graphics::renderResetButton()
{
    // Switch to HUD space so it stays fixed on screen
    window.setView(hudView);

    // 1. Draw the background plate using the stored resetButton bounds
    sf::RectangleShape rect(sf::Vector2f(resetButton.width, resetButton.height));
    rect.setPosition(resetButton.left, resetButton.top);
    rect.setFillColor(sf::Color(30, 30, 30, 230)); // Clean dark gray background
    rect.setOutlineColor(sf::Color::Red);          // Red warning border
    rect.setOutlineThickness(2.f);
    window.draw(rect);

    // 2. Leverage your existing helper to handle the text creation and centering
    float centerX = resetButton.left + resetButton.width / 2.f;
    float centerY = resetButton.top + resetButton.height / 2.f;

    drawTextCentered("RESET CAR", centerX, centerY, 20, sf::Color::White);
}

void Graphics::renderSettingsScreen(int currentLevel, int currentLaps, bool isMuted)
{
    window.setView(hudView);
    window.clear(sf::Color(20, 20, 25));

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

    // Standard Theme Colors
    sf::Color standardFill(45, 10, 15, 230);
    sf::Color standardOutline(110, 30, 35);
    sf::Color activeFill(180, 25, 35);
    sf::Color hoverOutline(255, 60, 70);

    // Static Page Text
    drawTextCentered("SETTINGS", 600.f, 50.f, 45, sf::Color::White);
    drawTextCentered("CAR PERFORMANCE PRESET", 600.f, 170.f, 22, sf::Color(200, 200, 200));
    drawTextCentered("RACE LAPS", 600.f, 350.f, 22, sf::Color(200, 200, 200));

    // Parallel Arrays mapping exactly to the 9 settingsButtons indices
    std::string labels[] = {
        "LEVEL 1", "LEVEL 2", "LEVEL 3",
        "1 LAP", "2 LAPS", "3 LAPS", "5 LAPS",
        isMuted ? "AUDIO: MUTED" : "AUDIO: ON",
        "RETURN TO MENU"};

    // Evaluates all configuration conditions instantly without nested logic
    bool activeStates[] = {
        currentLevel == 0, currentLevel == 1, currentLevel == 2,
        currentLaps == 0, currentLaps == 1, currentLaps == 2, currentLaps == 3,
        isMuted,
        false // Return button is never "toggled" on
    };

    // The single, ultra-clean rendering loop
    for (size_t i = 0; i < settingsButtons.size(); ++i)
    {
        sf::FloatRect rect = settingsButtons[i];

        sf::RectangleShape button(homeBtnSize);
        button.setPosition(rect.left, rect.top);

        bool isHovered = rect.contains(mappedMousePos);
        bool isActive = activeStates[i];

        // Apply dynamic styling
        button.setFillColor(isActive ? activeFill : standardFill);

        if (isHovered)
        {
            button.setOutlineColor(hoverOutline);
            button.setOutlineThickness(3.f);
        }
        else
        {
            button.setOutlineColor(isActive ? sf::Color::White : standardOutline);
            button.setOutlineThickness(2.f);
        }

        window.draw(button);
        drawTextCentered(labels[i], rect.left + rect.width / 2.f, rect.top + rect.height / 2.f, 18, sf::Color::White);
    }
}

void Graphics::renderPVPGameplay(const Player &player2)
{
    sf::Vector2u trackSize = track.getSize();

    // Store players and viewport starting X-coordinates in arrays for iteration
    const Player *players[2] = {&player, &player2};
    float viewportsX[2] = {0.f, 0.5f};

    // Dynamic Clamping Math based on zoom level
    const float zoomFactor = 0.4f;
    const float halfW = 300.f * zoomFactor; // 120.f
    const float halfH = 400.f * zoomFactor; // 160.f

    // One loop handles both Player 1 (i=0) and Player 2 (i=1) perfectly
    for (int i = 0; i < 2; ++i)
    {
        sf::View view;
        view.setSize(600.f, 800.f);
        view.setViewport(sf::FloatRect(viewportsX[i], 0.f, 0.5f, 1.f));
        view.zoom(zoomFactor);

        sf::Vector2f pos = players[i]->getPosition();

        // Fixed Camera Clamping using the zoomed dimensions
        if (pos.x < halfW)
            pos.x = halfW;
        else if (pos.x > trackSize.x - halfW)
            pos.x = trackSize.x - halfW;

        if (pos.y < halfH)
            pos.y = halfH;
        else if (pos.y > trackSize.y - halfH)
            pos.y = trackSize.y - halfH;

        view.setCenter(pos);
        window.setView(view);

        // Draw the world for the current viewport
        track.draw(window, sf::VideoMode::getDesktopMode());
        player.draw(window);
        player2.draw(window);
    }
}

void Graphics::renderPVPHUD(const Player &player2, int totalLaps, float totalRaceTime)
{
    window.setView(hudView);

    // Standard Theme Colors
    sf::Color podiumFill(20, 5, 8, 220);
    sf::Color outline(210, 35, 45);

    // --- 1. CENTER SHARED DISPLAY (Time & Total Laps) ---
    sf::RectangleShape centerPanel(sf::Vector2f(260.f, 70.f));
    centerPanel.setPosition(470.f, 15.f); // Centered over the 600.f dividing line
    centerPanel.setFillColor(podiumFill);
    centerPanel.setOutlineColor(outline);
    centerPanel.setOutlineThickness(2.f);
    window.draw(centerPanel);

    // Formats the raw float time internally now
    drawTextCentered("TIME: " + formatRaceTime(totalRaceTime), 600.f, 35.f, 22, sf::Color::White);
    drawTextCentered("TOTAL LAPS: " + std::to_string(totalLaps), 600.f, 65.f, 16, sf::Color(200, 200, 200));

    // --- 2. CENTER DIVIDER ---
    sf::RectangleShape divider(sf::Vector2f(6.f, 715.f));
    divider.setPosition(597.f, 85.f);
    divider.setFillColor(sf::Color::White);
    divider.setOutlineColor(sf::Color::Black);
    divider.setOutlineThickness(2.f);
    window.draw(divider);

    // --- 3. INDIVIDUAL PLAYER STATS ---
    // Safely handles the +1 lap offset and pulls all data internally
    int laps[] = {player.getCurrLap() + 1, player2.getCurrLap() + 1};
    int positions[] = {player.getRacePos(), player2.getRacePos()};
    int speeds[] = {(int)std::abs(player.getCurrSpeed()), (int)std::abs(player2.getCurrSpeed())};
    float sideX[] = {15.f, 1005.f};

    for (int i = 0; i < 2; ++i)
    {
        // Top Boxes (Position & Current Lap)
        sf::RectangleShape topBox(sf::Vector2f(180.f, 60.f));
        topBox.setPosition(sideX[i], 15.f + 50.f);
        topBox.setFillColor(podiumFill);
        topBox.setOutlineColor(outline);
        topBox.setOutlineThickness(2.f);
        window.draw(topBox);

        std::string posStr = (positions[i] == 1) ? "1ST PLACE" : "2ND PLACE";
        float centerX = sideX[i] + 90.f;

        drawTextCentered(posStr, centerX, 32.f + 50.f, 18, sf::Color::White);
        drawTextCentered("LAP " + std::to_string(laps[i]), centerX, 58.f + 50.f, 16, sf::Color(200, 200, 200));

        // Bottom Boxes (Speedometer)
        sf::RectangleShape botBox(sf::Vector2f(180.f, 60.f));
        botBox.setPosition(sideX[i], 725.f);
        botBox.setFillColor(podiumFill);
        botBox.setOutlineColor(outline);
        botBox.setOutlineThickness(2.f);
        window.draw(botBox);

        drawTextCentered(std::to_string(speeds[i]) + " KM/H", centerX, 755.f, 26, sf::Color::White);
    }
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mp = window.mapPixelToCoords(mousePos, hudView);

    if (pauseButton.contains(mp))
    {
        pause.setColor(sf::Color(255, 100, 100)); // Flash red on hover
    }
    else
    {
        pause.setColor(sf::Color(255, 255, 255, 220)); // Semi-transparent white
    }

    window.draw(pause);
}

void Graphics::renderPVPLvlComplete(const Player &player2)
{
    window.setView(hudView);

    // 1. Dark overlay
    sf::RectangleShape overlay(sf::Vector2f(1200.f, 800.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 220));
    window.draw(overlay);

    // 2. Results Panel
    sf::RectangleShape panel(sf::Vector2f(600.f, 400.f));
    panel.setPosition(300.f, 200.f);
    panel.setFillColor(sf::Color(20, 5, 8, 240));
    panel.setOutlineColor(sf::Color(210, 35, 45));
    panel.setOutlineThickness(3.f);
    window.draw(panel);

    // 3. Header & Stats
    drawTextCentered("RACE COMPLETE!", 600.f, 250.f, 40, sf::Color::White);

    drawTextCentered("P1: " + std::to_string(player.getCurrLap() + 1) + " LAPS", 600.f, 320.f, 24, sf::Color(200, 200, 200));
    drawTextCentered("P2: " + std::to_string(player2.getCurrLap() + 1) + " LAPS", 600.f, 360.f, 24, sf::Color(200, 200, 200));

    // 4. Buttons (Matching your event handler index 0 = REMATCH, 1 = MENU)
    levelCompleteButtons.clear();
    std::string labels[] = {"REMATCH", "MENU"};

    for (int i = 0; i < 2; ++i)
    {
        // Position buttons matching your handler's expected layout
        float btnX = 400.f + (i * 250.f);
        float btnY = 520.f;
        sf::FloatRect btnRect(btnX, btnY, 200.f, 50.f);

        levelCompleteButtons.push_back(btnRect);

        sf::RectangleShape btn(sf::Vector2f(btnRect.width, btnRect.height));
        btn.setPosition(btnRect.left, btnRect.top);
        btn.setFillColor(sf::Color(45, 10, 15, 230));
        btn.setOutlineColor(sf::Color(110, 30, 35));
        btn.setOutlineThickness(2.f);

        window.draw(btn);
        drawTextCentered(labels[i], btnRect.left + btnRect.width / 2.f, btnRect.top + btnRect.height / 2.f, 18, sf::Color::White);
    }
}

void Graphics::renderMinimap(const std::vector<Car *> &cars, Gamemode mode)
{
    window.setView(hudView);

    float minimapWidth = minimapTexture.getSize().x * minimapScale;
    float minimapHeight = minimapTexture.getSize().y * minimapScale;

    // Position based on mode
    float minimapX = (mode == Gamemode::TimeTrial) ? (1200.f - minimapWidth - 10.f) : // Bottom-right for single player
                         ((1200.f / 2.f) - (minimapWidth / 2.f));                     // Bottom-center for multiplayer

    float minimapY = 800.f - minimapHeight - 10.f;
    minimapSprite.setPosition(minimapX, minimapY);

    // Background box
    sf::RectangleShape minimapBox(sf::Vector2f(minimapWidth + 20.f, minimapHeight + 20.f));
    minimapBox.setPosition(minimapX - 10.f, minimapY - 10.f);
    minimapBox.setFillColor(sf::Color(20, 5, 8, 220));
    minimapBox.setOutlineColor(sf::Color(210, 35, 45));
    minimapBox.setOutlineThickness(2.f);
    window.draw(minimapBox);
    window.draw(minimapSprite);

    // Draw cars
    sf::Color carColors[] = {sf::Color::Cyan, sf::Color::Magenta, sf::Color::Yellow};
    for (size_t i = 0; i < cars.size(); ++i)
    {
        if (!cars[i]->getActive())
            continue;
        sf::CircleShape dot(4.f);
        dot.setFillColor(carColors[i % 3]);

        sf::Vector2f minimapPos = cars[i]->getPosition() * minimapScale;
        minimapPos.x += minimapX;
        minimapPos.y += minimapY;

        dot.setPosition(minimapPos);
        window.draw(dot);
    }
}