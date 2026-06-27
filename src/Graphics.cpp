#include "Graphics.h"

void Graphics::init()
{
    if (!homeTexture.loadFromFile("assets/textures/homescreen.png"))
        throw std::runtime_error("Home background texture not found");
    homeBackground.setTexture(homeTexture);
    //=====================================================================================
    if (!font.loadFromFile("assets/fonts/ProFontWindows.ttf"))
        throw std::runtime_error("Font not found");
    if (!pauseTexture.loadFromFile("assets/textures/pause.png"))
        throw std::runtime_error("Pause texture not found");
    pause.setTexture(pauseTexture);
    //=====================================================================================
    if (!minimapTexture.loadFromFile("assets/textures/track1mini.png"))
        throw std::runtime_error("Minimap texture not found");

    minimapSprite.setTexture(minimapTexture);
    minimapSprite.setScale(minimapScale, minimapScale);
    //=====================================================================================
    pauseButton = sf::FloatRect(1140.f, 15.f, 45.f, 45.f);
    pause.setPosition(pauseButton.left, pauseButton.top);
    float scaleX = pauseButton.width / pauseTexture.getSize().x;
    float scaleY = pauseButton.height / pauseTexture.getSize().y;
    pause.setScale(scaleX, scaleY);
    //=====================================================================================
    timerText.setFont(font);
    timerText.setCharacterSize(16);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition(10.f, 10.f);
    //=====================================================================================
    speedometer.setFont(font);
    speedometer.setCharacterSize(35);
    speedometer.setFillColor(sf::Color::White);
    speedometer.setPosition(10.f, 700.f);
    //=====================================================================================
    resetButton = sf::FloatRect(500.f, 650.f, 200.f, 50.f);

    resetButtonPVP.clear();

    // Player 1 Reset
    resetButtonPVP.push_back(sf::FloatRect(15.f, 660.f, 180.f, 50.f));

    // Player 2 Reset
    resetButtonPVP.push_back(sf::FloatRect(1005.f, 660.f, 180.f, 50.f));
    //=====================================================================================
    homeButtons.clear();
    float positionsY[] = {100.f, 275.f, 450.f, 625.f};
    float buttonX = 35.f;

    for (int i = 0; i < 4; ++i)
    {
        homeButtons.push_back(sf::FloatRect(buttonX, positionsY[i], homeBtnSize.x, homeBtnSize.y));
    }
    //=====================================================================================
    pauseButtons.clear();
    float btnWidth = 280.f;
    float btnHeight = 80.f;
    float centerX = (1200.f / 2.f) - (btnWidth / 2.f);

    pauseButtons.push_back(sf::FloatRect(centerX, 350.f, btnWidth, btnHeight)); // Continue
    pauseButtons.push_back(sf::FloatRect(centerX, 480.f, btnWidth, btnHeight)); // Exit
    //=====================================================================================
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
    // ========================================================
    // LEVEL COMPLETE BUTTONS
    // ========================================================
    levelCompleteButtons.clear();
    for (int i = 0; i < 2; ++i)
    {
        float btnX = 400.f + (i * 250.f);
        levelCompleteButtons.push_back(sf::FloatRect(btnX, 520.f, 200.f, 50.f));
    }
    //=====================================================================================
    standardFill = sf::Color(45, 10, 15, 230);
    standardOutline = sf::Color(110, 30, 35);
    standardText = sf::Color(255, 220, 220);
    hoverFill = sf::Color(180, 25, 35);
    hoverOutline = sf::Color(255, 60, 70);
    hoverText = sf::Color(255, 255, 255);
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
void Graphics::renderHUD(float elapsed, int currentLap, int totalLaps)
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
    drawTextCentered("LAP " + std::to_string(currentLap) + " / " + std::to_string(totalLaps),
                     135.f, 60.f, 16, sf::Color(255, 70, 80));

    // Refined Styling
    timerText.setCharacterSize(16);
    timerText.setFillColor(sf::Color(255, 230, 230)); // Pinkish off-white digits
    timerText.setPosition(25.f, 23.f);

    window.draw(timerText);
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
void Graphics::setCarColors(const std::vector<Car *> cars)
{
    for (size_t i = 0; i < cars.size(); i++)
    {
        cars[i]->setBodyColor(carColors[i]);
    }
}
void Graphics::renderHomeScreen()
{
    window.setView(hudView);
    window.clear(sf::Color::Black);
    window.draw(homeBackground);

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

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

    // 1. Background Overlay
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

    // 3. Header
    drawTextCentered("TIME TRIAL COMPLETE!", 600.f, 240.f, 35, sf::Color::White);

    // 4. Global Top 3 Loop
    for (size_t i = 0; i < std::min<size_t>(3, allTimes.size()); ++i)
    {
        std::string label = (i == 0) ? "1ST: " : (i == 1) ? "2ND: "
                                                          : "3RD: ";

        sf::Color textColor = sf::Color(200, 200, 200); // Default Silver
        if (i == 0)
            textColor = sf::Color(255, 215, 0); // Gold
        else if (i == 2)
            textColor = sf::Color(205, 127, 50); // Bronze

        std::string rowText = label + allTimes[i].title + " - " + formatRaceTime(allTimes[i].bestLap);
        drawTextCentered(rowText, 600.f, 290.f + (i * 35.f), 24, textColor);
    }

    if (lapData.bestLap != BESTLAP_INIT_VAL)
    {
        std::string bestLapDisplay = "Best Lap: " + formatRaceTime(lapData.bestLap);
        drawTextCentered(bestLapDisplay, 600.f, 430.f, 22, sf::Color::Cyan);

        if (!lapData.title.empty())
        {
            std::string byText = "By: " + lapData.title;
            drawTextCentered(byText, 600.f, 445.f, 18, sf::Color::Cyan);
        }
    }

    renderLevelCompleteButtons();
}

void Graphics::renderGamePlay(const std::vector<Car *> cars)
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
    if (player.isInvincible())
    {
        if ((int)(player.getITime() * 10.f) % 2 != 0)
            player.draw(window);
    }
    else
        player.draw(window);
    for (size_t i = 2; i < cars.size(); i++)
    {
        if (!cars[i]->getActive())
            continue;
        cars[i]->draw(window);
    }
    debugPlayDisplay(&player);
}

