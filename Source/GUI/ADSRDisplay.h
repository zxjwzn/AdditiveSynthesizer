/*
  ==============================================================================
    ADSRDisplay.h - ADSR envelope curve visualization
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

namespace gui
{

/**
 * Visualizes the ADSR envelope as a bezier curve path.
 * Updated when ADSR parameters change.
 */
class ADSRDisplay : public juce::Component
{
public:
    ADSRDisplay() = default;

    void setParameters(float attack, float decay, float sustain, float release)
    {
        a = attack;
        d = decay;
        s = sustain;
        r = release;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        // Background
        g.setColour(Colors::knobBackground);
        g.fillRoundedRectangle(bounds, 4.0f);

        const float w = bounds.getWidth();
        const float h = bounds.getHeight() - 4.0f;
        const float baseY = bounds.getBottom() - 2.0f;
        const float startX = bounds.getX() + 2.0f;

        // Calculate segment widths proportionally
        const float totalTime = a + d + 0.3f + r; // 0.3s for sustain hold display
        const float aW = (a / totalTime) * (w - 4.0f);
        const float dW = (d / totalTime) * (w - 4.0f);
        const float sW = (0.3f / totalTime) * (w - 4.0f);
        const float rW = (r / totalTime) * (w - 4.0f);

        juce::Path envPath;
        envPath.startNewSubPath(startX, baseY);

        // Attack: curve from 0 to peak
        const float aPeakX = startX + aW;
        const float aPeakY = baseY - h;
        envPath.quadraticTo(startX + aW * 0.3f, baseY - h * 0.7f,
                            aPeakX, aPeakY);

        // Decay: curve from peak to sustain level
        const float sustainY = baseY - h * s;
        const float dEndX = aPeakX + dW;
        envPath.quadraticTo(aPeakX + dW * 0.3f, aPeakY + (sustainY - aPeakY) * 0.3f,
                            dEndX, sustainY);

        // Sustain: flat line
        const float sEndX = dEndX + sW;
        envPath.lineTo(sEndX, sustainY);

        // Release: curve from sustain to zero
        const float rEndX = sEndX + rW;
        envPath.quadraticTo(sEndX + rW * 0.3f, sustainY + (baseY - sustainY) * 0.3f,
                            rEndX, baseY);

        // Draw filled area
        juce::Path filledPath(envPath);
        filledPath.lineTo(rEndX, baseY);
        filledPath.lineTo(startX, baseY);
        filledPath.closeSubPath();

        g.setColour(Colors::envelopeYellow.withAlpha(0.15f));
        g.fillPath(filledPath);

        // Draw curve stroke
        g.setColour(Colors::envelopeYellow);
        g.strokePath(envPath, juce::PathStrokeType(2.0f));

        // Draw stage labels
        g.setColour(Colors::textDim.withAlpha(0.5f));
        g.setFont(juce::FontOptions(8.0f));

        const float labelY = baseY + 1.0f;
        g.drawText("A", static_cast<int>(startX), static_cast<int>(labelY - 12),
                   static_cast<int>(aW), 10, juce::Justification::centred);
        g.drawText("D", static_cast<int>(aPeakX), static_cast<int>(labelY - 12),
                   static_cast<int>(dW), 10, juce::Justification::centred);
        g.drawText("S", static_cast<int>(dEndX), static_cast<int>(labelY - 12),
                   static_cast<int>(sW), 10, juce::Justification::centred);
        g.drawText("R", static_cast<int>(sEndX), static_cast<int>(labelY - 12),
                   static_cast<int>(rW), 10, juce::Justification::centred);
    }

private:
    float a = 0.01f;
    float d = 0.1f;
    float s = 0.8f;
    float r = 0.3f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ADSRDisplay)
};

} // namespace gui
