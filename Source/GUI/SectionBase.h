/*
  ==============================================================================
    SectionBase.h - Base class for synth UI sections (panel + knob strip)
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SectionPanel.h"
#include "KnobStrip.h"

namespace gui
{

/**
 * Base class for a synth section.
 *
 * Provides:
 *   - A titled SectionPanel background
 *   - A KnobStrip row with automatic APVTS binding
 *   - A virtual resizeContent() hook for subclass-specific displays
 *
 * Subclasses only need to supply the section title, knob descriptors,
 * and optionally override resizeContent() to lay out extra components.
 *
 * @param knobRowHeight  Height in pixels for the knob row.
 *                       Pass 0 to let the knobs fill the entire content area
 *                       (useful when there is no additional content below).
 */
class SectionBase : public juce::Component
{
public:
    SectionBase(const juce::String& title,
                juce::AudioProcessorValueTreeState& apvts,
                const std::vector<KnobDescriptor>& knobDescriptors,
                int knobRowHeight = 80)
        : panel(title), apvtsRef(apvts), knobHeight(knobRowHeight)
    {
        addAndMakeVisible(panel);
        knobStrip.init(apvts, knobDescriptors);
        addAndMakeVisible(knobStrip);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        panel.setBounds(bounds);

        auto content = panel.getContentArea();

        if (knobStrip.getNumKnobs() > 0)
        {
            if (knobHeight > 0)
            {
                knobStrip.setBounds(content.removeFromTop(knobHeight));
                content.removeFromTop(4); // gap between knobs and content
            }
            else
            {
                // knobHeight == 0 → knobs fill the entire content area
                knobStrip.setBounds(content);
                content = {};
            }
        }

        resizeContent(content);
    }

    /** Access the knob strip (e.g. to retrieve a specific knob by index). */
    KnobStrip& getKnobStrip() { return knobStrip; }

protected:
    /**
     * Override this to lay out section-specific components
     * (displays, buttons, etc.) in the area below the knob row.
     */
    virtual void resizeContent(juce::Rectangle<int> /*contentArea*/) {}

    /** Convenience accessor for the APVTS reference. */
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvtsRef; }

private:
    SectionPanel panel;
    KnobStrip knobStrip;
    juce::AudioProcessorValueTreeState& apvtsRef;
    int knobHeight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SectionBase)
};

} // namespace gui
