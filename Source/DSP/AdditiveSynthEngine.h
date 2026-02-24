/*
  ==============================================================================
    AdditiveSynthEngine.h - Synthesiser engine wrapping voices
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AdditiveVoice.h"
#include "UnisonProcessor.h"

namespace synth
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
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        const int numSamples = buffer.getNumSamples();
        buffer.clear();

        // Render synth directly to stereo buffer
        // (unison detuning + stereo spread is handled inside each AdditiveVoice)
        synth.renderNextBlock(buffer, midiMessages, 0, numSamples);

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

    /**
     * Compute a preview HarmonicData based on current params at a reference
     * frequency (A4 = 440 Hz). Used for spectrum display when no note is active.
     */
    HarmonicData computePreviewHarmonics() const
    {
        constexpr float refFreq = 440.0f;
        auto data = HarmonicSeries::compute(
            voiceParams.oscRatio, voiceParams.sawPhase, voiceParams.sqrPhase,
            refFreq, currentSampleRate);

        SpectralFilter::apply(
            data, voiceParams.filterCutoff, voiceParams.filterBoost,
            voiceParams.filterPhase, voiceParams.filterStretch,
            refFreq, currentSampleRate);

        if (voiceParams.waveFilterEnabled && voiceParams.waveFilterMix > 0.0f)
        {
            SpectralFilter::applyWaveformFilter(
                data, voiceParams.waveFilterSpectrum, voiceParams.waveFilterMix);
        }

        return data;
    }

private:
    juce::Synthesiser synth;
    AdditiveVoiceParams voiceParams;
    UnisonProcessor unisonProcessor;
    float masterGainDb = 0.0f;

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthEngine)
};

} // namespace synth
