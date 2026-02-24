/*
  ==============================================================================
    OscillatorSection.h - Oscillator controls (Ratio, Saw Phase, Sqr Phase)
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SectionPanel.h"
#include "ArcKnob.h"
#include "WaveformDisplay.h"

namespace gui
{

class OscillatorSection : public juce::Component
{
public:
    OscillatorSection(juce::AudioProcessorValueTreeState& apvts)
    {
        addAndMakeVisible(panel);
        addAndMakeVisible(ratioKnob);
        addAndMakeVisible(sawPhaseKnob);
        addAndMakeVisible(sqrPhaseKnob);
        addAndMakeVisible(waveformDisplay);

        // Attach to APVTS
        ratioAttach    = std::make_unique<SliderAttachment>(apvts, "oscRatio",   ratioKnob.getSlider());
        sawPhaseAttach = std::make_unique<SliderAttachment>(apvts, "sawPhase",   sawPhaseKnob.getSlider());
        sqrPhaseAttach = std::make_unique<SliderAttachment>(apvts, "sqrPhase",   sqrPhaseKnob.getSlider());
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        panel.setBounds(bounds);

        auto content = panel.getContentArea();

        // Knobs row
        auto knobRow = content.removeFromTop(80);
        const int knobWidth = knobRow.getWidth() / 3;
        ratioKnob.setBounds(knobRow.removeFromLeft(knobWidth));
        sawPhaseKnob.setBounds(knobRow.removeFromLeft(knobWidth));
        sqrPhaseKnob.setBounds(knobRow);

        // Waveform display
        content.removeFromTop(4);
        waveformDisplay.setBounds(content);
    }

    void setVisualizationBuffer(const juce::AudioBuffer<float>* buffer)
    {
        waveformDisplay.setBuffer(buffer);
    }

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    SectionPanel panel{ "OSCILLATOR" };

    ArcKnob ratioKnob   { "Ratio" };
    ArcKnob sawPhaseKnob{ "Saw \xCF\x86", "\xC2\xB0" };  // φ, °
    ArcKnob sqrPhaseKnob{ "Sqr \xCF\x86", "\xC2\xB0" };

    WaveformDisplay waveformDisplay;

    std::unique_ptr<SliderAttachment> ratioAttach;
    std::unique_ptr<SliderAttachment> sawPhaseAttach;
    std::unique_ptr<SliderAttachment> sqrPhaseAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorSection)
};

} // namespace gui
