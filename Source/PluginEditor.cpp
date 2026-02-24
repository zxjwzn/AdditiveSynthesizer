/*
  ==============================================================================
    PluginEditor.cpp - Main editor implementation
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AdditiveSynthesizerAudioProcessorEditor::AdditiveSynthesizerAudioProcessorEditor(
    AdditiveSynthesizerAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      oscillatorSection(p.getAPVTS()),
      spectralFilterSection(p.getAPVTS(), p.getWaveformAnalyzer()),
      envelopeSection(p.getAPVTS()),
      unisonOutputSection(p.getAPVTS()),
      midiKeyboard(p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&customLookAndFeel);
    setSize(900, 680);

    addAndMakeVisible(oscillatorSection);
    addAndMakeVisible(spectralFilterSection);
    addAndMakeVisible(envelopeSection);
    addAndMakeVisible(unisonOutputSection);
    addAndMakeVisible(midiKeyboard);

    // Style the keyboard to match the dark theme
    midiKeyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId,
                           juce::Colour(0xFF2A2A40));
    midiKeyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId,
                           juce::Colour(0xFF0E0E1A));
    midiKeyboard.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId,
                           juce::Colour(0xFF3A3A50));
    midiKeyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId,
                           gui::Colors::accent.withAlpha(0.6f));
    midiKeyboard.setAvailableRange(21, 108); // A0 to C8 — full piano range

    // Set up visualization
    oscillatorSection.setVisualizationBuffer(&p.getVisualizationBuffer());

    startTimerHz(20);
}

AdditiveSynthesizerAudioProcessorEditor::~AdditiveSynthesizerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void AdditiveSynthesizerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(gui::Colors::background);

    // Header bar
    auto headerBounds = getLocalBounds().removeFromTop(30).toFloat();
    g.setColour(gui::Colors::panelBackground);
    g.fillRect(headerBounds);

    g.setColour(gui::Colors::accent);
    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    g.drawText("ADDITIVE SYNTH", headerBounds.reduced(12.0f, 0.0f),
               juce::Justification::centredLeft);

    g.setColour(gui::Colors::textDim);
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("Poly: 8  |  v0.1", headerBounds.reduced(12.0f, 0.0f),
               juce::Justification::centredRight);

    // Subtle separator line
    g.setColour(gui::Colors::panelBorder.withAlpha(0.4f));
    g.drawHorizontalLine(30, 0.0f, static_cast<float>(getWidth()));
}

void AdditiveSynthesizerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(32); // Header

    // MIDI Keyboard at the bottom — scale key width to fill entire width
    auto keyboardBounds = bounds.removeFromBottom(50);
    // 88 keys (A0-C8): 52 white keys. Fit them into the available width.
    const float whiteKeyWidth = static_cast<float>(keyboardBounds.getWidth()) / 52.0f;
    midiKeyboard.setKeyWidth(whiteKeyWidth);
    midiKeyboard.setBounds(keyboardBounds);

    bounds = bounds.reduced(6);

    // Top half: Oscillator (left) | Spectral Filter (right)
    auto topHalf = bounds.removeFromTop(bounds.getHeight() * 55 / 100);
    const int topSplit = topHalf.getWidth() * 38 / 100; // 38% for oscillator
    oscillatorSection.setBounds(topHalf.removeFromLeft(topSplit).reduced(2));
    spectralFilterSection.setBounds(topHalf.reduced(2));

    bounds.removeFromTop(2);

    // Bottom half: Envelope (left) | Unison & Output (right)
    auto bottomHalf = bounds;
    const int bottomSplit = bottomHalf.getWidth() * 60 / 100; // 60% for envelope
    envelopeSection.setBounds(bottomHalf.removeFromLeft(bottomSplit).reduced(2));
    unisonOutputSection.setBounds(bottomHalf.reduced(2));
}

void AdditiveSynthesizerAudioProcessorEditor::timerCallback()
{
    // Update spectrum display with current harmonic data
    const auto* harmonicData = audioProcessor.getSynthEngine().getActiveHarmonicData();

    if (harmonicData != nullptr)
    {
        // Active voice — show live data
        spectralFilterSection.getSpectrumDisplay().setHarmonicData(harmonicData);
    }
    else
    {
        // No active voice — show a preview based on current parameters
        previewHarmonics = audioProcessor.getSynthEngine().computePreviewHarmonics();
        spectralFilterSection.getSpectrumDisplay().setHarmonicData(&previewHarmonics);
    }

    // Update filter visualization params
    auto& apvts = audioProcessor.getAPVTS();
    spectralFilterSection.getSpectrumDisplay().setFilterParams(
        apvts.getRawParameterValue("filterCutoff")->load(),
        apvts.getRawParameterValue("filterBoost")->load(),
        apvts.getRawParameterValue("filterStretch")->load());
}
