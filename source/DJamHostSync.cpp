#include "DJamHostSync.h"

bool getHostPhase(juce::AudioPlayHead* playHead, HostPhase& out)
{
    if (playHead == nullptr)
        return false;

    juce::AudioPlayHead::CurrentPositionInfo pos;
    if (!playHead->getCurrentPosition(pos))
        return false;

    out.bpm = (pos.bpm > 0.0 ? pos.bpm : out.bpm);
    out.numerator = (pos.timeSigNumerator > 0 ? pos.timeSigNumerator : out.numerator);
    out.denominator = (pos.timeSigDenominator > 0 ? pos.timeSigDenominator : out.denominator);
    out.isPlaying = pos.isPlaying;
    out.ppqPosition = pos.ppqPosition;
    out.currentSample = (juce::int64)pos.timeInSamples;

    return true;
}

int samplesToNextBar(const HostPhase& hp)
{
    // Guard against invalid tempo
    if (hp.bpm <= 0.0 || hp.sampleRate <= 0.0)
        return 0;

    // Compute the number of beats per bar and beats elapsed
    const double beatsPerBar = (double)hp.numerator;
    const double barPos = std::floor(hp.ppqPosition / beatsPerBar);
    const double nextBarBeat = (barPos + 1.0) * beatsPerBar;
    const double beatsToNext = nextBarBeat - hp.ppqPosition;

    const double secPerBeat = 60.0 / hp.bpm;
    const int samplesToNext = (int)std::round(beatsToNext * secPerBeat * hp.sampleRate);

    return std::max(1, samplesToNext);
}
