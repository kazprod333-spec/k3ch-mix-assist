#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr const char* kBypass = "bypass";
constexpr const char* kDryWet = "dry_wet";
constexpr const char* kOutput = "output_db";
constexpr const char* kVocal = "vocal_preset";
constexpr const char* kHpf = "hpf_hz";
constexpr const char* kEqLowHz = "eq_low_hz";
constexpr const char* kEqLowG = "eq_low_gain";
constexpr const char* kEqLowQ = "eq_low_q";
constexpr const char* kEqMidHz = "eq_mid_hz";
constexpr const char* kEqMidG = "eq_mid_gain";
constexpr const char* kEqMidQ = "eq_mid_q";
constexpr const char* kEqAirHz = "eq_air_hz";
constexpr const char* kEqAirG = "eq_air_gain";
constexpr const char* kCompTh = "comp_threshold";
constexpr const char* kCompRatio = "comp_ratio";
constexpr const char* kCompAtk = "comp_attack";
constexpr const char* kCompRel = "comp_release";
constexpr const char* kCompMk = "comp_makeup";
constexpr const char* kDeessHz = "deess_hz";
constexpr const char* kDeessAmt = "deess_amount";
constexpr const char* kSatDrv = "sat_drive";
constexpr const char* kSatMix = "sat_mix";
constexpr const char* kSendA = "send_a_preset";
constexpr const char* kSendAdB = "send_a_db";
constexpr const char* kSendB = "send_b_preset";
constexpr const char* kSendBdB = "send_b_db";
constexpr const char* kReturn = "return_mix";

float dbToLin (float db) { return juce::Decibels::decibelsToGain (db); }

std::unique_ptr<juce::AudioParameterFloat> mkFloat (const char* id, const char* name,
                                                    float min, float max, float def,
                                                    float step = 0.01f,
                                                    const juce::String& suffix = {})
{
    juce::NormalisableRange<float> range (min, max, step);
    auto p = std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id, 1 }, name, range, def);
    juce::ignoreUnused (suffix);
    return p;
}

} // namespace

PresetsProcessor::PresetsProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      library (k3ch_presets::PresetLibrary::loadEmbedded()),
      apvts (*this, nullptr, "K3CHPresets", createLayout (library)),
      parser (library)
{
    for (const auto* id : { kVocal, kSendA, kSendB })
        apvts.addParameterListener (id, this);

    loadVocalIndex (0, false);
}

PresetsProcessor::~PresetsProcessor()
{
    apvts.removeParameterListener (kVocal, this);
    apvts.removeParameterListener (kSendA, this);
    apvts.removeParameterListener (kSendB, this);
}

