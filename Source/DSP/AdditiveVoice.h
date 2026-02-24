/*
  ==============================================================================
    AdditiveVoice.h - Single polyphonic voice for additive synthesis
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SineLUT.h"
#include "HarmonicSeries.h"
#include "SpectralFilter.h"

namespace dsp
{

/**
 * A single sound for the Synthesiser — just accepts all MIDI notes.
 */
class AdditiveSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel(int /*midiChannel*/) override { return true; }
};

/**
 * Parameters shared across all voices, updated from APVTS on the audio thread.
 */
struct AdditiveVoiceParams
{
    float oscRatio      = 0.5f;   // 0=square, 1=saw
    float sawPhase      = 0.0f;   // radians
    float sqrPhase      = 0.0f;   // radians
    float filterCutoff  = 128.0f; // harmonic number
    float filterBoost   = 0.0f;   // dB
    float filterPhase   = 0.0f;   // radians
    float filterStretch = 1.0f;   // stretch factor

    // Waveform filter (imported spectrum)
    bool  waveFilterEnabled = false;
    float waveFilterMix     = 0.0f;
    std::array<float, kMaxHarmonics> waveFilterSpectrum{};

    // ADSR
    float envAttack  = 0.01f;
    float envDecay   = 0.1f;
    float envSustain = 0.8f;
    float envRelease = 0.3f;
};

/**
 * Single voice for additive synthesis.
 * Maintains 256 phase accumulators and renders via SineLUT.
 */
class AdditiveVoice : public juce::SynthesiserVoice
{
public:
    AdditiveVoice(const AdditiveVoiceParams& sharedParams)
        : params(sharedParams)
    {
    }

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<AdditiveSound*>(sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        noteVelocity = velocity;
        noteFrequency = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));

        // Reset phase accumulators
        phaseAccumulators.fill(0.0f);

        // Update ADSR parameters and start envelope
        updateADSR();
        adsr.noteOn();

        // Compute initial harmonics
        rebuildHarmonics();
    }

    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            adsr.noteOff();
        }
        else
        {
            adsr.reset();
            clearCurrentNote();
        }
    }

    void pitchWheelMoved(int /*newPitchWheelValue*/) override {}
    void controllerMoved(int /*controllerNumber*/, int /*newControllerValue*/) override {}

    void prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
    {
        adsr.setSampleRate(sampleRate);
        currentSampleRate = sampleRate;
    }

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample, int numSamples) override
    {
        if (!isVoiceActive())
            return;

        // Rebuild harmonics if parameters changed (cheap check)
        rebuildHarmonics();
        updateADSR();

        const auto& sineLUT = SineLUT::getInstance();
        const int activeHarmonics = harmonicData.activeCount;

        for (int sample = startSample; sample < startSample + numSamples; ++sample)
        {
            float output = 0.0f;

            // Sum all active harmonics
            for (int n = 0; n < activeHarmonics; ++n)
            {
                if (harmonicData.amplitudes[n] <= 0.0f)
                    continue;

                output += harmonicData.amplitudes[n]
                          * sineLUT.lookup(phaseAccumulators[n] + harmonicData.phases[n]);

                // Advance phase accumulator
                const float stretchedN = std::pow(static_cast<float>(n + 1), params.filterStretch);
                const float freq = noteFrequency * stretchedN;
                phaseAccumulators[n] += SineLUT::kTwoPi * freq
                                        / static_cast<float>(currentSampleRate);

                // Wrap phase to [0, 2π)
                if (phaseAccumulators[n] >= SineLUT::kTwoPi)
                    phaseAccumulators[n] -= SineLUT::kTwoPi;
            }

            // Apply ADSR envelope and velocity
            const float envelopeValue = adsr.getNextSample();
            output *= envelopeValue * noteVelocity;

            // If envelope has ended, clear the voice
            if (!adsr.isActive())
            {
                clearCurrentNote();
                break;
            }

            // Normalize output (prevent excessive amplitude from many harmonics)
            output *= 0.25f;

            // Add to output buffer (all channels)
            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
                outputBuffer.addSample(ch, sample, output);
        }
    }

    /** Get the current monophonic output for visualization. */
    float getCurrentOutput() const noexcept { return lastOutput; }

    /** Get current harmonic data for spectrum visualization. */
    const HarmonicData& getHarmonicData() const noexcept { return harmonicData; }

private:
    const AdditiveVoiceParams& params;

    float noteFrequency = 440.0f;
    float noteVelocity = 0.0f;
    double currentSampleRate = 44100.0;
    float lastOutput = 0.0f;

    juce::ADSR adsr;
    HarmonicData harmonicData;
    std::array<float, kMaxHarmonics> phaseAccumulators{};

    void rebuildHarmonics()
    {
        harmonicData = HarmonicSeries::compute(
            params.oscRatio, params.sawPhase, params.sqrPhase,
            noteFrequency, currentSampleRate);

        SpectralFilter::apply(
            harmonicData, params.filterCutoff, params.filterBoost,
            params.filterPhase, params.filterStretch,
            noteFrequency, currentSampleRate);

        if (params.waveFilterEnabled && params.waveFilterMix > 0.0f)
        {
            SpectralFilter::applyWaveformFilter(
                harmonicData, params.waveFilterSpectrum, params.waveFilterMix);
        }
    }

    void updateADSR()
    {
        juce::ADSR::Parameters adsrParams;
        adsrParams.attack  = params.envAttack;
        adsrParams.decay   = params.envDecay;
        adsrParams.sustain = params.envSustain;
        adsrParams.release = params.envRelease;
        adsr.setParameters(adsrParams);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveVoice)
};

} // namespace dsp
