#pragma once
#include <SFML/Graphics.hpp>
#include "Game.h"
// graphic functions, but still part of Game class
void Game::drawTextCentered(const std::string &str, float y, int size, sf::Color col)
{
    sf::Text text;
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(col);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    text.setPosition(240.f, y);
    window.draw(text);
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

void Game::renderHomeScreen()
{
    window.setView(hudView);
    window.clear(sf::Color::Black);
    window.draw(homeBackground);
    // --- BUTTON 1: PLAY ---
    sf::RectangleShape playBtn(sf::Vector2f(140.f, 50.f));
    playBtn.setPosition(170.f, 460.f);
    playBtn.setFillColor(sf::Color(20, 50, 90));
    playBtn.setOutlineColor(sf::Color(80, 160, 240));
    playBtn.setOutlineThickness(2.f);
    window.draw(playBtn);
    drawTextCentered("PLAY", 485.f, 24, sf::Color(180, 220, 255)); // Centered on Y = 485

    // --- BUTTON 2: CUSTOMIZE ---
    sf::RectangleShape customizeBtn(sf::Vector2f(140.f, 50.f));
    customizeBtn.setPosition(170.f, 530.f); // Shifted down 70px
    customizeBtn.setFillColor(sf::Color(20, 50, 90));
    customizeBtn.setOutlineColor(sf::Color(80, 160, 240));
    customizeBtn.setOutlineThickness(2.f);
    window.draw(customizeBtn);
    drawTextCentered("CUSTOMIZE", 555.f, 24, sf::Color(180, 220, 255)); // Centered on Y = 555

    // --- BUTTON 3: LEADERBOARD ---
    sf::RectangleShape leaderboardBtn(sf::Vector2f(140.f, 50.f));
    leaderboardBtn.setPosition(170.f, 600.f); // Shifted down 70px
    leaderboardBtn.setFillColor(sf::Color(20, 50, 90));
    leaderboardBtn.setOutlineColor(sf::Color(80, 160, 240));
    leaderboardBtn.setOutlineThickness(2.f);
    window.draw(leaderboardBtn);
    drawTextCentered("LEADERBOARD", 625.f, 24, sf::Color(180, 220, 255)); // Centered on Y = 625

    // --- BUTTON 4: SETTINGS ---
    sf::RectangleShape settingsBtn(sf::Vector2f(140.f, 50.f));
    settingsBtn.setPosition(170.f, 670.f); // Shifted down 70px
    settingsBtn.setFillColor(sf::Color(20, 50, 90));
    settingsBtn.setOutlineColor(sf::Color(80, 160, 240));
    settingsBtn.setOutlineThickness(2.f);
    window.draw(settingsBtn);
    drawTextCentered("SETTINGS", 695.f, 24, sf::Color(180, 220, 255)); // Centered on Y = 695
}

void Game::renderGameOver()
{
}

void Game::renderLevelComplete()
{
}

void Game::renderLevelSelect()
{
}