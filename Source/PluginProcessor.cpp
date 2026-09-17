#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace
{
constexpr auto kBypassId = "bypass";
}

MixAssistProcessor::MixAssistProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "K3CHMixAssist", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout MixAssistProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { kBypassId, 1 }, "Bypass", false));
    return { params.begin(), params.end() };
}

void MixAssistProcessor::prepareToPlay (double, int)
{
    peak[0].store (0.0f);
    peak[1].store (0.0f);
    rms[0].store (0.0f);
    rms[1].store (0.0f);
}

void MixAssistProcessor::releaseResources() {}

bool MixAssistProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return in == juce::AudioChannelSet::mono() || in == out;
}

void MixAssistProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    midi.clear();
    juce::ScopedNoDenormals noDenormals;

    const auto numIn  = getTotalNumInputChannels();
    const auto numOut = getTotalNumOutputChannels();
    const auto n      = buffer.getNumSamples();

    for (int ch = numIn; ch < numOut; ++ch)
        buffer.clear (ch, 0, n);

    // True pass-through: in-place copy. Mono insert → duplicate to stereo outs.
    if (numIn == 1 && numOut >= 2)
        buffer.copyFrom (1, 0, buffer, 0, 0, n);

    if (! isBypassed())
        measure (buffer);
    else
    {
        peak[0].store (0.0f);
        peak[1].store (0.0f);
        rms[0].store (0.0f);
        rms[1].store (0.0f);
    }
}

void MixAssistProcessor::measure (const juce::AudioBuffer<float>& buffer)
{
    const auto n = buffer.getNumSamples();
    if (n <= 0)
        return;

    const int channels = juce::jmin (2, buffer.getNumChannels());

    for (int ch = 0; ch < 2; ++ch)
    {
        float pk = 0.0f;
        float sumSq = 0.0f;
        const int src = juce::jmin (ch, channels - 1);
        const auto* data = buffer.getReadPointer (src);

        for (int i = 0; i < n; ++i)
        {
            const float s = std::abs (data[i]);
            pk = juce::jmax (pk, s);
            sumSq += data[i] * data[i];
        }

        const float blockRms = std::sqrt (sumSq / static_cast<float> (n));
        peak[(size_t) ch].store (pk);
        rms[(size_t) ch].store (blockRms);
    }
}

bool MixAssistProcessor::isBypassed() const
{
    if (auto* p = apvts.getRawParameterValue (kBypassId))
        return p->load() >= 0.5f;
    return false;
}

void MixAssistProcessor::getMeterLevels (float& peakL, float& peakR, float& rmsL, float& rmsR) const
{
    peakL = peak[0].load();
    peakR = peak[1].load();
    rmsL  = rms[0].load();
    rmsR  = rms[1].load();
}

void MixAssistProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("selectedSection", selectedSectionId, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void MixAssistProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
            selectedSectionId = apvts.state.getProperty ("selectedSection", selectedSectionId).toString();
        }
    }
}

juce::AudioProcessorEditor* MixAssistProcessor::createEditor()
{
    return new MixAssistEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MixAssistProcessor();
}
