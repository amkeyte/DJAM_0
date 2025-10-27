#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_formats/juce_audio_formats.h>

// Factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DJAM0AudioProcessor();
}

//===================== Parameter Layout =====================

juce::AudioProcessorValueTreeState::ParameterLayout DJAM0AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    for (int s = 0; s < kNumSlots; ++s)
    {
        params.emplace_back(std::make_unique<juce::AudioParameterInt>(
            paramId_slotClip(s), "Slot " + juce::String(s + 1) + " Clip", -1, 255, -1));

        params.emplace_back(std::make_unique<juce::AudioParameterBool>(
            paramId_slotMute(s), "Slot " + juce::String(s + 1) + " Mute", false));

        params.emplace_back(std::make_unique<juce::AudioParameterBool>(
            paramId_slotSolo(s), "Slot " + juce::String(s + 1) + " Solo", false));
    }

    return { params.begin(), params.end() };
}

//===================== Processor =====================

DJAM0AudioProcessor::DJAM0AudioProcessor()
    : juce::AudioProcessor(
        BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "PARAMS", createParameterLayout())
{
    DBG("Strting DJAM0AudioProcessor");


    // Register listeners & init clip-name properties for UI labels
    for (int s = 0; s < kNumSlots; ++s)
    {
        apvts.addParameterListener(paramId_slotClip(s), this);
        apvts.addParameterListener(paramId_slotMute(s), this);
        apvts.addParameterListener(paramId_slotSolo(s), this);

        if (!apvts.state.hasProperty(paramId_slotClipName(s)))
            apvts.state.setProperty(paramId_slotClipName(s), juce::String(), nullptr);
    }
}

DJAM0AudioProcessor::~DJAM0AudioProcessor()
{
    for (int s = 0; s < kNumSlots; ++s)
    {
        apvts.removeParameterListener(paramId_slotClip(s), this);
        apvts.removeParameterListener(paramId_slotMute(s), this);
        apvts.removeParameterListener(paramId_slotSolo(s), this);
    }
}

bool DJAM0AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto out = layouts.getMainOutputChannelSet();
    if (out.isDisabled()) return false;
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo()) return false;

    auto in = layouts.getMainInputChannelSet();
    return in.isDisabled() || in == out;
}

void DJAM0AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    DBG("prepareToPlay");
    juce::ignoreUnused(samplesPerBlock);

    hostPhase.sampleRate = sampleRate;

    loadSamplePack();

    // Give slots a pointer to the bank
    for (auto& s : slots)
        s.setClipBank(&pack);

    // Optional: seed UI labels from current params (intent)
    for (int i = 0; i < kNumSlots; ++i)
    {
        const int idx = (int)apvts.getRawParameterValue(paramId_slotClip(i))->load();
        const juce::String name = (idx >= 0 && idx < (int)pack.size())
            ? pack[(size_t)idx].getName() : juce::String();
        setSlotClipName(i, name);
    }
}

void DJAM0AudioProcessor::releaseResources() {}

//===================== Param Callbacks =====================

void DJAM0AudioProcessor::parameterChanged(const juce::String& paramID, float newValue)
{
    for (int s = 0; s < kNumSlots; ++s)
    {
        if (paramID == paramId_slotClip(s)) { onSlotClipParamChanged(s, (int)newValue); return; }
        if (paramID == paramId_slotMute(s)) { onSlotMuteParamChanged(s, newValue > 0.5f); return; }
        if (paramID == paramId_slotSolo(s)) { onSlotSoloParamChanged(s, newValue > 0.5f); return; }
    }
}

void DJAM0AudioProcessor::onSlotClipParamChanged(int slot, int newClipIdx)
{
    // Queue for next bar (quantized)
    if (newClipIdx >= 0)
        scheduler.request({ slot, newClipIdx });

    // Update label immediately (reflect intent)
    const juce::String name = (newClipIdx >= 0 && newClipIdx < (int)pack.size())
        ? pack[(size_t)newClipIdx].getName() : juce::String();
    setSlotClipName(slot, name);
}

void DJAM0AudioProcessor::onSlotMuteParamChanged(int slot, bool mute)
{
    if (mute != slots[(size_t)slot].isMuted())
        slots[(size_t)slot].toggleMute();
}

void DJAM0AudioProcessor::onSlotSoloParamChanged(int slot, bool solo)
{
    slots[(size_t)slot].setSolo(solo);
}

//===================== Main render =====================

