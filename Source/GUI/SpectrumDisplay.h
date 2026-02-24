/*
  ==============================================================================
    SpectrumDisplay.h - Harmonic spectrum bar chart visualization
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

namespace gui
{

/** Lightweight data struct for passing spectrum magnitudes into the display. */
struct SpectrumData
{
    const float* amplitudes = nullptr; ///< Pointer to amplitude array (NOT owned)
    int          count      = 0;      ///< Number of valid elements in `amplitudes`
};

/**
 * Displays the harmonic spectrum as a bar chart.
 * Shows up to 256 harmonic amplitudes with a filter curve overlay.
 */
class SpectrumDisplay : public juce::Component, public juce::Timer
{
public:
    SpectrumDisplay()
    {
        startTimerHz(20);
    }

    /** Set the spectrum data to visualize. */
    void setSpectrumData(const SpectrumData& data)
    {
        specData = data;
    }

    /** Set spectral filter parameters for drawing the filter curve overlay. */
    void setFilterParams(float cutoff, float boost, float stretch)
    {
        filterCutoff = cutoff;
        filterBoost = boost;
        filterStretch = stretch;
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        // Background
        g.setColour(Colors::knobBackground);
        g.fillRoundedRectangle(bounds, 4.0f);

        const float width = bounds.getWidth();
        const float height = bounds.getHeight();
        const int maxBars = 128; // Show up to 128 bars (higher harmonics are tiny)

        if (specData.amplitudes == nullptr || specData.count == 0)
        {
            g.setColour(Colors::textDim.withAlpha(0.4f));
            g.setFont(juce::FontOptions(10.0f));
            g.drawText("No harmonics active", bounds, juce::Justification::centred);
            return;
        }

        const int numBars = juce::jmin(maxBars, specData.count);
        const float barWidth = width / static_cast<float>(numBars);

        // Find max amplitude for normalization
        float maxAmp = 0.001f;
        for (int i = 0; i < numBars; ++i)
            maxAmp = juce::jmax(maxAmp, specData.amplitudes[i]);

        // Draw bars
        for (int i = 0; i < numBars; ++i)
        {
            const float amp = specData.amplitudes[i] / maxAmp;
            const float barHeight = amp * (height - 4.0f);
            const float x = bounds.getX() + static_cast<float>(i) * barWidth;
            const float y = bounds.getBottom() - barHeight - 2.0f;

            // Gradient from accent to cyan based on harmonic number
            const float t = static_cast<float>(i) / static_cast<float>(numBars);
            const auto barColor = Colors::spectrumCyan.interpolatedWith(Colors::accent, t);

            g.setColour(barColor.withAlpha(0.7f));
            g.fillRect(x + 0.5f, y, juce::jmax(1.0f, barWidth - 1.0f), barHeight);
        }

        // Draw filter cutoff line
        {
            const float cutoffX = bounds.getX()
                                  + (filterCutoff / static_cast<float>(maxBars)) * width;
            g.setColour(Colors::accent.withAlpha(0.6f));
            g.drawVerticalLine(static_cast<int>(cutoffX),
                               bounds.getY(), bounds.getBottom());

            // Small label
            g.setFont(juce::FontOptions(9.0f));
            g.drawText(juce::String(static_cast<int>(filterCutoff)),
                       static_cast<int>(cutoffX) + 2, static_cast<int>(bounds.getY()) + 2,
                       30, 12, juce::Justification::centredLeft);
        }
    }

    void timerCallback() override
    {
        repaint();
    }

private:
    SpectrumData specData;
    float filterCutoff = 128.0f;
    float filterBoost = 0.0f;
    float filterStretch = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumDisplay)
};

} // namespace gui
