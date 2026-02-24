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
      unisonOutputSection(p.getAPVTS())
{
    setLookAndFeel(&customLookAndFeel);
    setSize(900, 620);

    addAndMakeVisible(oscillatorSection);
    addAndMakeVisible(spectralFilterSection);
    addAndMakeVisible(envelopeSection);
    addAndMakeVisible(unisonOutputSection);

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
    spectralFilterSection.getSpectrumDisplay().setHarmonicData(harmonicData);

    // Update filter visualization params
    auto& apvts = audioProcessor.getAPVTS();
    spectralFilterSection.getSpectrumDisplay().setFilterParams(
        apvts.getRawParameterValue("filterCutoff")->load(),
        apvts.getRawParameterValue("filterBoost")->load(),
        apvts.getRawParameterValue("filterStretch")->load());
}
