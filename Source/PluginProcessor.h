/*
  ==============================================================================
    PluginProcessor.h - Main audio processor with APVTS and synth engine
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSP/AdditiveSynthEngine.h"
#include "DSP/WaveformAnalyzer.h"

class AdditiveSynthesizerAudioProcessor : public juce::AudioProcessor
{
public:
    AdditiveSynthesizerAudioProcessor();
    ~AdditiveSynthesizerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // === Public accessors ===
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    dsp::AdditiveSynthEngine& getSynthEngine() { return synthEngine; }
    const dsp::AdditiveSynthEngine& getSynthEngine() const { return synthEngine; }

    dsp::WaveformAnalyzer& getWaveformAnalyzer() { return waveformAnalyzer; }
    const dsp::WaveformAnalyzer& getWaveformAnalyzer() const { return waveformAnalyzer; }

    /** Thread-safe snapshot of the last rendered output for visualization. */
    const juce::AudioBuffer<float>& getVisualizationBuffer() const { return vizBuffer; }

private:
    juce::AudioProcessorValueTreeState apvts;
    dsp::AdditiveSynthEngine synthEngine;
    dsp::WaveformAnalyzer waveformAnalyzer;
    juce::AudioBuffer<float> vizBuffer;

    /** Create APVTS parameter layout. */
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    /** Pull APVTS parameter values and push them to the synth engine. */
    void updateSynthParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesizerAudioProcessor)
};
