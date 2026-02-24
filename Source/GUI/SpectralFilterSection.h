/*
  ==============================================================================
    SpectralFilterSection.h - Spectral filter controls + waveform import
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SectionBase.h"
#include "SpectrumDisplay.h"
#include <functional>

namespace gui
{

/**
 * Callback type for loading a waveform file.
 * Returns true if the file was loaded successfully.
 */
using FileLoadCallback = std::function<bool(const juce::File&)>;

class SpectralFilterSection : public SectionBase
{
public:
    SpectralFilterSection(juce::AudioProcessorValueTreeState& apvts,
                          FileLoadCallback loadCallback = nullptr)
        : SectionBase("SPECTRAL FILTER", apvts, {
              { "Cutoff",  "",                                                           "filterCutoff" },
              { "Boost",   "dB",                                                         "filterBoost" },
              { "Phase",   juce::String(juce::CharPointer_UTF8("\xc2\xb0")),             "filterPhase" },
              { "Stretch", "",                                                           "filterStretch" },
              { "Wet/Dry", "",                                                           "waveFilterMix" }
          }),
          onFileLoad(std::move(loadCallback))
    {
        addAndMakeVisible(spectrumDisplay);
        addAndMakeVisible(loadButton);
        addAndMakeVisible(fileLabel);

        loadButton.setButtonText("Load Waveform");
        loadButton.onClick = [this]() { loadWaveformFile(); };

        fileLabel.setText("No file loaded", juce::dontSendNotification);
        fileLabel.setColour(juce::Label::textColourId, Colors::textDim);
        fileLabel.setFont(juce::FontOptions(10.0f));
    }

    SpectrumDisplay& getSpectrumDisplay() { return spectrumDisplay; }

protected:
    void resizeContent(juce::Rectangle<int> content) override
    {
        auto specArea = content.removeFromTop(content.getHeight() - 28);
        spectrumDisplay.setBounds(specArea);

        content.removeFromTop(4);
        auto loadRow = content;
        loadButton.setBounds(loadRow.removeFromLeft(110).reduced(0, 2));
        loadRow.removeFromLeft(4);
        fileLabel.setBounds(loadRow.reduced(4, 2));
    }

private:
    SpectrumDisplay spectrumDisplay;

    juce::TextButton loadButton;
    juce::Label fileLabel;

    FileLoadCallback onFileLoad;

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
                if (file.existsAsFile() && onFileLoad)
                {
                    if (onFileLoad(file))
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
