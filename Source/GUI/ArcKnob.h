/*
  ==============================================================================
    ArcKnob.h - Custom arc-style rotary knob with value display
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

namespace gui
{

/**
 * A custom rotary knob that draws:
 *   - A dark circle background
 *   - A 270° arc track (dimmed)
 *   - A colored arc for the current value
 *   - Current value text at center
 *   - Parameter label below
 */
class ArcKnob : public juce::Component
{
public:
    ArcKnob(const juce::String& labelText, const juce::String& suffix = "")
        : label(labelText), valueSuffix(suffix)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setColour(juce::Slider::rotarySliderFillColourId, Colors::knobArcActive);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, Colors::knobArcTrack);
        addAndMakeVisible(slider);

        // Custom painting — we override paint, slider is invisible but handles mouse
        slider.setAlpha(0.0f);
        slider.addListener(this);
    }

    ~ArcKnob() override
    {
        slider.removeListener(this);
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat();
        const float knobSize = juce::jmin(bounds.getWidth(), bounds.getHeight() - 18.0f);
        const float radius = knobSize * 0.42f;
        const float arcThickness = 3.5f;
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getY() + knobSize * 0.5f;

        constexpr float startAngle = juce::MathConstants<float>::pi * 1.25f;  // -225°
        constexpr float endAngle = juce::MathConstants<float>::pi * 2.75f;    // +45°

        // Knob background circle
        g.setColour(Colors::knobBackground);
        g.fillEllipse(centreX - radius - 4.0f, centreY - radius - 4.0f,
                       (radius + 4.0f) * 2.0f, (radius + 4.0f) * 2.0f);

        // Background arc track
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                            startAngle, endAngle, true);
        g.setColour(Colors::knobArcTrack);
        g.strokePath(bgArc, juce::PathStrokeType(arcThickness,
                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Active arc
        const float proportion = static_cast<float>(slider.valueToProportionOfLength(slider.getValue()));
        const float currentAngle = startAngle + proportion * (endAngle - startAngle);

        if (proportion > 0.001f)
        {
            juce::Path activeArc;
            activeArc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                                    startAngle, currentAngle, true);

            juce::ColourGradient gradient(Colors::accentDark, centreX - radius, centreY,
                                          Colors::accent, centreX + radius, centreY, false);
            g.setGradientFill(gradient);
            g.strokePath(activeArc, juce::PathStrokeType(arcThickness,
                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Thumb dot
        {
            const float thumbRadius = 3.0f;
            const float thumbX = centreX + (radius) * std::cos(currentAngle);
            const float thumbY = centreY + (radius) * std::sin(currentAngle);
            g.setColour(Colors::knobThumb);
            g.fillEllipse(thumbX - thumbRadius, thumbY - thumbRadius,
                          thumbRadius * 2.0f, thumbRadius * 2.0f);
        }

        // Value text at center
        {
            juce::String valueStr;
            const double val = slider.getValue();

            if (std::abs(val) >= 100.0)
                valueStr = juce::String(static_cast<int>(val));
            else if (std::abs(val) >= 10.0)
                valueStr = juce::String(val, 1);
            else
                valueStr = juce::String(val, 2);

            valueStr += valueSuffix;

            g.setColour(Colors::textBright);
            g.setFont(juce::FontOptions(11.0f));
            g.drawText(valueStr,
                       static_cast<int>(centreX - radius), static_cast<int>(centreY - 7),
                       static_cast<int>(radius * 2.0f), 14,
                       juce::Justification::centred);
        }

        // Label below knob
        {
            g.setColour(Colors::textDim);
            g.setFont(juce::FontOptions(10.0f));
            g.drawText(label,
                       static_cast<int>(bounds.getX()),
                       static_cast<int>(centreY + radius + 6.0f),
                       static_cast<int>(bounds.getWidth()), 14,
                       juce::Justification::centred);
        }
    }

    void resized() override
    {
        // Slider covers the whole area for mouse interaction
        slider.setBounds(getLocalBounds());
    }

    juce::Slider& getSlider() { return slider; }

private:
    class SliderListener : private juce::Slider::Listener
    {
        friend class ArcKnob;
    };

    // Slider::Listener callback to trigger repaint
    struct Listener : juce::Slider::Listener
    {
        ArcKnob& owner;
        explicit Listener(ArcKnob& o) : owner(o) {}
        void sliderValueChanged(juce::Slider*) override { owner.repaint(); }
    };

    // We use the component itself as a Slider::Listener
    void sliderValueChanged(juce::Slider*) { repaint(); }

    juce::Slider slider;
    juce::String label;
    juce::String valueSuffix;

    // Make ArcKnob a Slider::Listener
    struct : juce::Slider::Listener
    {
        std::function<void()> callback;
        void sliderValueChanged(juce::Slider*) override { if (callback) callback(); }
    } internalListener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcKnob)
};

} // namespace gui
