/*
  ==============================================================================
    EnvelopeSection.h - ADSR envelope controls with curve visualization
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SectionPanel.h"
#include "ArcKnob.h"
#include "ADSRDisplay.h"

namespace gui
{

class EnvelopeSection : public juce::Component, public juce::Timer
{
public:
    EnvelopeSection(juce::AudioProcessorValueTreeState& apvts)
        : apvtsRef(apvts)
    {
        addAndMakeVisible(panel);
        addAndMakeVisible(attackKnob);
        addAndMakeVisible(decayKnob);
        addAndMakeVisible(sustainKnob);
        addAndMakeVisible(releaseKnob);
        addAndMakeVisible(adsrDisplay);

        attackAttach  = std::make_unique<SliderAttachment>(apvts, "envAttack",  attackKnob.getSlider());
        decayAttach   = std::make_unique<SliderAttachment>(apvts, "envDecay",   decayKnob.getSlider());
        sustainAttach = std::make_unique<SliderAttachment>(apvts, "envSustain", sustainKnob.getSlider());
        releaseAttach = std::make_unique<SliderAttachment>(apvts, "envRelease", releaseKnob.getSlider());

        startTimerHz(15);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        panel.setBounds(bounds);

        auto content = panel.getContentArea();

        // Knobs row
        auto knobRow = content.removeFromTop(80);
        const int knobWidth = knobRow.getWidth() / 4;
        attackKnob.setBounds(knobRow.removeFromLeft(knobWidth));
        decayKnob.setBounds(knobRow.removeFromLeft(knobWidth));
        sustainKnob.setBounds(knobRow.removeFromLeft(knobWidth));
        releaseKnob.setBounds(knobRow);

        // ADSR display
        content.removeFromTop(4);
        adsrDisplay.setBounds(content);
    }

    void timerCallback() override
    {
        const float a = apvtsRef.getRawParameterValue("envAttack")->load();
        const float d = apvtsRef.getRawParameterValue("envDecay")->load();
        const float s = apvtsRef.getRawParameterValue("envSustain")->load();
        const float r = apvtsRef.getRawParameterValue("envRelease")->load();
        adsrDisplay.setParameters(a, d, s, r);
    }

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    juce::AudioProcessorValueTreeState& apvtsRef;

    SectionPanel panel{ "ADSR ENVELOPE" };

    ArcKnob attackKnob { "Attack", "s" };
    ArcKnob decayKnob  { "Decay", "s" };
    ArcKnob sustainKnob{ "Sustain" };
    ArcKnob releaseKnob{ "Release", "s" };

    ADSRDisplay adsrDisplay;

    std::unique_ptr<SliderAttachment> attackAttach;
    std::unique_ptr<SliderAttachment> decayAttach;
    std::unique_ptr<SliderAttachment> sustainAttach;
    std::unique_ptr<SliderAttachment> releaseAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeSection)
};

} // namespace gui
