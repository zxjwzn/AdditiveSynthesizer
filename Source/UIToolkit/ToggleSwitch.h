/*
  ==============================================================================
    ToggleSwitch.h - iOS-style toggle switch with label
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

namespace gui
{

/**
 * An iOS/Material-style toggle switch with an optional label on the right.
 *
 * Wraps a juce::ToggleButton but draws a custom pill-shaped track
 * with an animated circular thumb. Can be bound to a boolean
 * AudioProcessorValueTreeState parameter via ButtonAttachment.
 */
class ToggleSwitch : public juce::Component
{
public:
    explicit ToggleSwitch(const juce::String& labelText = "")
        : label(labelText)
    {
        button.setClickingTogglesState(true);
        button.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::transparentBlack);
        button.setColour(juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(button);

        // Custom painting — button is invisible but handles mouse / state
        button.setAlpha(0.0f);
        button.onClick = [this]() { repaint(); };
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat();
        const bool isOn = button.getToggleState();

        // Track dimensions
        constexpr float trackHeight = 18.0f;
        constexpr float trackWidth  = 34.0f;
        const float trackY = (bounds.getHeight() - trackHeight) * 0.5f;
        const float trackX = bounds.getX();

        const auto trackBounds = juce::Rectangle<float>(trackX, trackY, trackWidth, trackHeight);
        const float trackRadius = trackHeight * 0.5f;

        // Draw track
        g.setColour(isOn ? Colors::accent.withAlpha(0.5f) : Colors::knobArcTrack);
        g.fillRoundedRectangle(trackBounds, trackRadius);

        // Draw thumb
        constexpr float thumbDiameter = 14.0f;
        const float thumbY = trackY + (trackHeight - thumbDiameter) * 0.5f;
        const float thumbXOff = trackX + 2.0f;
        const float thumbXOn  = trackX + trackWidth - thumbDiameter - 2.0f;
        const float thumbX = isOn ? thumbXOn : thumbXOff;

        g.setColour(isOn ? Colors::accent : Colors::textDim);
        g.fillEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter);

        // Subtle border on thumb
        g.setColour(Colors::knobThumb.withAlpha(0.3f));
        g.drawEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter, 0.8f);

        // Label text
        if (label.isNotEmpty())
        {
            const float labelX = trackX + trackWidth + 8.0f;
            g.setColour(Colors::textBright);
            g.setFont(juce::FontOptions(11.0f));
            g.drawText(label,
                       static_cast<int>(labelX),
                       0,
                       static_cast<int>(bounds.getWidth() - labelX),
                       static_cast<int>(bounds.getHeight()),
                       juce::Justification::centredLeft);
        }
    }

    void resized() override
    {
        button.setBounds(getLocalBounds());
    }

    /** Access the underlying ToggleButton (for ButtonAttachment). */
    juce::ToggleButton& getButton() { return button; }

    /** Query the current on/off state. */
    bool isOn() const { return button.getToggleState(); }

    /** Programmatically set the state. */
    void setOn(bool on, juce::NotificationType notification = juce::sendNotification)
    {
        button.setToggleState(on, notification);
        repaint();
    }

private:
    juce::ToggleButton button;
    juce::String label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToggleSwitch)
};

} // namespace gui
