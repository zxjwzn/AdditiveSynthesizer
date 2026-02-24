/*
  ==============================================================================
    SpectralFilter.h - Apply spectral domain filtering to harmonic data
  ==============================================================================
*/

#pragma once

#include "HarmonicSeries.h"
#include <cmath>
#include <algorithm>

namespace synth
{

/**
 * Applies spectral-domain filtering to a HarmonicData:
 *   - Cutoff: sigmoid low-pass in harmonic domain
 *   - Boost:  resonant peak near cutoff
 *   - Phase:  per-harmonic phase rotation
 *   - Stretch: harmonic frequency remapping f_n = n^stretch * f0
 */
class SpectralFilter
{
public:
    /**
     * Apply spectral filter in-place on HarmonicData.
     *
     * @param data        HarmonicData to modify
     * @param cutoff      Cutoff harmonic number (1..256)
     * @param boostDb     Boost amount at cutoff in dB (0..24)
     * @param phaseRot    Phase rotation amount in radians
     * @param stretch     Harmonic stretch factor (0.5..2.0, 1.0 = normal)
     * @param noteFreqHz  Fundamental frequency
     * @param sampleRate  Current sample rate
     */
    static void apply(HarmonicData& data, float cutoff, float boostDb,
                      float phaseRot, float stretch,
                      float noteFreqHz, double sampleRate)
    {
        const float nyquist = static_cast<float>(sampleRate) * 0.5f;
        const float boostLinear = std::pow(10.0f, boostDb / 20.0f);
        constexpr float smoothness = 2.0f; // sigmoid smoothness factor
        int newActive = 0;

        for (int n = 1; n <= data.activeCount; ++n)
        {
            const int idx = n - 1;

            // --- Stretch: remap harmonic frequency ---
            const float stretchedN = std::pow(static_cast<float>(n), stretch);
            const float stretchedFreq = noteFreqHz * stretchedN;

            // Nyquist cutoff for stretched harmonics
            if (stretchedFreq >= nyquist)
            {
                data.amplitudes[idx] = 0.0f;
                continue;
            }

            // --- Cutoff: sigmoid low-pass ---
            const float x = (static_cast<float>(n) - cutoff) / smoothness;
            const float sigmoidGain = 1.0f / (1.0f + std::exp(x));

            // --- Boost: bell curve at cutoff ---
            const float dist = static_cast<float>(n) - cutoff;
            constexpr float bellWidth = 3.0f;
            const float bellGain = 1.0f + (boostLinear - 1.0f)
                                   * std::exp(-0.5f * (dist * dist) / (bellWidth * bellWidth));

            // Apply amplitude modifications
            data.amplitudes[idx] *= sigmoidGain * bellGain;

            // --- Phase rotation ---
            data.phases[idx] += phaseRot * static_cast<float>(n);

            newActive = n;
        }

        data.activeCount = newActive;
    }

    /**
     * Apply imported waveform spectral envelope as a multiplicative filter.
     *
     * @param data             HarmonicData to modify
     * @param spectralEnvelope Normalized spectral envelope (256 floats, 0..1)
     * @param mix              Dry/Wet mix (0 = dry/bypass, 1 = full wet)
     */
    static void applyWaveformFilter(HarmonicData& data,
                                     const std::array<float, kMaxHarmonics>& spectralEnvelope,
                                     float mix)
    {
        if (mix <= 0.0f)
            return;

        for (int n = 0; n < data.activeCount; ++n)
        {
            const float filtered = data.amplitudes[n] * spectralEnvelope[n];
            data.amplitudes[n] = data.amplitudes[n] * (1.0f - mix) + filtered * mix;
        }
    }
};

} // namespace synth