juce::AudioProcessorValueTreeState::ParameterLayout PresetsProcessor::createLayout (const k3ch_presets::PresetLibrary& lib)
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto vocalNames = lib.vocalNames();
    auto fxNames = lib.fxNames();
    if (vocalNames.isEmpty())
        vocalNames.add ("ModernRap");
    if (fxNames.isEmpty())
        fxNames.add ("Off");

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { kBypass, 1 }, "Bypass", false));
    params.push_back (mkFloat (kDryWet, "Dry/Wet", 0.0f, 1.0f, 1.0f, 0.01f));
    params.push_back (mkFloat (kOutput, "Output", -24.0f, 12.0f, 0.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { kVocal, 1 }, "Vocal Preset", vocalNames, 0));

    params.push_back (mkFloat (kHpf, "HPF", 20.0f, 300.0f, 85.0f, 1.0f));
    params.push_back (mkFloat (kEqLowHz, "EQ Low Hz", 80.0f, 500.0f, 280.0f, 1.0f));
    params.push_back (mkFloat (kEqLowG, "EQ Low", -12.0f, 6.0f, -3.0f, 0.1f));
    params.push_back (mkFloat (kEqLowQ, "EQ Low Q", 0.3f, 4.0f, 1.2f, 0.01f));
    params.push_back (mkFloat (kEqMidHz, "EQ Mid Hz", 1000.0f, 8000.0f, 3500.0f, 1.0f));
    params.push_back (mkFloat (kEqMidG, "EQ Mid", -12.0f, 12.0f, 2.0f, 0.1f));
    params.push_back (mkFloat (kEqMidQ, "EQ Mid Q", 0.3f, 4.0f, 1.4f, 0.01f));
    params.push_back (mkFloat (kEqAirHz, "Air Hz", 6000.0f, 16000.0f, 11000.0f, 1.0f));
    params.push_back (mkFloat (kEqAirG, "Air", -6.0f, 12.0f, 1.8f, 0.1f));

    params.push_back (mkFloat (kCompTh, "Comp Thresh", -40.0f, 0.0f, -18.0f, 0.1f));
    params.push_back (mkFloat (kCompRatio, "Comp Ratio", 1.0f, 12.0f, 4.0f, 0.1f));
    params.push_back (mkFloat (kCompAtk, "Comp Attack", 0.1f, 50.0f, 3.0f, 0.1f));
    params.push_back (mkFloat (kCompRel, "Comp Release", 10.0f, 400.0f, 80.0f, 1.0f));
    params.push_back (mkFloat (kCompMk, "Comp Makeup", 0.0f, 18.0f, 4.0f, 0.1f));
    params.push_back (mkFloat (kDeessHz, "De-ess Hz", 3000.0f, 10000.0f, 6400.0f, 1.0f));
    params.push_back (mkFloat (kDeessAmt, "De-ess", 0.0f, 1.0f, 0.42f, 0.01f));
    params.push_back (mkFloat (kSatDrv, "Sat Drive", 0.0f, 1.0f, 0.22f, 0.01f));
    params.push_back (mkFloat (kSatMix, "Sat Mix", 0.0f, 1.0f, 0.28f, 0.01f));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { kSendA, 1 }, "Send A", fxNames, 0));
    params.push_back (mkFloat (kSendAdB, "Send A Level", -60.0f, 0.0f, -15.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { kSendB, 1 }, "Send B", fxNames, 0));
    params.push_back (mkFloat (kSendBdB, "Send B Level", -60.0f, 0.0f, -15.0f, 0.1f));
    params.push_back (mkFloat (kReturn, "Return Mix", 0.0f, 1.0f, 1.0f, 0.01f));

    return { params.begin(), params.end() };
}

void PresetsProcessor::setFloatParam (const juce::String& id, float value, bool)
{
    if (auto* p = dynamic_cast<juce::AudioParameterFloat*> (apvts.getParameter (id)))
        p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (value));
}

void PresetsProcessor::setChoiceParam (const juce::String& id, int index, bool)
{
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (id)))
        p->setValueNotifyingHost (p->convertTo0to1 ((float) juce::jlimit (0, p->choices.size() - 1, index)));
}

void PresetsProcessor::setBoolParam (const juce::String& id, bool value, bool)
{
    if (auto* p = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter (id)))
        p->setValueNotifyingHost (value ? 1.0f : 0.0f);
}

void PresetsProcessor::loadVocalIndex (int index, bool notifyHost)
{
    const auto* p = library.vocalAt (index);
    if (p == nullptr)
        return;

    const juce::ScopedValueSetter<bool> sv (applyingPreset, true);
    setChoiceParam (kVocal, index, notifyHost);
    const auto& s = p->settings;
    setFloatParam (kHpf, s.hpfHz, notifyHost);
    setFloatParam (kEqLowHz, s.eqLowHz, notifyHost);
    setFloatParam (kEqLowG, s.eqLowGainDb, notifyHost);
    setFloatParam (kEqLowQ, s.eqLowQ, notifyHost);
    setFloatParam (kEqMidHz, s.eqMidHz, notifyHost);
    setFloatParam (kEqMidG, s.eqMidGainDb, notifyHost);
    setFloatParam (kEqMidQ, s.eqMidQ, notifyHost);
    setFloatParam (kEqAirHz, s.eqAirHz, notifyHost);
    setFloatParam (kEqAirG, s.eqAirGainDb, notifyHost);
    setFloatParam (kCompTh, s.compThresholdDb, notifyHost);
    setFloatParam (kCompRatio, s.compRatio, notifyHost);
    setFloatParam (kCompAtk, s.compAttackMs, notifyHost);
    setFloatParam (kCompRel, s.compReleaseMs, notifyHost);
    setFloatParam (kCompMk, s.compMakeupDb, notifyHost);
    setFloatParam (kDeessHz, s.deessHz, notifyHost);
    setFloatParam (kDeessAmt, s.deessAmount, notifyHost);
    setFloatParam (kSatDrv, s.satDrive, notifyHost);
    setFloatParam (kSatMix, s.satMix, notifyHost);

    auto applyDefinedSend = [this, notifyHost] (int slot, const juce::String& fxId)
    {
        if (fxId.isEmpty())
            return juce::String();
        const int fx = library.findFxIndex (fxId);
        const auto* fxP = library.fxAt (fx);
        if (fxP == nullptr)
            return juce::String();
        setChoiceParam (slot == 0 ? kSendA : kSendB, fx, notifyHost);
        if (fxP->settings.active)
            setFloatParam (slot == 0 ? kSendAdB : kSendBdB, fxP->defaultSendDb, notifyHost);
        return fxP->name;
    };
    const auto sendAName = applyDefinedSend (0, p->sendAId);
    const auto sendBName = applyDefinedSend (1, p->sendBId);

    lastStatus = juce::String::fromUTF8 ("Cha\xc3\xaene appliqu\xc3\xa9e : ") + p->name
                 + juce::String::fromUTF8 (" \xe2\x80\x94 pose ce plug-in sur l\xe2\x80\x99insert FL choisi");
    if (sendAName.isNotEmpty() || sendBName.isNotEmpty())
    {
        lastStatus += juce::String::fromUTF8 (" \xc2\xb7 Send A ");
        lastStatus += sendAName.isNotEmpty() ? sendAName : "Off";
        lastStatus += juce::String::fromUTF8 (" / Send B ");
        lastStatus += sendBName.isNotEmpty() ? sendBName : "Off";
    }
    pushEngineFromParams();
}

