/*
  ==============================================================================
    AdditiveSynthEngine.h - Synthesiser engine wrapping voices
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AdditiveVoice.h"
#include "UnisonProcessor.h"

namespace dsp
{

static constexpr int kMaxPolyphony = 8;

/**
 * Main synthesis engine. Owns:
 *   - juce::Synthesiser with 8 AdditiveVoice instances
 *   - UnisonProcessor for stereo widening
 *   - Shared voice parameters
 */
class AdditiveSynthEngine
{
public:
    AdditiveSynthEngine()
    {
        synth.addSound(new AdditiveSound());

        for (int i = 0; i < kMaxPolyphony; ++i)
            synth.addVoice(new AdditiveVoice(voiceParams));
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        synth.setCurrentPlaybackSampleRate(sampleRate);
        currentSampleRate = sampleRate;
        currentBlockSize = samplesPerBlock;

        // Prepare each voice
        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<AdditiveVoice*>(synth.getVoice(i)))
                voice->prepareToPlay(sampleRate, samplesPerBlock);
        }

        unisonProcessor.prepareToPlay(sampleRate, samplesPerBlock);

        // Mono buffer for synth output (before unison processing)
        monoBuffer.setSize(1, samplesPerBlock);
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        const int numSamples = buffer.getNumSamples();

        // Render synth to a temporary mono buffer first
        monoBuffer.setSize(1, numSamples, false, false, true);
        monoBuffer.clear();

        synth.renderNextBlock(monoBuffer, midiMessages, 0, numSamples);

        // Apply Unison processing (mono → stereo)
        unisonProcessor.process(monoBuffer, buffer);

        // Apply master gain
        const float gainLinear = juce::Decibels::decibelsToGain(masterGainDb);
        buffer.applyGain(gainLinear);
    }

    void releaseResources()
    {
        // Nothing specific to release
    }

    /** Access voice parameters for updating from APVTS. */
    AdditiveVoiceParams& getVoiceParams() { return voiceParams; }
    const AdditiveVoiceParams& getVoiceParams() const { return voiceParams; }

    /** Access unison processor for updating parameters. */
    UnisonProcessor& getUnisonProcessor() { return unisonProcessor; }

    /** Set master gain in dB. */
    void setMasterGain(float gainDb) { masterGainDb = gainDb; }

    /** Get the first active voice's harmonic data for visualization. */
    const HarmonicData* getActiveHarmonicData() const
    {
        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            auto* voice = dynamic_cast<const AdditiveVoice*>(synth.getVoice(i));
            if (voice != nullptr && voice->isVoiceActive())
                return &voice->getHarmonicData();
        }
        return nullptr;
    }

private:
    juce::Synthesiser synth;
    AdditiveVoiceParams voiceParams;
    UnisonProcessor unisonProcessor;
    float masterGainDb = 0.0f;

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    juce::AudioBuffer<float> monoBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthEngine)
};

} // namespace dsp
