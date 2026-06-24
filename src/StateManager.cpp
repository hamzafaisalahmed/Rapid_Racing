#include "StateManager.h"

void StateManager::pushPlaying()
{
    homeAudio.stop();
    endscreenAudio.stop();
    engineAudio.play();
    engineAudio.setVolume(0.f);
    stateStack.push(GameState::Playing);
}

void StateManager::pushPause()
{
    engineAudio.pause();
    stateStack.push(GameState::Paused);
    engineAudio.setVolume(0.f);
}

void StateManager::pushLevelComplete()
{
    homeAudio.stop();
    engineAudio.stop();
    engineAudio.setVolume(0.f);
    endscreenAudio.play();
    stateStack.push(GameState::LevelComplete);
}

void StateManager::pushHome()
{
    engineAudio.stop();
    engineAudio.setVolume(0.f);
    endscreenAudio.stop();
    homeAudio.play();
    stateStack.push(GameState::Home);
}

void StateManager::pushSettings()
{
    stateStack.push(GameState::Settings);
}

void StateManager::pop()
{
    if (!stateStack.empty())
        stateStack.pop();

    // Restore audio for the state we're returning to
    if (!stateStack.empty())
    {
        GameState currentState = stateStack.top();
        if (currentState == GameState::Playing)
        {
            engineAudio.play();
            endscreenAudio.stop();
            homeAudio.stop();
        }
        else if (currentState == GameState::Home)
        {
            engineAudio.stop();
            endscreenAudio.stop();
            homeAudio.play();
        }
        else if (currentState == GameState::LevelComplete)
        {
            engineAudio.stop();
            endscreenAudio.play();
        }
    }
}

void StateManager::clear()
{
    while (stateStack.size() > 0)
        stateStack.pop();
    pushHome();
}

void StateManager::toggleMute()
{
    if (currVol == 70.f)
        currVol = 0.f;
    else
        currVol = 70.f;
    homeAudio.setVolume(currVol);
    endscreenAudio.setVolume(currVol);
}

void StateManager::stopAudio()
{
    engineAudio.stop();
    endscreenAudio.stop();
    homeAudio.stop();
}