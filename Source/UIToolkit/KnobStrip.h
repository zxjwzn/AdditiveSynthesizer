/*
  ==============================================================================
    KnobStrip.h - A row of ArcKnobs with automatic APVTS binding and layout
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ArcKnob.h"
#include <vector>

namespace gui
{

/**
 * Descriptor for a single knob: display label, value suffix, and APVTS parameter ID.
 */
struct KnobDescriptor
{
    juce::String label;
    juce::String suffix;
    juce::String paramId;
};

/**
 * A component that manages a horizontal row of ArcKnobs.
 *
 * Knobs are created from a list of KnobDescriptors and automatically
 * attached to the corresponding APVTS parameters. Layout distributes
 * knobs equally across the available width.
 */
class KnobStrip : public juce::Component
{
public:
    KnobStrip() = default;

    /**
     * Initialize the strip with knob descriptors and bind to an APVTS.
     * Call this once in the parent's constructor.
     */
    void init(juce::AudioProcessorValueTreeState& apvts,
              const std::vector<KnobDescriptor>& descriptors)
    {
        for (const auto& desc : descriptors)
        {
            auto knob = std::make_unique<ArcKnob>(desc.label, desc.suffix);
            addAndMakeVisible(*knob);

            auto attachment = std::make_unique<
                juce::AudioProcessorValueTreeState::SliderAttachment>(
                apvts, desc.paramId, knob->getSlider());

            knobs.push_back(std::move(knob));
            attachments.push_back(std::move(attachment));
        }
    }

    void resized() override
    {
        if (knobs.empty())
            return;

        auto bounds = getLocalBounds();
        const int knobWidth = bounds.getWidth() / static_cast<int>(knobs.size());

        for (size_t i = 0; i < knobs.size(); ++i)
        {
            if (i < knobs.size() - 1)
                knobs[i]->setBounds(bounds.removeFromLeft(knobWidth));
            else
                knobs[i]->setBounds(bounds); // last knob takes remaining width
        }
    }

    /** Access a knob by index. */
    ArcKnob& getKnob(int index) { return *knobs[static_cast<size_t>(index)]; }

    /** Number of knobs in this strip. */
    int getNumKnobs() const { return static_cast<int>(knobs.size()); }

private:
    std::vector<std::unique_ptr<ArcKnob>> knobs;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KnobStrip)
};

} // namespace gui
