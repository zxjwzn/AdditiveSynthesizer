/*
  ==============================================================================
    CustomLookAndFeel.h - Dark modern theme for the synth UI
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace gui
{

/** Color palette for the dark theme. */
namespace Colors
{
    static const juce::Colour background       { 0xff1A1A2E };
    static const juce::Colour panelBackground  { 0xff16213E };
    static const juce::Colour panelBorder      { 0xff0F3460 };
    static const juce::Colour accent           { 0xffE94560 };
    static const juce::Colour accentDark       { 0xffA83279 };
    static const juce::Colour textBright       { 0xffEEEEEE };
    static const juce::Colour textDim          { 0xff8899AA };
    static const juce::Colour knobBackground   { 0xff0A0E1A };
    static const juce::Colour knobArcTrack     { 0xff2A2E4A };
    static const juce::Colour knobArcActive    { 0xffE94560 };
    static const juce::Colour knobThumb        { 0xffFFFFFF };
    static const juce::Colour waveformGreen    { 0xff00E676 };
    static const juce::Colour spectrumCyan     { 0xff00BCD4 };
    static const juce::Colour envelopeYellow   { 0xffFFD54F };
}

/**
 * Custom LookAndFeel based on V4 with dark theme styling.
 */
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        // Set the dark colour scheme
        setColour(juce::ResizableWindow::backgroundColourId, Colors::background);
        setColour(juce::Label::textColourId, Colors::textBright);
        setColour(juce::Slider::textBoxTextColourId, Colors::textBright);
        setColour(juce::Slider::textBoxBackgroundColourId, Colors::knobBackground);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::TextButton::buttonColourId, Colors::panelBorder);
        setColour(juce::TextButton::textColourOffId, Colors::textBright);
        setColour(juce::TextButton::buttonOnColourId, Colors::accent);
        setColour(juce::TextButton::textColourOnId, Colors::textBright);
        setColour(juce::ComboBox::backgroundColourId, Colors::panelBackground);
        setColour(juce::ComboBox::textColourId, Colors::textBright);
        setColour(juce::ComboBox::outlineColourId, Colors::panelBorder);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& /*slider*/) override
    {
        // This is called for standard rotary sliders (not our custom ArcKnob).
        // ArcKnob does its own painting, but we provide a fallback here.
        const float radius = static_cast<float>(juce::jmin(width, height)) * 0.4f;
        const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
        const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
        const float angle = rotaryStartAngle
                            + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Background arc
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(Colors::knobArcTrack);
        g.strokePath(bgArc, juce::PathStrokeType(3.0f));

        // Active arc
        juce::Path activeArc;
        activeArc.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                                rotaryStartAngle, angle, true);
        g.setColour(Colors::knobArcActive);
        g.strokePath(activeArc, juce::PathStrokeType(3.0f));

        // Thumb
        juce::Path thumb;
        const float thumbLen = radius * 0.3f;
        thumb.addRectangle(-1.5f, -radius, 3.0f, thumbLen);
        g.setColour(Colors::knobThumb);
        g.fillPath(thumb, juce::AffineTransform::rotation(angle)
                              .translated(centreX, centreY));
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(label.getFont());

        const auto textArea = label.getBorderSize().subtractedFrom(label.getLocalBounds());
        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         juce::jmax(1, static_cast<int>(static_cast<float>(textArea.getHeight())
                                                         / label.getFont().getHeight())),
                         label.getMinimumHorizontalScale());
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
        const auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        auto col = backgroundColour;

        if (shouldDrawButtonAsDown)
            col = col.brighter(0.2f);
        else if (shouldDrawButtonAsHighlighted)
            col = col.brighter(0.1f);

        g.setColour(col);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(Colors::panelBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    }
};

} // namespace gui
