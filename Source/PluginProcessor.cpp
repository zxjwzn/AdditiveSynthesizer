/*
  ==============================================================================
    PluginProcessor.cpp - Main audio processor implementation
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
static constexpr float kDegreesToRadians = juce::MathConstants<float>::twoPi / 360.0f;

//==============================================================================
AdditiveSynthesizerAudioProcessor::AdditiveSynthesizerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

AdditiveSynthesizerAudioProcessor::~AdditiveSynthesizerAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
AdditiveSynthesizerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // --- Oscillator ---
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "oscRatio", 1 }, "Saw/Square Ratio",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sawPhase", 1 }, "Saw Phase",
        juce::NormalisableRange<float>(0.0f, 360.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "sqrPhase", 1 }, "Square Phase",
        juce::NormalisableRange<float>(0.0f, 360.0f, 0.1f), 0.0f));

    // --- Spectral Filter ---
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "filterCutoff", 1 }, "Spectral Cutoff",
        juce::NormalisableRange<float>(1.0f, 256.0f, 0.1f, 0.5f), 128.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "filterBoost", 1 }, "Boost",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "filterPhase", 1 }, "Phase Rotation",
        juce::NormalisableRange<float>(0.0f, 360.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "filterStretch", 1 }, "Harmonic Stretch",
        juce::NormalisableRange<float>(0.5f, 2.0f, 0.01f), 1.0f));

    // --- Waveform Filter ---
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "waveFilterMix", 1 }, "Waveform Filter Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    // --- Unison ---
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{ "unisonCount", 1 }, "Unison Voices", 1, 8, 1));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "unisonDetune", 1 }, "Detune",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 10.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "stereoWidth", 1 }, "Stereo Width",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // --- ADSR ---
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "envAttack", 1 }, "Attack",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.3f), 0.01f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "envDecay", 1 }, "Decay",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.3f), 0.1f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "envSustain", 1 }, "Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "envRelease", 1 }, "Release",
        juce::NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f), 0.3f));

    // --- Master ---
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "masterGain", 1 }, "Master Gain",
        juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), 0.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
void AdditiveSynthesizerAudioProcessor::updateSynthParameters()
{
    auto& vp = synthEngine.getVoiceParams();

    vp.oscRatio      = apvts.getRawParameterValue("oscRatio")->load();
    vp.sawPhase      = apvts.getRawParameterValue("sawPhase")->load() * kDegreesToRadians;
    vp.sqrPhase      = apvts.getRawParameterValue("sqrPhase")->load() * kDegreesToRadians;

    vp.filterCutoff  = apvts.getRawParameterValue("filterCutoff")->load();
    vp.filterBoost   = apvts.getRawParameterValue("filterBoost")->load();
    vp.filterPhase   = apvts.getRawParameterValue("filterPhase")->load() * kDegreesToRadians;
    vp.filterStretch = apvts.getRawParameterValue("filterStretch")->load();

    vp.waveFilterMix     = apvts.getRawParameterValue("waveFilterMix")->load();
    vp.waveFilterEnabled = waveformAnalyzer.isFileLoaded();
    if (vp.waveFilterEnabled)
        vp.waveFilterSpectrum = waveformAnalyzer.getSpectralEnvelope();

    vp.envAttack  = apvts.getRawParameterValue("envAttack")->load();
    vp.envDecay   = apvts.getRawParameterValue("envDecay")->load();
    vp.envSustain = apvts.getRawParameterValue("envSustain")->load();
    vp.envRelease = apvts.getRawParameterValue("envRelease")->load();

    // Unison
    auto& unison = synthEngine.getUnisonProcessor();
    unison.setVoiceCount(static_cast<int>(apvts.getRawParameterValue("unisonCount")->load()));
    unison.setDetuneAmount(apvts.getRawParameterValue("unisonDetune")->load());
    unison.setStereoWidth(apvts.getRawParameterValue("stereoWidth")->load());

    // Master
    synthEngine.setMasterGain(apvts.getRawParameterValue("masterGain")->load());
}

//==============================================================================
const juce::String AdditiveSynthesizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AdditiveSynthesizerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AdditiveSynthesizerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AdditiveSynthesizerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AdditiveSynthesizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AdditiveSynthesizerAudioProcessor::getNumPrograms()
{
    return 1;
}

int AdditiveSynthesizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AdditiveSynthesizerAudioProcessor::setCurrentProgram(int /*index*/)
{
}

const juce::String AdditiveSynthesizerAudioProcessor::getProgramName(int /*index*/)
{
    return {};
}

void AdditiveSynthesizerAudioProcessor::changeProgramName(int /*index*/,
                                                           const juce::String& /*newName*/)
{
}

//==============================================================================
void AdditiveSynthesizerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synthEngine.prepareToPlay(sampleRate, samplesPerBlock);
    vizBuffer.setSize(2, samplesPerBlock);
    vizBuffer.clear();
}

void AdditiveSynthesizerAudioProcessor::releaseResources()
{
    synthEngine.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AdditiveSynthesizerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void AdditiveSynthesizerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                      juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear buffer (synth output only, no input passthrough)
    buffer.clear();

    // Merge on-screen keyboard MIDI events into the incoming buffer
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    // Update parameters from APVTS
    updateSynthParameters();

    // Render synth
    synthEngine.processBlock(buffer, midiMessages);

    // Copy output for visualization
    const int numSamples = buffer.getNumSamples();
    vizBuffer.setSize(buffer.getNumChannels(), numSamples, false, false, true);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        vizBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
}

//==============================================================================
bool AdditiveSynthesizerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AdditiveSynthesizerAudioProcessor::createEditor()
{
    return new AdditiveSynthesizerAudioProcessorEditor(*this);
}

//==============================================================================
void AdditiveSynthesizerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AdditiveSynthesizerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AdditiveSynthesizerAudioProcessor();
}
