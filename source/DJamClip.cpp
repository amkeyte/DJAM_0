#include "DJamClip.h"
#include <juce_audio_formats/juce_audio_formats.h>


// Shared static AudioFormatManager for all DJamClips
juce::AudioFormatManager& getSharedFormatManager()
{
    static juce::AudioFormatManager fm;
    static bool initialized = false;

    if (!initialized)
    {
        fm.registerBasicFormats();  // WAV, AIFF, MP3, etc.
        initialized = true;
    }
    return fm;
}


void DJamClip::loadFromFile(const juce::File& file)
{
    DBG("loadFromFile loading: " + file.getFileName());

    name = file.getFileNameWithoutExtension();

    auto& fm = getSharedFormatManager();
    std::unique_ptr<juce::AudioFormatReader> reader(fm.createReaderFor(file));

    if (reader != nullptr)
    {
        buffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&buffer, 0, (int)reader->lengthInSamples, 0, true, true);
    }
    else
    {
        DBG("Failed to create reader for: " + file.getFullPathName());
    }
}
void DJamClip::render(juce::AudioBuffer<float>& output,
    int startSample, int numSamples,
    int startBeat, int beatCount)
{
    if (!isLoaded())
        return;

    const int channels = std::min(output.getNumChannels(), buffer.getNumChannels());
    const int samplesPerBeat = buffer.getNumSamples() / numBeats;
    const int startSampleInClip = startBeat * samplesPerBeat;
    const int samplesToCopy = std::min(numSamples, buffer.getNumSamples() - startSampleInClip);

    for (int ch = 0; ch < channels; ++ch)
    {
        output.addFrom(ch, startSample,
            buffer,
            ch, startSampleInClip,
            samplesToCopy);
    }
}
