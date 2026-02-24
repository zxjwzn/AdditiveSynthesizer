/*
  ==============================================================================
    UnisonOutputSection.h - Unison + stereo width + master gain controls
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SectionPanel.h"
#include "ArcKnob.h"

namespace gui
{

class UnisonOutputSection : public juce::Component
{
public:
    UnisonOutputSection(juce::AudioProcessorValueTreeState& apvts)
    {
        addAndMakeVisible(panel);
        addAndMakeVisible(voicesKnob);
        addAndMakeVisible(detuneKnob);
        addAndMakeVisible(widthKnob);
        addAndMakeVisible(gainKnob);

        voicesAttach = std::make_unique<SliderAttachment>(apvts, "unisonCount",  voicesKnob.getSlider());
        detuneAttach = std::make_unique<SliderAttachment>(apvts, "unisonDetune", detuneKnob.getSlider());
        widthAttach  = std::make_unique<SliderAttachment>(apvts, "stereoWidth",  widthKnob.getSlider());
        gainAttach   = std::make_unique<SliderAttachment>(apvts, "masterGain",   gainKnob.getSlider());
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        panel.setBounds(bounds);

        auto content = panel.getContentArea();

        // All knobs in one row, with gain separated
        auto knobRow = content;
        const int knobWidth = knobRow.getWidth() / 4;
        voicesKnob.setBounds(knobRow.removeFromLeft(knobWidth));
        detuneKnob.setBounds(knobRow.removeFromLeft(knobWidth));
        widthKnob.setBounds(knobRow.removeFromLeft(knobWidth));
        gainKnob.setBounds(knobRow);
    }

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    SectionPanel panel{ "UNISON & OUTPUT" };

    ArcKnob voicesKnob{ "Voices" };
    ArcKnob detuneKnob{ "Detune", "ct" };
    ArcKnob widthKnob { "Width" };
    ArcKnob gainKnob  { "Gain", "dB" };

    std::unique_ptr<SliderAttachment> voicesAttach;
    std::unique_ptr<SliderAttachment> detuneAttach;
    std::unique_ptr<SliderAttachment> widthAttach;
    std::unique_ptr<SliderAttachment> gainAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnisonOutputSection)
};

} // namespace gui
