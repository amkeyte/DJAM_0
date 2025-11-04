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
    int  pendingClip = -1;    // clip to activate when armed
    juce::String name;        // optional cache for display
};

/** A single performer slot that can play one clip at a time. */
class Slot
{
public:
    Slot() = default;

    void setClipBank(const std::vector<DJamClip>* bank);

    void armStart(int clipIndex);
    void applyArmedStart();

    void stopPlayback();
    void jumpTo(double ppq);

    void toggleMute();
    void setSolo(bool v);

    bool isMuted()   const noexcept;
    bool isSolo()    const noexcept;
    bool isArmed()   const noexcept;

    int getActiveClipIndex()  const noexcept;
    int getPendingClipIndex() const noexcept;

    const DJamClip* getActiveClip()  const noexcept;
    const DJamClip* getPendingClip() const noexcept;

    juce::String getActiveClipName()  const noexcept;
    juce::String getPendingClipName() const noexcept;

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
