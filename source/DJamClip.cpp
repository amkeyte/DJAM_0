#include <juce_audio_formats/juce_audio_formats.h>
#include "DJamClip.h"

void DJamClip::loadFromFile(const juce::File& file, double targetSR)
{
    DBG("loadFromFile loading: " + file.getFileName());
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();

    if (auto* r = fm.createReaderFor(file))
    {
        name = file.getFileNameWithoutExtension();
        std::unique_ptr<juce::AudioFormatReader> reader(r);
        const juce::int64 n = (juce::int64)reader->lengthInSamples;

        juce::AudioBuffer<float> tmp((int)reader->numChannels, (int)n);
        reader->read(&tmp, 0, (int)n, 0, true, true);

        // Optional resample if file sample rate ≠ target sample rate
        if (reader->sampleRate != targetSR && targetSR > 0.0)
        {
            const double ratio = reader->sampleRate / targetSR;
            const int newNumSamples = (int)((double)n / ratio);
            juce::AudioBuffer<float> resampled((int)reader->numChannels, newNumSamples);

            juce::LagrangeInterpolator interp;
            for (int ch = 0; ch < tmp.getNumChannels(); ++ch)
            {
                interp.reset();
                interp.process(ratio,
                    tmp.getReadPointer(ch),
                    resampled.getWritePointer(ch),
                    newNumSamples);
            }

            buffer = std::move(resampled);
        }
        else
        {
            buffer = std::move(tmp);
        }
    }
}

int DJamClip::totalSamplesAt(double sampleRate, const HostPhase& hp) const
{
    const double beatsPerBar = (double)hp.numerator;
    const double secPerBeat = 60.0 / hp.bpm;
    const double secPerBar = beatsPerBar * secPerBeat;
    return (int)std::round(secPerBar * sampleRate * barsLength);
}

void DJamClip::render(juce::AudioBuffer<float>& outBuffer,
    int startSample,
    int numSamples,
    int destOffset,
    int phaseSamples) const
{
    if (!isLoaded() || numSamples <= 0)
        return;

    const int numChannels = juce::jmin(outBuffer.getNumChannels(), buffer.getNumChannels());
    const int clipLength = buffer.getNumSamples();
    if (clipLength <= 0)
        return;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* dest = outBuffer.getWritePointer(ch, destOffset);
        const auto* src = buffer.getReadPointer(ch % buffer.getNumChannels());

        int pos = phaseSamples % clipLength;

        for (int i = 0; i < numSamples; ++i)
        {
            dest[i] += src[pos];
            pos++;
            if (pos >= clipLength)
                pos = 0; // loop wrap
        }
    }
}
