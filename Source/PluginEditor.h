/*
  ==============================================================================
    PluginEditor.h - Main editor assembling all GUI sections
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/CustomLookAndFeel.h"
#include "GUI/OscillatorSection.h"
#include "GUI/SpectralFilterSection.h"
#include "GUI/EnvelopeSection.h"
#include "GUI/UnisonOutputSection.h"

class AdditiveSynthesizerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                 public juce::Timer
{
public:
    AdditiveSynthesizerAudioProcessorEditor(AdditiveSynthesizerAudioProcessor&);
    ~AdditiveSynthesizerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    AdditiveSynthesizerAudioProcessor& audioProcessor;

    gui::CustomLookAndFeel customLookAndFeel;

    gui::OscillatorSection      oscillatorSection;
    gui::SpectralFilterSection  spectralFilterSection;
    gui::EnvelopeSection        envelopeSection;
    gui::UnisonOutputSection    unisonOutputSection;

    juce::MidiKeyboardComponent midiKeyboard;

    // Preview harmonic data for spectrum display when no note is active
    synth::HarmonicData previewHarmonics;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesizerAudioProcessorEditor)
};
