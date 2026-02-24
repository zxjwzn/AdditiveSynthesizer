/*
  ==============================================================================
    WaveformDisplay.h - Real-time waveform visualization
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

namespace gui
{

/**
 * Displays a real-time waveform from an audio buffer.
 * Draws a filled path with gradient coloring.
 */
class WaveformDisplay : public juce::Component, public juce::Timer
{
public:
    WaveformDisplay()
    {
        startTimerHz(30);
    }

    void setBuffer(const juce::AudioBuffer<float>* bufferToVisualize)
    {
        sourceBuffer = bufferToVisualize;
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        // Background
        g.setColour(Colors::knobBackground);
        g.fillRoundedRectangle(bounds, 4.0f);

        if (sourceBuffer == nullptr || sourceBuffer->getNumSamples() == 0)
        {
            g.setColour(Colors::textDim.withAlpha(0.4f));
            g.setFont(juce::FontOptions(10.0f));
            g.drawText("No signal", bounds, juce::Justification::centred);
            return;
        }

        // Draw waveform
        const auto* data = sourceBuffer->getReadPointer(0);
        const int numSamples = sourceBuffer->getNumSamples();
        const float width = bounds.getWidth();
        const float height = bounds.getHeight();
        const float centreY = bounds.getCentreY();
        const float halfHeight = height * 0.45f;

        juce::Path wavePath;
        wavePath.startNewSubPath(bounds.getX(), centreY);

        const float samplesPerPixel = static_cast<float>(numSamples) / width;

        for (float px = 0; px < width; px += 1.0f)
        {
            const int sampleIdx = juce::jmin(
                static_cast<int>(px * samplesPerPixel), numSamples - 1);
            const float y = centreY - data[sampleIdx] * halfHeight;
            wavePath.lineTo(bounds.getX() + px, y);
        }

        // Stroke the waveform
        juce::ColourGradient gradient(Colors::waveformGreen.withAlpha(0.9f),
                                       bounds.getX(), centreY,
                                       Colors::waveformGreen.withAlpha(0.4f),
                                       bounds.getRight(), centreY, false);
        g.setGradientFill(gradient);
        g.strokePath(wavePath, juce::PathStrokeType(1.5f));

        // Centre line
        g.setColour(Colors::textDim.withAlpha(0.2f));
        g.drawHorizontalLine(static_cast<int>(centreY), bounds.getX(), bounds.getRight());
    }

    void timerCallback() override
    {
        repaint();
    }

private:
    const juce::AudioBuffer<float>* sourceBuffer = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};

} // namespace gui
