#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "SlotRow.h"

/**
 * The main plugin editor UI for D-Jam.
 */
class DJAM0AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit DJAM0AudioProcessorEditor(DJAM0AudioProcessor& p);
    ~DJAM0AudioProcessorEditor() override = default;

    void resized() override;

private:
    DJAM0AudioProcessor& processor;

    juce::Label titleLabel;
    //juce::ComboBox sceneSelect;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> sceneAttachment;

    juce::OwnedArray<SlotRow> slotRows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DJAM0AudioProcessorEditor)
};
