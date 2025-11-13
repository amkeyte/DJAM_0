#include "Slot.h"

void Slot::loadClip(const juce::File& file)
{
    clip = std::make_unique<DJamClip>();
    clip->loadFromFile(file);
    // No longer sets loop length — DJamClip is now read-only
}

void Slot::render(juce::AudioBuffer<float>& output,
    int startSample, int numSamples,
    const HostPhase& hp)
{
    if (!clip || !clip->isLoaded())
        return;

    const int totalSamples = totalSamplesAtHostPhase(hp.sampleRate, hp);
    const double samplesPerBeat = (60.0 / hp.bpm) * hp.sampleRate;

    int startBeat = static_cast<int>(std::floor(playheadPositionBeats));
    int beatCount = static_cast<int>(std::ceil((double)numSamples / samplesPerBeat));

    clip->render(output, startSample, numSamples, startBeat, beatCount);

    // Update playhead position
    playheadPositionBeats += ((double)numSamples / samplesPerBeat);
    while (playheadPositionBeats >= barsLength * hp.beatsPerBar)
        playheadPositionBeats -= barsLength * hp.beatsPerBar;
}

void Slot::jumpTo(double hostPPQ)
{
    if (!clip || !clip->isLoaded())
        return;

    const double beatsPerBar = clip->getBeatsPerBar();
    const double totalBeats = barsLength * beatsPerBar;

    playheadPositionBeats = std::fmod(hostPPQ, totalBeats);
}

int Slot::totalSamplesAtHostPhase(double sampleRate, const HostPhase& hp) const
{
    if (!clip || !clip->isLoaded())
        return 0;

    const double beats = barsLength * hp.beatsPerBar;
    const double seconds = (60.0 * beats) / hp.bpm;
    return static_cast<int>(seconds * sampleRate);
}
