/*
  ==============================================================================
    HarmonicSeries.h - Compute harmonic amplitudes & phases for saw/square mix
  ==============================================================================
*/

#pragma once

#include <array>
#include <cmath>

namespace dsp
{

static constexpr int kMaxHarmonics = 256;

/**
 * Computes the harmonic amplitude and phase arrays for a blend
 * of sawtooth and square wave, given oscillator parameters.
 *
 * This is recomputed when parameters change, NOT per-sample.
 */
struct HarmonicData
{
    std::array<float, kMaxHarmonics> amplitudes{};
    std::array<float, kMaxHarmonics> phases{};
    int activeCount = 0; // number of harmonics below Nyquist
};

class HarmonicSeries
{
public:
    /**
     * Recompute harmonic data.
     *
     * @param ratio       Saw/Square mix ratio (0=square, 1=saw)
     * @param sawPhase    Phase offset for sawtooth harmonics (radians)
     * @param sqrPhase    Phase offset for square harmonics (radians)
     * @param noteFreqHz  Fundamental frequency of the note
     * @param sampleRate  Current sample rate
     */
    static HarmonicData compute(float ratio, float sawPhase, float sqrPhase,
                                float noteFreqHz, double sampleRate)
    {
        HarmonicData data;
        const float nyquist = static_cast<float>(sampleRate) * 0.5f;
        int active = 0;

        for (int n = 1; n <= kMaxHarmonics; ++n)
        {
            const float freq = noteFreqHz * static_cast<float>(n);
            if (freq >= nyquist)
                break;

            // Sawtooth: all harmonics, amplitude = 1/n
            const float sawAmp = 1.0f / static_cast<float>(n);

            // Square: only odd harmonics, amplitude = 1/n
            const float sqrAmp = (n % 2 == 1) ? (1.0f / static_cast<float>(n)) : 0.0f;

            // Linear blend
            data.amplitudes[n - 1] = ratio * sawAmp + (1.0f - ratio) * sqrAmp;

            // Phase: weighted blend of phase offsets per harmonic type contribution
            // When ratio=1 (pure saw), use sawPhase; ratio=0 (pure square), use sqrPhase
            float phase = 0.0f;
            if (ratio > 0.0f && ratio < 1.0f && data.amplitudes[n - 1] > 0.0f)
            {
                const float sawContrib = ratio * sawAmp;
                const float sqrContrib = (1.0f - ratio) * sqrAmp;
                const float total = sawContrib + sqrContrib;
                if (total > 0.0f)
                    phase = (sawContrib * sawPhase + sqrContrib * sqrPhase) / total;
            }
            else if (ratio >= 1.0f)
            {
                phase = sawPhase;
            }
            else
            {
                phase = (n % 2 == 1) ? sqrPhase : 0.0f;
            }

            data.phases[n - 1] = phase * static_cast<float>(n); // Phase offset scales with harmonic number

            active = n;
        }

        data.activeCount = active;
        return data;
    }
};

} // namespace dsp
