/*
  ==============================================================================
    UnisonProcessor.h - Unison stacking with detuning and stereo spread
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>

namespace dsp
{

/**
 * Processes a mono signal into stereo by stacking detuned copies
 * with pan spread for stereo width.
 *
 * Uses pitch-shifting via resampling (fractional delay) for detuning.
 * For simplicity, Unison here operates at the output stage by applying
 * micro-delays and subtle pitch offsets via allpass interpolation.
 *
 * Simpler approach: since additive synthesis gives us explicit control,
 * we duplicate the mono input with slight phase/pitch modulation.
 */
class UnisonProcessor
{
public:
    static constexpr int kMaxUnisonVoices = 8;

    UnisonProcessor() = default;

    void prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
    {
        currentSampleRate = sampleRate;

        // Initialize delay lines for each unison voice (for chorus-like effect)
        for (auto& dl : delayLines)
        {
            dl.resize(static_cast<size_t>(sampleRate * 0.05), 0.0f); // 50ms max delay
            dl.assign(dl.size(), 0.0f);
        }
        for (auto& idx : writeIndices)
            idx = 0;
        for (auto& phase : lfoPhases)
            phase = 0.0f;
    }

    /**
     * Process mono input buffer into stereo output buffer.
     * Applies unison stacking with detuning and stereo spread.
     */
    void process(const juce::AudioBuffer<float>& monoInput,
                 juce::AudioBuffer<float>& stereoOutput)
    {
        const int numSamples = monoInput.getNumSamples();
        const int numOutChannels = stereoOutput.getNumChannels();
        const float* monoData = monoInput.getReadPointer(0);

        stereoOutput.clear();

        if (voiceCount <= 1 || numOutChannels < 2)
        {
            // No unison: just copy mono to both channels
            for (int ch = 0; ch < numOutChannels; ++ch)
                stereoOutput.copyFrom(ch, 0, monoInput, 0, 0, numSamples);
            return;
        }

        const float gainPerVoice = 1.0f / std::sqrt(static_cast<float>(voiceCount));

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float input = monoData[sample];
            float leftSum = 0.0f;
            float rightSum = 0.0f;

            for (int v = 0; v < voiceCount; ++v)
            {
                // Apply chorus-like detuning via modulated delay
                float detuned = input;
                if (v > 0)
                {
                    // LFO-modulated micro delay for detuning effect
                    const float lfoRate = 0.5f + static_cast<float>(v) * 0.2f; // Hz
                    const float lfoDepth = detuneAmount * 0.0001f
                                           * static_cast<float>(currentSampleRate);
                    lfoPhases[v] += lfoRate / static_cast<float>(currentSampleRate);
                    if (lfoPhases[v] >= 1.0f) lfoPhases[v] -= 1.0f;

                    const float sign = (v % 2 == 0) ? 1.0f : -1.0f;
                    const float delayMs = 0.5f + sign * lfoDepth
                                          * std::sin(lfoPhases[v] * 6.283185f);
                    const float delaySamples = std::max(0.1f,
                        delayMs * static_cast<float>(currentSampleRate) / 1000.0f);

                    // Write to delay line
                    auto& dl = delayLines[v];
                    const int dlSize = static_cast<int>(dl.size());
                    dl[writeIndices[v] % dlSize] = input;
                    writeIndices[v] = (writeIndices[v] + 1) % dlSize;

                    // Read from delay line with linear interpolation
                    const float readPos = static_cast<float>(writeIndices[v]) - delaySamples;
                    int readIdx0 = static_cast<int>(std::floor(readPos));
                    while (readIdx0 < 0) readIdx0 += dlSize;
                    const int readIdx1 = (readIdx0 + 1) % dlSize;
                    const float frac = readPos - std::floor(readPos);

                    detuned = dl[readIdx0 % dlSize] * (1.0f - frac)
                            + dl[readIdx1] * frac;
                }

                // Stereo panning based on voice index and width
                // Voices spread from center to edges based on stereoWidth
                float pan = 0.5f; // center
                if (voiceCount > 1)
                {
                    const float spread = static_cast<float>(v) / static_cast<float>(voiceCount - 1);
                    pan = 0.5f + stereoWidth * (spread - 0.5f);
                }
                pan = juce::jlimit(0.0f, 1.0f, pan);

                // Constant-power panning
                const float leftGain = std::cos(pan * juce::MathConstants<float>::halfPi);
                const float rightGain = std::sin(pan * juce::MathConstants<float>::halfPi);

                leftSum += detuned * leftGain * gainPerVoice;
                rightSum += detuned * rightGain * gainPerVoice;
            }

            stereoOutput.setSample(0, sample, leftSum);
            if (numOutChannels >= 2)
                stereoOutput.setSample(1, sample, rightSum);
        }
    }

    void setVoiceCount(int count) { voiceCount = juce::jlimit(1, kMaxUnisonVoices, count); }
    void setDetuneAmount(float cents) { detuneAmount = cents; }
    void setStereoWidth(float width) { stereoWidth = juce::jlimit(0.0f, 1.0f, width); }

    int getVoiceCount() const { return voiceCount; }
    float getDetuneAmount() const { return detuneAmount; }
    float getStereoWidth() const { return stereoWidth; }

private:
    int voiceCount = 1;
    float detuneAmount = 10.0f;   // cents
    float stereoWidth = 0.5f;     // 0..1
    double currentSampleRate = 44100.0;

    // Delay lines for chorus-based detuning
    std::array<std::vector<float>, kMaxUnisonVoices> delayLines;
    std::array<int, kMaxUnisonVoices> writeIndices{};
    std::array<float, kMaxUnisonVoices> lfoPhases{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnisonProcessor)
};

} // namespace dsp