void Graphics::renderPauseScreen()
{
    window.setView(hudView);
    window.draw(pause); // Draw your pause texture overlay

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

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
        drawTextCentered(labels[i], rect.left + rect.width / 2.f, rect.top + rect.height / 2.f, 22, isHovered ? hoverText : standardText);
    }
}

void Graphics::debugPlayDisplay(Car *car)
{
    sf::Color colors[]{sf::Color::Red, sf::Color::Yellow, sf::Color::Blue, sf::Color::Green};
    auto corners = car->getCorners(car->getPosition(), car->getAngle());
    for (int i = 0; i < 4; i++)
    {
        sf::CircleShape dot(1.f);
        dot.setFillColor(colors[i]);
        dot.setPosition(corners[i].x - 1.f, corners[i].y - 1.f);
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

void Graphics::renderResetButton(Gamemode mode, bool p1, bool p2)
{
    // Switch to HUD space so it stays fixed on screen
    window.setView(hudView);

    if (mode != Gamemode::PVP)
    {
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

        drawTextCentered("RESET CAR (PRESS R)", centerX, centerY, 20, sf::Color::White);
    }
    else
    {
        bool needsDraw[] = {p1, p2};

        for (size_t i = 0; i < 2; ++i)
        {
            if (needsDraw[i] && i < resetButtonPVP.size())
            {
                sf::FloatRect rect = resetButtonPVP[i];
                sf::RectangleShape btn(sf::Vector2f(rect.width, rect.height));
                btn.setPosition(rect.left, rect.top);
                btn.setFillColor(sf::Color(30, 30, 30, 230));
                btn.setOutlineColor(i == 0 ? sf::Color::Cyan : sf::Color::Yellow); // Distinct colors per player
                btn.setOutlineThickness(2.f);
                window.draw(btn);

                std::string label = (i == 0) ? "P1 RESET (PRESS R)" : "P2 RESET (PRESS M)";
                drawTextCentered(label, rect.left + rect.width / 2.f,
                                 rect.top + rect.height / 2.f, 16, sf::Color::White);
            }
        }
    }
}

void Graphics::renderSettingsScreen(int currentLevel, int currentLaps, bool isMuted)
{
    window.setView(hudView);
    window.clear(sf::Color(20, 20, 25));

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

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
        button.setFillColor(isActive ? hoverFill : standardFill);

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

void Graphics::renderPVPGameplay(Player &player2)
{
    sf::Vector2u trackSize = track.getSize();

    // Store players and viewport starting X-coordinates in arrays for iteration
    const Car *players[2] = {&player, &player2};
    float viewportsX[2] = {0.f, 0.5f};

    // Dynamic Clamping Math based on zoom level
    const float zoomFactor = 0.3f;
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
        if (player.isInvincible())
        {
            if ((int)(player.getITime() * 10.f) % 2 != 0)
                player.draw(window);
        }
        else
            player.draw(window);
        if (player2.isInvincible())
        {
            if ((int)(player2.getITime() * 10.f) % 2 != 0)
                player2.draw(window);
        }
        else
            player2.draw(window);
    }
    debugPlayDisplay(&player);
    debugPlayDisplay(&player2);
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
    int laps[] = {player.getCurrLap(), player2.getCurrLap()};
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

void Graphics::renderPVPLvlComplete(const Player &player2, const std::vector<Car *> &podium)
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

    // Track rows drawn to format text positions and labels correctly if a car is skipped
    size_t displayedCount = 0;
    for (size_t i = 0; i < podium.size() && displayedCount < 2; ++i)
    {
        // Guardrail: Skip inactive or null cars
        if (!podium[i] || !podium[i]->getActive())
            continue;

        std::string label = (displayedCount == 0) ? "1ST: " : "2ND: ";
        sf::Color textColor = (displayedCount == 0) ? sf::Color(255, 215, 0) : sf::Color(200, 200, 200);

        drawTextCentered(label + podium[i]->getTitle(),
                         600.f, 310.f + (displayedCount * 35.f), 24, textColor);

        displayedCount++;
    }

    float p1Best = player.getBestLapTime(); // Assuming player1 is accessible in this class/scope
    float p2Best = player2.getBestLapTime();

    // Determine the fastest of the two
    float fastestTime = std::min(p1Best, p2Best);
    std::string fastestTitle = (p1Best < p2Best) ? player.getTitle() : player2.getTitle();

    if (fastestTime != BESTLAP_INIT_VAL)
    {
        std::string lapDisplay = "Fastest Lap: " + formatRaceTime(fastestTime);
        std::string byDisplay = "By: " + fastestTitle;

        drawTextCentered(lapDisplay, 600.f, 410.f, 22, sf::Color::Cyan);
        drawTextCentered(byDisplay, 600.f, 435.f, 18, sf::Color::Cyan);
    }

    renderLevelCompleteButtons();
}
void Graphics::renderMinimap(const std::vector<Car *> &cars, Gamemode mode)
{
    window.setView(hudView);

    float minimapWidth = minimapTexture.getSize().x * minimapScale;
    float minimapHeight = minimapTexture.getSize().y * minimapScale;

    // Position based on mode
    float minimapX = (mode != Gamemode::PVP) ? (10.f) :           // Bottom-right for single player
                         ((1200.f / 2.f) - (minimapWidth / 2.f)); // Bottom-center for multiplayer

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
    for (size_t i = 0; i < cars.size(); ++i)
    {
        if (!cars[i]->getActive())
            continue;
        sf::CircleShape dot(4.f);

        const sf::Uint8 lightnessBoost = 60;
        sf::Color original = carColors[i];
        sf::Color minimapColor(
            std::min(255, original.r + lightnessBoost),
            std::min(255, original.g + lightnessBoost),
            std::min(255, original.b + lightnessBoost));

        dot.setFillColor(minimapColor);

        sf::Vector2f minimapPos = cars[i]->getPosition() * minimapScale;
        minimapPos.x += minimapX;
        minimapPos.y += minimapY;

        dot.setPosition(minimapPos);
        window.draw(dot);
    }
}

void Graphics::renderAILvlComplete(Car &player, const LapTime &lapData, const std::vector<Car *> &podium)
{
    window.setView(window.getDefaultView());

    // 1. Background & Panel
    sf::RectangleShape overlay(sf::Vector2f(1200.f, 800.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 220));
    window.draw(overlay);

    sf::RectangleShape panel(sf::Vector2f(600.f, 400.f));
    panel.setPosition(300.f, 200.f);
    panel.setFillColor(sf::Color(20, 5, 8, 240));
    panel.setOutlineColor(sf::Color(210, 35, 45));
    panel.setOutlineThickness(3.f);
    window.draw(panel);

    // 2. Text
    drawTextCentered("RACE COMPLETE!", 600.f, 240.f, 40, sf::Color::White);

    // Track rows drawn to format text positions and labels correctly if a car is skipped
    size_t displayedCount = 0;
    for (size_t i = 0; i < podium.size() && displayedCount < 3; ++i)
    {
        // Guardrail: Skip inactive or null cars
        if (!podium[i] || !podium[i]->getActive())
            continue;

        std::string label = (displayedCount == 0) ? "1ST: " : (displayedCount == 1) ? "2ND: "
                                                                                    : "3RD: ";

        sf::Color textColor = sf::Color(200, 200, 200); // Silver/Grey basic
        if (displayedCount == 0)
            textColor = sf::Color(255, 215, 0); // Gold
        else if (displayedCount == 2)
            textColor = sf::Color(205, 127, 50); // Bronze

        drawTextCentered(label + podium[i]->getTitle(),
                         600.f, 290.f + (displayedCount * 35.f), 24, textColor);

        displayedCount++;
    }

    // Shifted Best Lap down to 410.f to make room
    if (lapData.bestLap != BESTLAP_INIT_VAL)
    {
        drawTextCentered("Best Lap: " + formatRaceTime(lapData.bestLap), 600.f, 410.f, 22, sf::Color(160, 160, 170));
        if (!lapData.title.empty())
        {
            std::string byText = "By: " + lapData.title;
            drawTextCentered(byText, 600.f, 430.f, 18, sf::Color(160, 160, 170));
        }
    }
    // Live player position display
    drawTextCentered("Your Position: P" + std::to_string(player.getRacePos()), 600.f, 460.f, 22, sf::Color::Cyan);

    // 3. Buttons (Clear and reuse existing member vector)
    renderLevelCompleteButtons();
}

void Graphics::renderAIHUD(const std::vector<Car *> &raceLeaderboard, int totalLaps, float elapsed)
{
    // Switch to HUD view so UI elements remain locked to the window
    window.setView(hudView);

    // ========================================================
    // 1. TOP-LEFT TIMING & LAP PODIUM
    // ========================================================
    sf::RectangleShape topPodium(sf::Vector2f(240.f, 75.f));
    topPodium.setPosition(15.f, 15.f);
    topPodium.setFillColor(sf::Color(30, 8, 10, 200));
    topPodium.setOutlineColor(sf::Color(140, 30, 40));
    topPodium.setOutlineThickness(1.5f);
    window.draw(topPodium);

    // Row 1: Race Timer
    timerText.setString("TIME " + formatRaceTime(elapsed));
    timerText.setCharacterSize(16);
    timerText.setFillColor(sf::Color(255, 230, 230));
    timerText.setPosition(25.f, 22.f);
    window.draw(timerText);

    // Row 2: Player Current Lap Counter (Accessing member 'player' directly)
    sf::Text lapCounterText;
    lapCounterText.setFont(*timerText.getFont());
    lapCounterText.setCharacterSize(15);
    lapCounterText.setFillColor(sf::Color(160, 160, 170));

    int currentLapDisplay = std::min(player.getCurrLap(), totalLaps);
    lapCounterText.setString("LAP  " + std::to_string(currentLapDisplay) + " / " + std::to_string(totalLaps));
    lapCounterText.setPosition(25.f, 48.f);
    window.draw(lapCounterText);

    // ========================================================
    // 2. F1-STYLE LIVE LEADERBOARD (LEFT SIDE)
    // ========================================================
    float rowHeight = 28.f;
    float padding = 15.f;
    float leaderHeight = (raceLeaderboard.size() * rowHeight) + padding;

    sf::RectangleShape leaderPodium(sf::Vector2f(120.f, leaderHeight));
    leaderPodium.setPosition(15.f, 105.f);
    leaderPodium.setFillColor(sf::Color(20, 5, 8, 220));
    leaderPodium.setOutlineColor(sf::Color(210, 35, 45));
    leaderPodium.setOutlineThickness(1.5f);
    window.draw(leaderPodium);

    sf::Text leaderText;
    leaderText.setFont(*timerText.getFont());
    leaderText.setCharacterSize(15);

    size_t displayedCount = 0;
    for (size_t i = 0; i < raceLeaderboard.size(); ++i)
    {
        Car *c = raceLeaderboard[i];
        if (!c || !c->getActive())
            continue;

        int pos = c->getRacePos();
        std::string title = c->getTitle();
        float startY = 115.f + (displayedCount * rowHeight);

        // Dynamic podium coloring
        sf::Color rowColor = sf::Color::White;
        if (pos == 1)
            rowColor = sf::Color(255, 215, 0); // Gold
        else if (pos == 2)
            rowColor = sf::Color(192, 192, 192); // Silver
        else if (pos == 3)
            rowColor = sf::Color(205, 127, 50); // Bronze

        if (title == "PLR")
            rowColor = sf::Color::Cyan;

        // Col 1: Pos
        leaderText.setString(std::to_string(pos));
        leaderText.setFillColor(rowColor);
        leaderText.setPosition(25.f, startY);
        window.draw(leaderText);

        // Col 2: Tag
        leaderText.setString(title);
        leaderText.setFillColor(rowColor);
        leaderText.setPosition(65.f, startY);
        window.draw(leaderText);

        displayedCount++;
    }

    // ========================================================
    // 3. BOTTOM-RIGHT SPEEDOMETER (SINGLE PLAYER ONLY)
    // ========================================================
    float speedX = 960.f;
    float speedY1 = 700.f;
    int displaySpeed1 = (int)(std::abs(player.getCurrSpeed())); // Using member 'player' here too

    sf::RectangleShape speedPodium1(sf::Vector2f(220.f, 85.f));
    speedPodium1.setPosition(speedX, speedY1);
    speedPodium1.setFillColor(sf::Color(20, 5, 8, 220));
    speedPodium1.setOutlineColor(sf::Color(210, 35, 45));
    speedPodium1.setOutlineThickness(2.f);
    window.draw(speedPodium1);

    speedometer.setString(std::to_string(displaySpeed1));
    speedometer.setCharacterSize(45);
    speedometer.setFillColor(sf::Color::White);
    speedometer.setPosition(speedX + 20.f, speedY1 + 10.f);
    window.draw(speedometer);

    sf::Text unitText1;
    unitText1.setFont(*timerText.getFont());
    unitText1.setString("KM/H");
    unitText1.setCharacterSize(14);
    unitText1.setFillColor(sf::Color(160, 160, 170));
    unitText1.setPosition(speedX + 140.f, speedY1 + 45.f);
    window.draw(unitText1);

    // ========================================================
    // 4. TOP-RIGHT PAUSE BUTTON ICON INTERACTION
    // ========================================================
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mappedMousePos = window.mapPixelToCoords(mousePos, hudView);

    if (pauseButton.contains(mappedMousePos))
    {
        pause.setColor(sf::Color(255, 100, 100)); // Hover flash
    }
    else
    {
        pause.setColor(sf::Color(255, 255, 255, 220));
    }
    window.draw(pause);

    // Restore original game view camera configuration
    window.setView(gameView);
}

void Graphics::renderLevelCompleteButtons()
{
    std::string labels[] = {"RESTART", "MENU"};

    // Iterate over the vector that was already populated in initUI()
    for (size_t i = 0; i < levelCompleteButtons.size(); ++i)
    {
        // 1. Retrieve the pre-calculated bounds
        const sf::FloatRect &btnRect = levelCompleteButtons[i];

        // 2. Set up the shape for this frame
        sf::RectangleShape btn(sf::Vector2f(btnRect.width, btnRect.height));
        btn.setPosition(btnRect.left, btnRect.top);

        // Use your cached class member colors here
        btn.setFillColor(standardFill);
        btn.setOutlineColor(standardOutline);
        btn.setOutlineThickness(2.f);

        // 3. Draw shape and text
        window.draw(btn);

        // Leverage the existing FloatRect bounds to perfectly center the text
        drawTextCentered(labels[i],
                         btnRect.left + (btnRect.width / 2.f),
                         btnRect.top + (btnRect.height / 2.f),
                         18, sf::Color::White);
    }
}

void Graphics::renderCountdown(float countdownTimer)
{
    window.setView(hudView);

    static sf::RectangleShape overlay(sf::Vector2f(1200.f, 800.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    // 2. Logic: Determine string and color
    std::string countdownStr = (countdownTimer > 2.0f) ? "3" : (countdownTimer > 1.0f) ? "2"
                                                           : (countdownTimer > 0.5f)   ? "1"
                                                                                       : "GO!";

    sf::Color textColor = (countdownTimer > 0.0f) ? sf::Color::White : sf::Color::Green;

    // 3. Draw text using your centralized helper
    // 600, 350 centers it horizontally and sets the vertical anchor
    drawTextCentered(countdownStr, 600.f, 350.f, 200, textColor);

    // 4. Subtitle
    drawTextCentered("GET READY", 600.f, 460.f, 32, sf::Color(200, 200, 200));
}