void PresetsProcessor::loadFxIndex (int sendSlot, int index, bool applyDefaultLevel, bool notifyHost)
{
    const auto* p = library.fxAt (index);
    if (p == nullptr || (sendSlot != 0 && sendSlot != 1))
        return;

    const juce::ScopedValueSetter<bool> sv (applyingPreset, true);
    setChoiceParam (sendSlot == 0 ? kSendA : kSendB, index, notifyHost);
    if (applyDefaultLevel && p->settings.active)
        setFloatParam (sendSlot == 0 ? kSendAdB : kSendBdB, p->defaultSendDb, notifyHost);

    lastStatus = juce::String ("Send ") + (sendSlot == 0 ? "A" : "B")
                 + juce::String::fromUTF8 (" \xe2\x86\x90 ") + p->name;
    pushEngineFromParams();
}

k3ch_presets::Command PresetsProcessor::applyCommand (const juce::String& text)
{
    auto cmd = parser.parse (text);
    if (! cmd.ok)
    {
        lastStatus = cmd.message;
        return cmd;
    }

    switch (cmd.type)
    {
        case k3ch_presets::Command::Type::LoadVocal:
            loadVocalIndex (cmd.vocalIndex, true);
            cmd.message = lastStatus;
            break;
        case k3ch_presets::Command::Type::LoadSend:
            loadFxIndex (cmd.sendIndex, cmd.fxIndex, true, true);
            cmd.message = lastStatus;
            break;
        case k3ch_presets::Command::Type::SetSendLevel:
            setFloatParam (cmd.sendIndex == 0 ? kSendAdB : kSendBdB, cmd.value, true);
            lastStatus = cmd.message;
            pushEngineFromParams();
            break;
        case k3ch_presets::Command::Type::SetDryWet:
            setFloatParam (kDryWet, cmd.value, true);
            lastStatus = cmd.message;
            break;
        case k3ch_presets::Command::Type::Bypass:
            setBoolParam (kBypass, cmd.bypassOn, true);
            lastStatus = cmd.message;
            break;
        case k3ch_presets::Command::Type::Help:
            lastStatus = cmd.message;
            break;
        case k3ch_presets::Command::Type::Unknown:
            break;
    }
    return cmd;
}

void PresetsProcessor::parameterChanged (const juce::String& parameterID, float)
{
    if (applyingPreset)
        return;
    if (parameterID == kVocal)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (kVocal)))
            loadVocalIndex (p->getIndex(), false);
    }
    else if (parameterID == kSendA)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (kSendA)))
            loadFxIndex (0, p->getIndex(), false, false);
    }
    else if (parameterID == kSendB)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (kSendB)))
            loadFxIndex (1, p->getIndex(), false, false);
    }
}

