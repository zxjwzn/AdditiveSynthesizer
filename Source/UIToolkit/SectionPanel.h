/*
  ==============================================================================
    SectionPanel.h - Rounded panel container with title
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

namespace gui
{

/**
 * A container component that draws a rounded rectangle background
 * with a section title label.
 */
class SectionPanel : public juce::Component
{
public:
    explicit SectionPanel(const juce::String& title)
        : sectionTitle(title)
    {
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(1.0f);

        // Panel background with rounded corners
        g.setColour(Colors::panelBackground);
        g.fillRoundedRectangle(bounds, 8.0f);

        // Subtle border
        g.setColour(Colors::panelBorder.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

        // Section title
        g.setColour(Colors::accent);
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText(sectionTitle, bounds.reduced(12.0f, 6.0f).removeFromTop(20.0f),
                   juce::Justification::centredLeft);
    }

    /** Get the content area (below the title). */
    juce::Rectangle<int> getContentArea() const
    {
        return getLocalBounds().reduced(8, 4).withTrimmedTop(22);
    }

private:
    juce::String sectionTitle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SectionPanel)
};

} // namespace gui
