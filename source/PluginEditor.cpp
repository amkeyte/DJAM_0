#include "PluginEditor.h"


void SlotRow::resized()
{
    auto r = getLocalBounds().reduced(4);
    auto col1 = r.removeFromLeft(100);  // title
    auto col3 = r.removeFromRight(120); // index slider
    auto col4 = r.removeFromRight(30);  // mute
    auto col5 = r.removeFromRight(30);  // solo

    // Whatever is left between title and slider becomes clip name
    auto col2 = r;

    titleLabel.setBounds(col1);
    clipNameLabel.setBounds(col2);
    clipIndexSlider.setBounds(col3);
    muteButton.setBounds(col4.reduced(2));
    soloButton.setBounds(col5.reduced(2));
}

//=============================================
// DJAM0AudioProcessorEditor Implementation
//=============================================
DJAM0AudioProcessorEditor::DJAM0AudioProcessorEditor(DJAM0AudioProcessor& p)
    : juce::AudioProcessorEditor(&p), processor(p)
{
    // Window title
    titleLabel.setText("D-Jam Performance Mixer", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Build rows dynamically from kNumSlots
    for (int s = 0; s < DJAM0AudioProcessor::getNumSlots(); ++s)
        slotRows.add(new SlotRow(processor, s)), addAndMakeVisible(slotRows.getLast());

    setSize(650, 60 + DJAM0AudioProcessor::getNumSlots() * 36);
}

void DJAM0AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(8);

    // Title at top
    titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(4);


    // Slot rows
    for (auto* row : slotRows)
        row->setBounds(area.removeFromTop(34));
}
