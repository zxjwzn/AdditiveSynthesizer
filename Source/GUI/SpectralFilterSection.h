/*
  ==============================================================================
    SpectralFilterSection.h - Spectral filter controls + waveform import
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SectionPanel.h"
#include "ArcKnob.h"
#include "SpectrumDisplay.h"
#include "../DSP/WaveformAnalyzer.h"

namespace gui
{

class SpectralFilterSection : public juce::Component
{
public:
    SpectralFilterSection(juce::AudioProcessorValueTreeState& apvts,
                          dsp::WaveformAnalyzer& analyzer)
        : waveformAnalyzer(analyzer)
    {
        addAndMakeVisible(panel);
        addAndMakeVisible(cutoffKnob);
        addAndMakeVisible(boostKnob);
        addAndMakeVisible(phaseKnob);
        addAndMakeVisible(stretchKnob);
        addAndMakeVisible(spectrumDisplay);
        addAndMakeVisible(loadButton);
        addAndMakeVisible(wetDryKnob);
        addAndMakeVisible(fileLabel);

        // Attach to APVTS
        cutoffAttach  = std::make_unique<SliderAttachment>(apvts, "filterCutoff",  cutoffKnob.getSlider());
        boostAttach   = std::make_unique<SliderAttachment>(apvts, "filterBoost",   boostKnob.getSlider());
        phaseAttach   = std::make_unique<SliderAttachment>(apvts, "filterPhase",   phaseKnob.getSlider());
        stretchAttach = std::make_unique<SliderAttachment>(apvts, "filterStretch", stretchKnob.getSlider());
        wetDryAttach  = std::make_unique<SliderAttachment>(apvts, "waveFilterMix", wetDryKnob.getSlider());

        loadButton.setButtonText("Load Waveform");
        loadButton.onClick = [this]() { loadWaveformFile(); };

        fileLabel.setText("No file loaded", juce::dontSendNotification);
        fileLabel.setColour(juce::Label::textColourId, Colors::textDim);
        fileLabel.setFont(juce::FontOptions(10.0f));
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        panel.setBounds(bounds);

        auto content = panel.getContentArea();

        // Knobs row
        auto knobRow = content.removeFromTop(80);
        const int knobWidth = knobRow.getWidth() / 4;
        cutoffKnob.setBounds(knobRow.removeFromLeft(knobWidth));
        boostKnob.setBounds(knobRow.removeFromLeft(knobWidth));
        phaseKnob.setBounds(knobRow.removeFromLeft(knobWidth));
        stretchKnob.setBounds(knobRow);

        // Spectrum display
        content.removeFromTop(4);
        auto specArea = content.removeFromTop(content.getHeight() - 32);
        spectrumDisplay.setBounds(specArea);

        // Load waveform row
        content.removeFromTop(4);
        auto loadRow = content;
        loadButton.setBounds(loadRow.removeFromLeft(110).reduced(0, 2));
        loadRow.removeFromLeft(4);
        wetDryKnob.setBounds(loadRow.removeFromRight(60));
        fileLabel.setBounds(loadRow.reduced(4, 2));
    }

    SpectrumDisplay& getSpectrumDisplay() { return spectrumDisplay; }

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    SectionPanel panel{ "SPECTRAL FILTER" };

    ArcKnob cutoffKnob  { "Cutoff" };
    ArcKnob boostKnob   { "Boost", "dB" };
    ArcKnob phaseKnob   { "Phase", "\xC2\xB0" };
    ArcKnob stretchKnob { "Stretch" };
    ArcKnob wetDryKnob  { "Wet/Dry" };

    SpectrumDisplay spectrumDisplay;

    juce::TextButton loadButton;
    juce::Label fileLabel;

    dsp::WaveformAnalyzer& waveformAnalyzer;

    std::unique_ptr<SliderAttachment> cutoffAttach;
    std::unique_ptr<SliderAttachment> boostAttach;
    std::unique_ptr<SliderAttachment> phaseAttach;
    std::unique_ptr<SliderAttachment> stretchAttach;
    std::unique_ptr<SliderAttachment> wetDryAttach;

    void loadWaveformFile()
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select a waveform file",
            juce::File{},
            "*.wav;*.aiff;*.flac;*.mp3;*.ogg");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    if (waveformAnalyzer.loadFile(file))
                    {
                        fileLabel.setText(file.getFileName(), juce::dontSendNotification);
                        fileLabel.setColour(juce::Label::textColourId, Colors::waveformGreen);
                    }
                    else
                    {
                        fileLabel.setText("Failed to load", juce::dontSendNotification);
                        fileLabel.setColour(juce::Label::textColourId, Colors::accent);
                    }
                }
            });
    }

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectralFilterSection)
};

} // namespace gui