void DJAM0AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    juce::ignoreUnused(midi);

    if (!getHostPhase(getPlayHead(), hostPhase))
        hostPhase.isPlaying = false;

    hostPhase.sampleRate = getSampleRate();

    const bool anySolo = std::any_of(slots.begin(), slots.end(),
        [](const Slot& s) { return s.isSolo(); });

    const int total = buffer.getNumSamples();
    int remaining = total;
    int blockOffset = 0;

    while (remaining > 0)
    {
        const int toNextBar = (hostPhase.isPlaying ? samplesToNextBar(hostPhase) : remaining);
        const int step = std::min(remaining, std::max(1, toNextBar));
        const bool crosses = hostPhase.isPlaying && (step == toNextBar);

        juce::AudioBuffer<float> sub(buffer.getArrayOfWritePointers(),
            buffer.getNumChannels(),
            blockOffset, step);

        for (int i = 0; i < kNumSlots; ++i)
        {
            if (anySolo && !slots[(size_t)i].isSolo()) continue;
            if (slots[(size_t)i].isMuted())           continue;

            slots[(size_t)i].render(sub, 0, step, 0, hostPhase);
        }

        if (crosses)
        {
            // Commit requests exactly at bar boundary
            scheduler.flushAtBar([this](const StartRequest& r)
                {
                    if (r.slot >= 0 && r.slot < kNumSlots)
                        slots[(size_t)r.slot].armStart(r.clip);
                });

            // Apply armed starts
            for (auto& s : slots) s.applyArmedStart();

            // Reflect "reality" in labels (now actually playing)
            for (int i = 0; i < kNumSlots; ++i)
                setSlotClipName(i, slots[(size_t)i].getActiveClipName());
        }

        if (hostPhase.isPlaying)
        {
            hostPhase.currentSample += step;
            const double spb = hostPhase.sampleRate * 60.0 / hostPhase.bpm;
            hostPhase.ppqPosition += step / spb;
        }

        remaining -= step;
        blockOffset += step;
    }
}

//===================== Clip pack helpers =====================

juce::File DJAM0AudioProcessor::findResourceSamplesRoot() const
{
    DBG("findResourceSamplesRoot");

#if JUCE_MAC || JUCE_IOS
    auto resources = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
        .getChildFile("Contents").getChildFile("Resources");
#else
    // 🔧 Hardcoded override for Windows dev environment
    juce::File resources("C:\\_LocalFiles\\DJam\\Clips");

#endif

    DBG("samples root: " + resources.getFullPathName());
    return resources;
}


void DJAM0AudioProcessor::loadSamplePack()
{
    DBG("loadSamplePack");
    pack.clear();

    const auto root = findResourceSamplesRoot();
       
    if (!root.isDirectory())
        return;

    DBG("loading sample pack in folder: " + root.getFullPathName());

    juce::Array<juce::File> files;
    root.findChildFiles(files, juce::File::findFiles, true, "*.wav");
    pack.reserve((size_t)files.size());

    for (auto& f : files)
    {
        DJamClip c;
        c.loadFromFile(f, getSampleRate());
        if (c.isLoaded())
            pack.emplace_back(std::move(c));
    }

    // Rebind slots to the bank (in case 'pack' reallocated)
    for (auto& s : slots)
        s.setClipBank(&pack);
}

//===================== State save/restore =====================

void DJAM0AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    juce::MemoryOutputStream mos(destData, true);
    state.writeToStream(mos);
}

void DJAM0AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, (size_t)sizeInBytes);
    if (tree.isValid())
        apvts.replaceState(tree);
}

//===================== Utilities =====================

void DJAM0AudioProcessor::setSlotClipName(int slotIndex, const juce::String& name)
{
    if (slotIndex >= 0 && slotIndex < kNumSlots)
        apvts.state.setProperty(paramId_slotClipName(slotIndex), name, nullptr);
}

void DJAM0AudioProcessor::toneGen(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused(midi);
    static double phase = 0.0;

    auto* L = buffer.getWritePointer(0);
    auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    const double sr = getSampleRate();
    const double freq = 440.0;

    for (int n = 0; n < buffer.getNumSamples(); ++n)
    {
        float s = (float)std::sin(phase);
        phase += juce::MathConstants<double>::twoPi * freq / sr;
        if (phase >= juce::MathConstants<double>::twoPi) phase -= juce::MathConstants<double>::twoPi;

        L[n] = s * 0.1f;
        if (R) R[n] = s * 0.1f;
    }
}

//===================== Editor =====================

juce::AudioProcessorEditor* DJAM0AudioProcessor::createEditor()
{
    return new DJAM0AudioProcessorEditor(*this);
}
