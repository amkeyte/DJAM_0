#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * Holds host timing information (tempo, time signature, sample position)
 * and helper utilities for bar-synced clip scheduling.
 */
struct HostPhase
{
    double bpm = 120.0;
    int numerator = 4, denominator = 4;
    bool isPlaying = false;
    double ppqPosition = 0.0;      // Host beats from start
    juce::int64 currentSample = 0;       // Host sample position
    double sampleRate = 44100.0;

    /** Returns the number of samples in one bar. */
    int samplesPerBar() const
    {
        const double beatsPerBar = (double)numerator;
        const double secPerBeat = 60.0 / bpm;
        return (int)std::round(beatsPerBar * secPerBeat * sampleRate);
    }
};

/** Queries the host for current tempo, signature, and play state. */
bool getHostPhase(juce::AudioPlayHead* playHead, HostPhase& out);

/** Computes the number of samples remaining until the next bar boundary. */
int samplesToNextBar(const HostPhase& hp);
