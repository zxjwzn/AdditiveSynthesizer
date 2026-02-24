/*
  ==============================================================================
    UnisonOutputSection.h - Unison + stereo width + master gain controls
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SectionBase.h"

namespace gui
{

class UnisonOutputSection : public SectionBase
{
public:
    UnisonOutputSection(juce::AudioProcessorValueTreeState& apvts)
        : SectionBase("UNISON & OUTPUT", apvts, {
              { "Voices", "",   "unisonCount" },
              { "Detune", "ct", "unisonDetune" },
              { "Width",  "",   "stereoWidth" },
              { "Gain",   "dB", "masterGain" }
          }, 0) // knobHeight=0: knobs fill the entire content area
    {
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnisonOutputSection)
};

} // namespace gui
