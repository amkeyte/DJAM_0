#include "SlotRow.h"
#include "PluginProcessor.h"

SlotRow::SlotRow(DJAM0AudioProcessor& proc, int slotIndex)
    : processor(proc), slot(slotIndex)
{
    auto& apvts = processor.getAPVTS();
    auto& vt = apvts.state;

    // Title
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    // Clip name
    clipNameLabel.setJustificationType(juce::Justification::centredLeft);
    clipNameLabel.setEditable(false, false, false);
    addAndMakeVisible(clipNameLabel);

    // Loop info label (bind to our *combined* Value)
    clipLoopInfoLabel.setJustificationType(juce::Justification::centredLeft);
    clipLoopInfoLabel.getTextValue().referTo(clipLoopInfoValue);
    addAndMakeVisible(clipLoopInfoLabel);

    // Slider & buttons
    clipIndexSlider.setSliderStyle(juce::Slider::IncDecButtons);
    clipIndexSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 60, 20);
    addAndMakeVisible(clipIndexSlider);

    muteButton.setTooltip("Mute");
    soloButton.setTooltip("Solo");
    addAndMakeVisible(muteButton);
    addAndMakeVisible(soloButton);

    // Attach parameters
    clipAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, paramId_slotClip(slot), clipIndexSlider);
    muteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, paramId_slotMute(slot), muteButton);
    soloAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, paramId_slotSolo(slot), soloButton);

    // Bind the clip name label to the ValueTree property (auto-updates)
    clipNameLabel.getTextValue().referTo(vt.getPropertyAsValue(paramId_slotClipName(slot), nullptr));

    // Bind the *sources* we want to combine
    clipLenBarsValue.referTo(vt.getPropertyAsValue(paramId_slotClipLenBars(slot), nullptr));
    clipSamplesValue.referTo(vt.getPropertyAsValue(paramId_slotClipSamplesAt(slot), nullptr));

    // Listen for changes on both sources
    clipLenBarsValue.addListener(this);
    clipSamplesValue.addListener(this);

    // Set initial title & combined text
    auto clipName = vt.getProperty(paramId_slotClipName(slot)).toString();
    if (clipName.isEmpty()) clipName = "(empty)";
    titleLabel.setText("Slot " + juce::String(slot + 1) + ": " + clipName, juce::dontSendNotification);

    updateClipLoopInfoText(); // initialize combined label
}

SlotRow::~SlotRow()
{
    // Remove listeners to avoid dangling callbacks
    clipLenBarsValue.removeListener(this);
    clipSamplesValue.removeListener(this);
}

void SlotRow::valueChanged(juce::Value& /*v*/)
{
    updateClipLoopInfoText();
}

void SlotRow::updateClipLoopInfoText()
{
    auto barsStr = clipLenBarsValue.toString();
    auto samplesStr = clipSamplesValue.toString();

    if (barsStr.isEmpty())    barsStr = "##";
    if (samplesStr.isEmpty()) samplesStr = "########";

    // Update the combined Value once; Label updates automatically via referTo()
    clipLoopInfoValue = "Bars: " + barsStr + " | Samples: " + samplesStr;
}


void SlotRow::resized()
{
    auto r = getLocalBounds().reduced(4);      // Leaves 8px horizontal padding
    auto col1 = r.removeFromLeft(88);          // "Slot 1:"
    auto col6 = r.removeFromRight(30);         // Solo button (rightmost)
    auto col5 = r.removeFromRight(30);         // Mute button
    auto col4 = r.removeFromRight(90);         // Slider
    auto col3 = r.removeFromRight(160);        // Loop info: "Bars: x | Samples: y"
    auto col2 = r;                              // Remaining = Clip name

    titleLabel.setBounds(col1);
    clipNameLabel.setBounds(col2);
    clipLoopInfoLabel.setBounds(col3);
    clipIndexSlider.setBounds(col4);
    muteButton.setBounds(col5.reduced(2));
    soloButton.setBounds(col6.reduced(2));
}
