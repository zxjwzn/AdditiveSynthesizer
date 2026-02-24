/*
  ==============================================================================
    SineLUT.h - High-performance sine lookup table (header-only)
  ==============================================================================
*/

#pragma once

#include <cmath>
#include <array>

namespace dsp
{

/**
 * Static sine lookup table with linear interpolation.
 * 4096-point table for ~16-bit precision, much faster than std::sin().
 */
class SineLUT
{
public:
    static constexpr int kTableSize = 4096;
    static constexpr float kTwoPi = 6.283185307179586f;

    static SineLUT& getInstance()
    {
        static SineLUT instance;
        return instance;
    }

    /** Look up sin(phase) where phase is in radians [0, 2π). */
    [[nodiscard]] float lookup(float phase) const noexcept
    {
        // Normalize phase to [0, 1)
        float normalized = phase * kInvTwoPi;
        normalized -= std::floor(normalized); // wrap to [0, 1)

        const float index = normalized * static_cast<float>(kTableSize);
        const int idx0 = static_cast<int>(index);
        const int idx1 = (idx0 + 1) & kTableMask;
        const float frac = index - static_cast<float>(idx0);

        return table[idx0 & kTableMask] + frac * (table[idx1] - table[idx0 & kTableMask]);
    }

    /** Batch compute: output[i] = sin(phases[i]) */
    void lookupBatch(const float* phases, float* output, int count) const noexcept
    {
        for (int i = 0; i < count; ++i)
            output[i] = lookup(phases[i]);
    }

private:
    static constexpr int kTableMask = kTableSize - 1;
    static constexpr float kInvTwoPi = 1.0f / kTwoPi;

    std::array<float, kTableSize + 1> table{}; // +1 for interpolation guard

    SineLUT()
    {
        for (int i = 0; i <= kTableSize; ++i)
            table[i] = std::sin(static_cast<float>(i) / static_cast<float>(kTableSize) * kTwoPi);
    }

    SineLUT(const SineLUT&) = delete;
    SineLUT& operator=(const SineLUT&) = delete;
};

} // namespace dsp
