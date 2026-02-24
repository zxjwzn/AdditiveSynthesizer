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

    // Unison (rendered per-voice, not post-processed)
    int   unisonCount   = 1;      // 1..8
    float unisonDetune  = 10.0f;  // cents
    float stereoWidth   = 0.5f;   // 0..1

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

        // Reset phase accumulators for all unison sub-voices
        for (auto& arr : uniPhaseAccumulators)
            arr.fill(0.0f);

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

        rebuildHarmonics();
        updateADSR();

        const auto& sineLUT = SineLUT::getInstance();
        const int activeHarmonics = harmonicData.activeCount;
        const int uniCount = juce::jlimit(1, kMaxUnisonVoices, params.unisonCount);
        const bool isStereo = outputBuffer.getNumChannels() >= 2;

        // Gain normalization: constant-power across unison voices
        const float gainPerUni = 1.0f / std::sqrt(static_cast<float>(uniCount));

        // Precompute per-unison frequency multiplier and stereo pan
        std::array<float, kMaxUnisonVoices> freqMul{};
        std::array<float, kMaxUnisonVoices> panL{}, panR{};

        for (int u = 0; u < uniCount; ++u)
        {
            float detuneOffsetCents = 0.0f;
            float panPos = 0.5f;

            if (uniCount > 1)
            {
                // Spread from -1 to +1
                const float spread = static_cast<float>(u) / static_cast<float>(uniCount - 1)
                                     * 2.0f - 1.0f;
                detuneOffsetCents = params.unisonDetune * spread;
                panPos = 0.5f + params.stereoWidth * spread * 0.5f;
                panPos = juce::jlimit(0.0f, 1.0f, panPos);
            }

            freqMul[u] = std::pow(2.0f, detuneOffsetCents / 1200.0f);
            panL[u] = std::cos(panPos * juce::MathConstants<float>::halfPi);
            panR[u] = std::sin(panPos * juce::MathConstants<float>::halfPi);
        }

        const float invSampleRate = 1.0f / static_cast<float>(currentSampleRate);

        for (int sample = startSample; sample < startSample + numSamples; ++sample)
        {
            float leftOut = 0.0f;
            float rightOut = 0.0f;

            for (int u = 0; u < uniCount; ++u)
            {
                float uniOutput = 0.0f;

                for (int n = 0; n < activeHarmonics; ++n)
                {
                    if (harmonicData.amplitudes[n] <= 0.0f)
                        continue;

                    uniOutput += harmonicData.amplitudes[n]
                                 * sineLUT.lookup(uniPhaseAccumulators[u][n]
                                                  + harmonicData.phases[n]);

                    // Advance phase: detuned frequency per unison voice
                    const float stretchedN = std::pow(static_cast<float>(n + 1),
                                                      params.filterStretch);
                    const float freq = noteFrequency * freqMul[u] * stretchedN;
                    uniPhaseAccumulators[u][n] += SineLUT::kTwoPi * freq * invSampleRate;

                    if (uniPhaseAccumulators[u][n] >= SineLUT::kTwoPi)
                        uniPhaseAccumulators[u][n] -= SineLUT::kTwoPi;
                }

                leftOut  += uniOutput * panL[u] * gainPerUni;
                rightOut += uniOutput * panR[u] * gainPerUni;
            }

            // Apply ADSR envelope and velocity
            const float envelopeValue = adsr.getNextSample();
            leftOut  *= envelopeValue * noteVelocity * 0.25f;
            rightOut *= envelopeValue * noteVelocity * 0.25f;

            if (!adsr.isActive())
            {
                clearCurrentNote();
                break;
            }

            outputBuffer.addSample(0, sample, leftOut);
            if (isStereo)
                outputBuffer.addSample(1, sample, rightOut);
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

    // Per-unison-voice phase accumulators: [unisonIdx][harmonicIdx]
    static constexpr int kMaxUnisonVoices = 8;
    std::array<std::array<float, kMaxHarmonics>, kMaxUnisonVoices> uniPhaseAccumulators{};

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
