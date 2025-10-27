#pragma once
#include <juce_core/juce_core.h> 
#include <juce_audio_basics/juce_audio_basics.h>
#include "DJamHostSync.h"

/**
 * Represents a short, loopable audio clip loaded from disk.
 * Each clip is assumed to be bar-aligned and ready for
 * bar-quantized playback inside the DJam engine.
 */
class DJamClip
{
public:
    DJamClip() = default;

    /** Loads a clip from file and optionally resamples to match targetSR. */
    void loadFromFile(const juce::File& file, double targetSR);

    /** Returns true if a valid audio buffer is loaded. */
    bool isLoaded() const noexcept { return buffer.getNumSamples() > 0; }

    /** Sets the loop length in musical bars (for timing sync). */
    void setLoopLengthBars(int bars) noexcept { barsLength = bars; }

    /** Returns the total loop length in bars. */
    int getLoopLengthBars() const noexcept { return barsLength; }

    /** Returns the total number of samples for this clip at a given sample rate. */
    int totalSamplesAt(double sampleRate, const HostPhase& hp) const;

    /** name of the file */
    const juce::String& getName() const noexcept { return name; }

    /**
     * Renders the clip into `outBuffer` starting at destOffset,
     * looping seamlessly if the playback phase wraps.
     */
    void render(juce::AudioBuffer<float>& outBuffer,
        int startSample,
        int numSamples,
        int destOffset,
        int phaseSamples) const;

private:
    juce::AudioBuffer<float> buffer;
    int barsLength = 1;
    juce::String name;


};
