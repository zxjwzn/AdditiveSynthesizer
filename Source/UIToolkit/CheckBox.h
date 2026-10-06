/*
  ==============================================================================
    CheckBox.h - Dark-themed checkbox with label
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

namespace gui
{

/**
 * A custom checkbox that draws a rounded-square box with
 * an accent-colored check mark and an optional label.
 *
 * Wraps a juce::ToggleButton and can be bound to a boolean
 * AudioProcessorValueTreeState parameter via ButtonAttachment.
 */
class CheckBox : public juce::Component
{
public:
    explicit CheckBox(const juce::String& labelText = "")
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
        const bool checked = button.getToggleState();

        // Box dimensions
        constexpr float boxSize = 16.0f;
        const float boxY = (bounds.getHeight() - boxSize) * 0.5f;
        const float boxX = bounds.getX() + 2.0f;
        const auto boxBounds = juce::Rectangle<float>(boxX, boxY, boxSize, boxSize);

        // Draw box background
        g.setColour(checked ? Colors::accent.withAlpha(0.2f) : Colors::knobBackground);
        g.fillRoundedRectangle(boxBounds, 3.0f);

        // Draw border
        g.setColour(checked ? Colors::accent : Colors::panelBorder);
        g.drawRoundedRectangle(boxBounds, 3.0f, 1.2f);

        // Draw check mark
        if (checked)
        {
            juce::Path checkPath;
            const float cx = boxX;
            const float cy = boxY;
            // Check mark: three points forming a ✓
            checkPath.startNewSubPath(cx + 3.5f, cy + boxSize * 0.50f);
            checkPath.lineTo(cx + boxSize * 0.40f, cy + boxSize * 0.72f);
            checkPath.lineTo(cx + boxSize - 3.5f, cy + boxSize * 0.30f);

            g.setColour(Colors::accent);
            g.strokePath(checkPath, juce::PathStrokeType(2.0f,
                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Label text
        if (label.isNotEmpty())
        {
            const float labelX = boxX + boxSize + 8.0f;
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

    /** Query the current checked state. */
    bool isChecked() const { return button.getToggleState(); }

    /** Programmatically set the state. */
    void setChecked(bool on, juce::NotificationType notification = juce::sendNotification)
    {
        button.setToggleState(on, notification);
        repaint();
    }

private:
    juce::ToggleButton button;
    juce::String label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CheckBox)
};

} // namespace gui
