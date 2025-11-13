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
    void loadFromFile(const juce::File& file);

    bool isLoaded() const noexcept { return buffer.getNumSamples() > 0; }

    int getLoopLengthBars() const noexcept { return barsLength; }
    float getBPM() const noexcept { return bpm; }
    int getBeatsPerBar() const noexcept { return beatsPerBar; }
    double getSampleRate() const noexcept { return sampleRate; }

    void setLoopLengthBars(int bars) noexcept { barsLength = bars; }

    /** Renders a bar-aligned clip into the audio buffer. */
    void render(juce::AudioBuffer<float>& output,
        int startSample, int numSamples,
        int startBeat, int beatCount);

private:
    juce::AudioBuffer<float> buffer;
    juce::String name;

    int barsLength = 1;
    int beatsPerBar = 4;
    int numBeats = 16;
    float bpm = 120.f;
    double sampleRate = 44100.0;
};
