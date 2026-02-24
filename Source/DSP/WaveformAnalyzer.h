/*
  ==============================================================================
    WaveformAnalyzer.h - Load and analyze external waveforms via FFT
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "HarmonicSeries.h"
#include <array>
#include <cmath>

namespace dsp
{

/**
 * Loads an audio file, performs FFT analysis, and extracts
 * a normalized spectral envelope to be used as a multiplicative
 * filter on the harmonic series.
 */
class WaveformAnalyzer
{
public:
    WaveformAnalyzer()
    {
        formatManager.registerBasicFormats();
        spectralEnvelope.fill(1.0f);
    }

    /**
     * Load and analyze an audio file.
     * Extracts spectral envelope from the first ~4096 samples.
     *
     * @param file  The audio file to load
     * @return true if successfully loaded and analyzed
     */
    bool loadFile(const juce::File& file)
    {
        auto reader = std::unique_ptr<juce::AudioFormatReader>(
            formatManager.createReaderFor(file));

        if (reader == nullptr)
            return false;

        // Read up to kFFTSize samples
        const int samplesToRead = std::min(static_cast<int>(reader->lengthInSamples), kFFTSize);
        juce::AudioBuffer<float> buffer(1, kFFTSize);
        buffer.clear();

        reader->read(&buffer, 0, samplesToRead, 0, true, false);

        // Perform FFT
        analyze(buffer.getReadPointer(0), samplesToRead);

        loadedFile = file;
        fileLoaded = true;
        return true;
    }

    /** Get the extracted spectral envelope (256 bins). */
    const std::array<float, kMaxHarmonics>& getSpectralEnvelope() const
    {
        return spectralEnvelope;
    }

    bool isFileLoaded() const { return fileLoaded; }
    juce::String getLoadedFileName() const
    {
        return fileLoaded ? loadedFile.getFileName() : juce::String("No file loaded");
    }

    /** Get raw FFT magnitudes for visualization (kFFTSize/2 bins). */
    const std::array<float, kFFTSize / 2>& getFFTMagnitudes() const { return fftMagnitudes; }

private:
    static constexpr int kFFTOrder = 12; // 2^12 = 4096
    static constexpr int kFFTSize = 1 << kFFTOrder;

    juce::AudioFormatManager formatManager;
    juce::dsp::FFT fft{ kFFTOrder };

    std::array<float, kMaxHarmonics> spectralEnvelope;
    std::array<float, kFFTSize / 2> fftMagnitudes{};

    juce::File loadedFile;
    bool fileLoaded = false;

    void analyze(const float* data, int numSamples)
    {
        // Prepare FFT input (zero-padded, windowed)
        std::array<float, kFFTSize * 2> fftData{};

        // Apply Hann window
        for (int i = 0; i < numSamples && i < kFFTSize; ++i)
        {
            const float window = 0.5f * (1.0f - std::cos(
                juce::MathConstants<float>::twoPi * static_cast<float>(i)
                / static_cast<float>(numSamples - 1)));
            fftData[i] = data[i] * window;
        }

        // Perform forward FFT
        fft.performRealOnlyForwardTransform(fftData.data());

        // Extract magnitudes
        float maxMagnitude = 0.0f;
        const int halfSize = kFFTSize / 2;

        for (int i = 0; i < halfSize; ++i)
        {
            const float real = fftData[i * 2];
            const float imag = fftData[i * 2 + 1];
            fftMagnitudes[i] = std::sqrt(real * real + imag * imag);
            maxMagnitude = std::max(maxMagnitude, fftMagnitudes[i]);
        }

        // Normalize and map to harmonic bins
        if (maxMagnitude > 0.0f)
        {
            for (int i = 0; i < halfSize; ++i)
                fftMagnitudes[i] /= maxMagnitude;
        }

        // Map FFT bins to harmonic envelope (256 harmonics)
        // Use simple bin-averaging to map kFFTSize/2 bins → 256 harmonics
        const float binsPerHarmonic = static_cast<float>(halfSize) / static_cast<float>(kMaxHarmonics);

        for (int h = 0; h < kMaxHarmonics; ++h)
        {
            const int startBin = static_cast<int>(static_cast<float>(h) * binsPerHarmonic);
            const int endBin = std::min(
                static_cast<int>(static_cast<float>(h + 1) * binsPerHarmonic),
                halfSize);

            float sum = 0.0f;
            int count = 0;
            for (int b = startBin; b < endBin; ++b)
            {
                sum += fftMagnitudes[b];
                ++count;
            }

            spectralEnvelope[h] = (count > 0) ? (sum / static_cast<float>(count)) : 0.0f;
        }

        // Normalize spectral envelope
        float maxEnv = 0.0f;
        for (float v : spectralEnvelope)
            maxEnv = std::max(maxEnv, v);

        if (maxEnv > 0.0f)
        {
            for (float& v : spectralEnvelope)
                v /= maxEnv;
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformAnalyzer)
};

} // namespace dsp
