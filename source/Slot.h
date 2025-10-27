#pragma once
#include <vector>
#include <juce_audio_basics/juce_audio_basics.h>
#include "DJamClip.h"
#include "DJamHostSync.h"

/** Playback state for one slot */
struct SlotState
{
    int  activeClip = -1;
    bool mute = false;
    bool solo = false;
    int  phaseSamples = 0;    // current position within clip
    bool armedStart = false;
    int  pendingClip = -1;   // clip to activate when armed
    juce::String name;        // optional cache for display
};

/** A single performer slot that can play one clip at a time. */
class Slot
{
public:
    Slot() = default;

    void setClipBank(const std::vector<DJamClip>* bank) { _clips = bank; }

    void armStart(int clipIndex) { _slotState.armedStart = true; _slotState.pendingClip = clipIndex; }
    void toggleMute() { _slotState.mute = !_slotState.mute; }
    void setSolo(bool v) { _slotState.solo = v; }

    bool isMuted()   const noexcept { return _slotState.mute; }
    bool isSolo()    const noexcept { return _slotState.solo; }
    bool isArmed()   const noexcept { return _slotState.armedStart; }

    // Indices
    int getActiveClipIndex()  const noexcept { return _slotState.activeClip; }
    int getPendingClipIndex() const noexcept { return _slotState.pendingClip; }

    // Direct pointers (nullptr if invalid)
    const DJamClip* getActiveClip()  const noexcept;
    const DJamClip* getPendingClip() const noexcept;

    // *** Minimal convenience (null-safe one-liners) ***
    juce::String getActiveClipName()  const noexcept;
    juce::String getPendingClipName() const noexcept;

    void applyArmedStart();

    bool render(juce::AudioBuffer<float>& out,
        int startSample,
        int numSamples,
        int destOffset,
        const HostPhase& hp);

    const SlotState& state() const noexcept { return _slotState; }

private:
    const std::vector<DJamClip>* _clips = nullptr;
    SlotState _slotState;
};
