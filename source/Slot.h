#pragma once

#include <memory>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "DJamClip.h"
#include "DJamHostSync.h"

/**
 * A single audio slot that holds one clip and renders it
 * during scheduled playback, quantized to bars/beats.
 */
class Slot
{
public:
    Slot() = default;

    /** Loads an audio clip into this slot. */
    void loadClip(const juce::File& file);

    /** Renders the clip at the specified host phase into the output buffer. */
    void render(juce::AudioBuffer<float>& output,
        int startSample, int numSamples,
        const HostPhase& hp);

    /** Sets the loop length of this slot (in bars). */
    void setBarsLength(int bars) noexcept { barsLength = bars; }

    /** Called to sync this slot to a new playhead position. */
    void jumpTo(double hostPPQ);

    /** Returns the number of samples this clip will occupy at the given host phase. */
    int totalSamplesAtHostPhase(double sampleRate, const HostPhase& hp) const;

private:
    std::unique_ptr<DJamClip> clip;
    int barsLength = 1;

    double playheadPositionBeats = 0.0;
};
