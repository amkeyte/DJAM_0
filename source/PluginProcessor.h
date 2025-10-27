#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <vector>

#include "DJamHostSync.h"
#include "QuantizedScheduler.h"
#include "DJamClip.h"
#include "Slot.h"

// Forward-declare the editor
class DJAM0AudioProcessorEditor;

// -------- Shared parameter IDs --------
static inline juce::String paramId_slotClip(int i) { return "slot" + juce::String(i) + "_clip"; }
static inline juce::String paramId_slotClipName(int i) { return "slot" + juce::String(i) + "_clipName"; }
static inline juce::String paramId_slotClipLenBars(int i) { return "slot" + juce::String(i) + "_clipLenBars"; }
static inline juce::String paramId_slotClipSamplesAt(int i) { return "slot" + juce::String(i) + "_clipSamplesAt"; }
static inline juce::String paramId_slotMute(int i) { return "slot" + juce::String(i) + "_mute"; }
static inline juce::String paramId_slotSolo(int i) { return "slot" + juce::String(i) + "_solo"; }

class DJAM0AudioProcessor
    : public juce::AudioProcessor
    , public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==========================================================================
    DJAM0AudioProcessor();
    ~DJAM0AudioProcessor() override;

    // AudioProcessor overrides
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void toneGen(juce::AudioBuffer<float>&, juce::MidiBuffer&);

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "D-Jam"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Accessors
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    static constexpr int getNumSlots() { return kNumSlots; }

    // UI label sync (stores to APVTS.state property)
    void setSlotClipName(int slotIndex, const juce::String& name);

    

    // APVTS param change listener
    void parameterChanged(const juce::String& paramID, float newValue) override;

private:
    //==========================================================================
    static constexpr int kNumSlots = 8;

    // Params
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Engine
    std::vector<DJamClip>           pack;   // loaded clips (bank)
    std::array<Slot, kNumSlots>     slots;  // performer channels
    QuantizedScheduler              scheduler;
    HostPhase                       hostPhase{};

    // Helpers
    juce::File findResourceSamplesRoot() const;
    void loadSamplePack();

    // Param reactions (working-state only)
    void onSlotClipParamChanged(int slot, int newClipIdx);
    void onSlotMuteParamChanged(int slot, bool mute);
    void onSlotSoloParamChanged(int slot, bool solo);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DJAM0AudioProcessor)
};
