#pragma once
#include <stack>
#include <SFML/Audio.hpp>
#include "Utils.h"

class StateManager
{
    std::stack<GameState> stateStack;
    sf::Music &engineAudio;
    sf::Music &endscreenAudio;
    sf::Music &homeAudio;
    float currVol;

public:
    StateManager(sf::Music &engine, sf::Music &endscreen, sf::Music &Home)
        : engineAudio(engine), endscreenAudio(endscreen), homeAudio(Home), currVol(70.f) {}
    void pushPlaying();
    void pushPause();
    void pushLevelComplete();
    void pushHome();
    void pushSettings();
    void pushAISetup();
    void toggleMute();
    void pop();
    void clear();
    float getCurrVol() const { return currVol; }
    GameState getCurrentState() { return stateStack.top(); }
    void stopAudio();
};