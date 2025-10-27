#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

class SlotRow : public juce::Component,
    public juce::Value::Listener
{
public:
    SlotRow(DJAM0AudioProcessor& proc, int slotIndex);
    ~SlotRow() override;

    void resized() override;

private:
    void valueChanged(juce::Value& v) override; // from Value::Listener
    void updateClipLoopInfoText();

    DJAM0AudioProcessor& processor;
    int slot = 0;

    juce::Label titleLabel, clipNameLabel, clipLoopInfo;
    juce::Slider clipIndexSlider;
    juce::ToggleButton muteButton, soloButton;

    // Bindings
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> clipAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAttachment, soloAttachment;

    // Values for binding/combining
    juce::Value clipLenBarsValue;     // refers to paramId_slotClipLenBars(slot)
    juce::Value clipSamplesValue;     // refers to paramId_slotClipSamplesAt(slot)
    juce::Value clipLoopInfoValue;    // combined -> label refers to this
};
