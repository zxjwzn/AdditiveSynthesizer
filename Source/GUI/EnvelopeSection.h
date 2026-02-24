/*
  ==============================================================================
    EnvelopeSection.h - ADSR envelope controls with curve visualization
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SectionBase.h"
#include "ADSRDisplay.h"

namespace gui
{

class EnvelopeSection : public SectionBase, public juce::Timer
{
public:
    EnvelopeSection(juce::AudioProcessorValueTreeState& apvts)
        : SectionBase("ADSR ENVELOPE", apvts, {
              { "Attack",  "s", "envAttack" },
              { "Decay",   "s", "envDecay" },
              { "Sustain", "",  "envSustain" },
              { "Release", "s", "envRelease" }
          })
    {
        addAndMakeVisible(adsrDisplay);
        startTimerHz(15);
    }

    void timerCallback() override
    {
        auto& apvts = getAPVTS();
        const float a = apvts.getRawParameterValue("envAttack")->load();
        const float d = apvts.getRawParameterValue("envDecay")->load();
        const float s = apvts.getRawParameterValue("envSustain")->load();
        const float r = apvts.getRawParameterValue("envRelease")->load();
        adsrDisplay.setParameters(a, d, s, r);
    }

protected:
    void resizeContent(juce::Rectangle<int> area) override
    {
        adsrDisplay.setBounds(area);
    }

private:
    ADSRDisplay adsrDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeSection)
};

} // namespace gui