void PresetsProcessor::pushEngineFromParams()
{
    auto gf = [this] (const char* id, float fb) -> float
    {
        if (auto* v = apvts.getRawParameterValue (id))
            return v->load();
        return fb;
    };

    k3ch_presets::VocalSettings v;
    v.hpfHz = gf (kHpf, 85.0f);
    v.eqLowHz = gf (kEqLowHz, 280.0f);
    v.eqLowGainDb = gf (kEqLowG, 0.0f);
    v.eqLowQ = gf (kEqLowQ, 1.0f);
    v.eqMidHz = gf (kEqMidHz, 3500.0f);
    v.eqMidGainDb = gf (kEqMidG, 0.0f);
    v.eqMidQ = gf (kEqMidQ, 1.0f);
    v.eqAirHz = gf (kEqAirHz, 12000.0f);
    v.eqAirGainDb = gf (kEqAirG, 0.0f);
    v.compThresholdDb = gf (kCompTh, -18.0f);
    v.compRatio = gf (kCompRatio, 4.0f);
    v.compAttackMs = gf (kCompAtk, 5.0f);
    v.compReleaseMs = gf (kCompRel, 80.0f);
    v.compMakeupDb = gf (kCompMk, 2.0f);
    v.deessHz = gf (kDeessHz, 6500.0f);
    v.deessAmount = gf (kDeessAmt, 0.0f);
    v.satDrive = gf (kSatDrv, 0.0f);
    v.satMix = gf (kSatMix, 0.0f);
    engine.setVocal (v);

    auto applySend = [this] (int slot, const char* choiceId)
    {
        int idx = 0;
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (choiceId)))
            idx = p->getIndex();
        k3ch_presets::SendSettings s;
        if (const auto* fx = library.fxAt (idx))
            s = fx->settings;
        engine.setSend (slot, s);
    };
    applySend (0, kSendA);
    applySend (1, kSendB);

    k3ch_presets::MixSettings mix;
    mix.dryWet = gf (kDryWet, 1.0f);
    mix.returnMix = gf (kReturn, 1.0f);
    mix.outputGain = dbToLin (gf (kOutput, 0.0f));
    mix.sendGainA = dbToLin (gf (kSendAdB, -15.0f));
    mix.sendGainB = dbToLin (gf (kSendBdB, -15.0f));
    engine.setMix (mix);
}

void PresetsProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    pushEngineFromParams();
    peak[0].store (0.0f);
    peak[1].store (0.0f);
    rms[0].store (0.0f);
    rms[1].store (0.0f);
}

void PresetsProcessor::releaseResources()
{
    engine.reset();
}

bool PresetsProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return in == juce::AudioChannelSet::mono() || in == out;
}

void PresetsProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    midi.clear();
    juce::ScopedNoDenormals noDenormals;

    const auto numIn  = getTotalNumInputChannels();
    const auto numOut = getTotalNumOutputChannels();
    const auto n      = buffer.getNumSamples();

    for (int ch = numIn; ch < numOut; ++ch)
        buffer.clear (ch, 0, n);

    if (numIn == 1 && numOut >= 2)
        buffer.copyFrom (1, 0, buffer, 0, 0, n);

    if (isBypassed())
    {
        peak[0].store (0.0f);
        peak[1].store (0.0f);
        rms[0].store (0.0f);
        rms[1].store (0.0f);
        return;
    }

    pushEngineFromParams();

    float* chans[2] = {
        buffer.getWritePointer (0),
        buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : buffer.getWritePointer (0)
    };
    engine.process (chans, juce::jmin (2, buffer.getNumChannels()), n);

    peak[0].store (engine.lastPeakL());
    peak[1].store (engine.lastPeakR());
    rms[0].store (engine.lastRmsL());
    rms[1].store (engine.lastRmsR());
}

bool PresetsProcessor::isBypassed() const
{
    if (auto* p = apvts.getRawParameterValue (kBypass))
        return p->load() >= 0.5f;
    return false;
}

void PresetsProcessor::getMeterLevels (float& peakL, float& peakR, float& rmsL, float& rmsR) const
{
    peakL = peak[0].load();
    peakR = peak[1].load();
    rmsL = rms[0].load();
    rmsR = rms[1].load();
}

void PresetsProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void PresetsProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
    pushEngineFromParams();
}

juce::AudioProcessorEditor* PresetsProcessor::createEditor()
{
    return new PresetsEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PresetsProcessor();
}
