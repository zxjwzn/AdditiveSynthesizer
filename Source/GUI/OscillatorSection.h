/*
  ==============================================================================
    OscillatorSection.h - Oscillator controls (Ratio, Saw Phase, Sqr Phase)
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SectionBase.h"
#include "WaveformDisplay.h"

namespace gui
{

class OscillatorSection : public SectionBase
{
public:
    OscillatorSection(juce::AudioProcessorValueTreeState& apvts)
        : SectionBase("OSCILLATOR", apvts, {
              { "Ratio",  "",  "oscRatio" },
              { juce::String(juce::CharPointer_UTF8("Saw \xcf\x86")), "", "sawPhase" },
              { juce::String(juce::CharPointer_UTF8("Sqr \xcf\x86")), "", "sqrPhase" }
          })
    {
        addAndMakeVisible(waveformDisplay);
    }

    void setVisualizationBuffer(const juce::AudioBuffer<float>* buffer)
    {
        waveformDisplay.setBuffer(buffer);
    }

protected:
    void resizeContent(juce::Rectangle<int> area) override
    {
        waveformDisplay.setBounds(area);
    }

private:
    WaveformDisplay waveformDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorSection)
};

} // namespace gui
