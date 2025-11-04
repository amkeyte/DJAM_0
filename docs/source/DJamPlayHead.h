#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <functional>

/**
 * DJAMPlayHead
 * Tracks host transport state and emits edge events.
 *
 * Call `update(getPlayHead())` once per block.
 * Subscribe to events using the `on...` listener hooks.
 */
class DJamPlayHead
{
public:
    DJamPlayHead() = default;

    // Call this each block with the plugin's current playhead
    void update(juce::AudioPlayHead* playHead);

    // Listener hooks — these fire only on change
    std::function<void(const juce::AudioPlayHead::CurrentPositionInfo&)> onStart;
    std::function<void()> onStop;
    std::function<void(double oldPPQ, double newPPQ)> onJump;

    // State getters
    bool isPlaying() const noexcept { return playingNow; }
    double getPPQPosition() const noexcept { return currentInfo.ppqPosition; }
    const juce::AudioPlayHead::CurrentPositionInfo& getInfo() const noexcept { return currentInfo; }

private:
    juce::AudioPlayHead::CurrentPositionInfo currentInfo{};
    juce::AudioPlayHead::CurrentPositionInfo previousInfo{};

    bool playingNow = false;
    bool playingPrev = false;
    double lastPPQ = 0.0;
    bool validLast = false;
};
