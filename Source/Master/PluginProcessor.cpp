#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace
{
constexpr const char* kBypass = "bypass";
constexpr const char* kMode = "mode";
constexpr const char* kMasterPreset = "master_preset";
constexpr const char* kInput = "input_db";
constexpr const char* kLowHz = "eq_low_hz";
constexpr const char* kLowG = "eq_low_gain";
constexpr const char* kHighHz = "eq_high_hz";
constexpr const char* kHighG = "eq_high_gain";
constexpr const char* kMidOn = "eq_mid_on";
constexpr const char* kMidHz = "eq_mid_hz";
constexpr const char* kMidG = "eq_mid_gain";
constexpr const char* kMidQ = "eq_mid_q";
constexpr const char* kSatOn = "sat_on";
constexpr const char* kSatDrv = "sat_drive";
constexpr const char* kSatMix = "sat_mix";
constexpr const char* kWidth = "width";
constexpr const char* kBassMono = "bass_mono_hz";
constexpr const char* kClipOn = "clip_on";
constexpr const char* kClipCeil = "clip_ceiling";
constexpr const char* kLimOn = "lim_on";
constexpr const char* kLimCeil = "lim_ceiling";
constexpr const char* kLimRel = "lim_release";
constexpr const char* kLimLook = "lim_lookahead";
constexpr const char* kOutput = "output_db";
constexpr const char* kTpOn = "tp_on";
constexpr const char* kTpCeil = "tp_ceiling";
constexpr const char* kInsertPreset = "insert_preset";
constexpr const char* kTargetCh = "target_channel";

std::unique_ptr<juce::AudioParameterFloat> mkFloat (const char* id, const juce::String& name,
                                                    float min, float max, float def, float step)
{
    return std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id, 1 }, name,
        juce::NormalisableRange<float> (min, max, step), def);
}

juce::StringArray masterPresetNames()
{
    juce::StringArray names;
    for (const auto& p : k3ch_master::allMasterPresets())
        names.add (p.name);
    if (names.isEmpty())
        names.add ("Streaming -14 LUFS");
    return names;
}
} // namespace

MasterProcessor::MasterProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      insertLibrary (k3ch_master::InsertPlanLibrary::loadEmbedded()),
      apvts (*this, nullptr, "K3CHMaster", createLayout (insertLibrary))
{
    apvts.addParameterListener (kMasterPreset, this);
    loadMasterPreset (0, false);
    lastStatus = juce::String::fromUTF8 (
        "Pose ce plug-in sur le bus Master. Mode Inserts = plans FX Chain (pass-through). "
        "Passe en Master en fin de mix pour la cha\xc3\xae" "ne DSP.");
}

MasterProcessor::~MasterProcessor()
{
    apvts.removeParameterListener (kMasterPreset, this);
}

juce::AudioProcessorValueTreeState::ParameterLayout MasterProcessor::createLayout (
    const k3ch_master::InsertPlanLibrary& lib)
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto insertNames = lib.names();
    if (insertNames.isEmpty())
        insertNames.add ("Voix lead");

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { kBypass, 1 }, "Bypass", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { kMode, 1 }, "Mode",
        juce::StringArray { "Inserts / Mix", "Master" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { kMasterPreset, 1 },
        juce::String::fromUTF8 ("Preset master"),
        masterPresetNames(), 0));

    params.push_back (mkFloat (kInput, juce::String::fromUTF8 ("Gain d'entr\xc3\xa9" "e"),
                               -18.0f, 12.0f, 0.0f, 0.1f));
    params.push_back (mkFloat (kLowHz, juce::String::fromUTF8 ("Grave Hz"), 40.0f, 400.0f, 120.0f, 1.0f));
    params.push_back (mkFloat (kLowG, juce::String::fromUTF8 ("Grave"), -6.0f, 6.0f, 0.0f, 0.05f));
    params.push_back (mkFloat (kHighHz, juce::String::fromUTF8 ("Aigu Hz"), 2000.0f, 16000.0f, 8000.0f, 1.0f));
    params.push_back (mkFloat (kHighG, juce::String::fromUTF8 ("Aigu"), -6.0f, 6.0f, 0.0f, 0.05f));
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { kMidOn, 1 },
        juce::String::fromUTF8 ("Bande m\xc3\xa9" "dium"), false));
    params.push_back (mkFloat (kMidHz, juce::String::fromUTF8 ("M\xc3\xa9" "dium Hz"),
                               200.0f, 8000.0f, 2500.0f, 1.0f));
    params.push_back (mkFloat (kMidG, juce::String::fromUTF8 ("M\xc3\xa9" "dium"), -6.0f, 6.0f, 0.0f, 0.05f));
    params.push_back (mkFloat (kMidQ, juce::String::fromUTF8 ("Q m\xc3\xa9" "dium"), 0.3f, 4.0f, 1.0f, 0.01f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { kSatOn, 1 }, "Saturation", false));
    params.push_back (mkFloat (kSatDrv, "Drive", 0.0f, 1.0f, 0.12f, 0.01f));
    params.push_back (mkFloat (kSatMix, "Mix sat", 0.0f, 1.0f, 0.15f, 0.01f));
    params.push_back (mkFloat (kWidth, juce::String::fromUTF8 ("Largeur st\xc3\xa9r\xc3\xa9o"),
                               0.0f, 1.8f, 1.0f, 0.01f));
    params.push_back (mkFloat (kBassMono, "Basse mono", 40.0f, 250.0f, 120.0f, 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { kClipOn, 1 }, "Soft clip", true));
    params.push_back (mkFloat (kClipCeil, "Plafond clip", -3.0f, 0.0f, -0.6f, 0.05f));
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { kLimOn, 1 }, "Limiteur", true));
    params.push_back (mkFloat (kLimCeil, "Plafond limiteur", -3.0f, 0.0f, -1.0f, 0.05f));
    params.push_back (mkFloat (kLimRel, juce::String::fromUTF8 ("Rel\xc3\xa2" "chement"),
                               10.0f, 400.0f, 80.0f, 1.0f));
    params.push_back (mkFloat (kLimLook, "Lookahead", 0.5f, 8.0f, 2.0f, 0.1f));
    params.push_back (mkFloat (kOutput, "Gain de sortie", -12.0f, 12.0f, 0.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { kTpOn, 1 }, "True peak", true));
    params.push_back (mkFloat (kTpCeil, "Plafond TP", -3.0f, 0.0f, -1.0f, 0.05f));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { kInsertPreset, 1 },
        juce::String::fromUTF8 ("Cha\xc3\xae" "ne insert"),
        insertNames, 0));
    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { kTargetCh, 1 },
        "Canal cible", 1, 125, 1));

    return { params.begin(), params.end() };
}

void MasterProcessor::setFloatParam (const juce::String& id, float value)
{
    if (auto* p = dynamic_cast<juce::AudioParameterFloat*> (apvts.getParameter (id)))
        p->setValueNotifyingHost (p->getNormalisableRange().convertTo0to1 (value));
}

void MasterProcessor::setChoiceParam (const juce::String& id, int index)
{
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (id)))
        p->setValueNotifyingHost (p->convertTo0to1 ((float) juce::jlimit (0, p->choices.size() - 1, index)));
}

void MasterProcessor::setBoolParam (const juce::String& id, bool value)
{
    if (auto* p = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter (id)))
        p->setValueNotifyingHost (value ? 1.0f : 0.0f);
}

void MasterProcessor::setModeMaster (bool master, bool notifyHost)
{
    juce::ignoreUnused (notifyHost);
    setChoiceParam (kMode, master ? 1 : 0);
    lastStatus = master
        ? juce::String::fromUTF8 ("Mode Master \xe2\x80\x94 cha\xc3\xae" "ne DSP active sur CE bus")
        : juce::String::fromUTF8 (
              "Mode Inserts / Mix \xe2\x80\x94 pass-through. Plans FX Chain seulement. "
              "Ce plug-in ne pilote pas les autres canaux.");
}

void MasterProcessor::loadMasterPreset (int index, bool notifyHost)
{
    juce::ignoreUnused (notifyHost);
    const auto* p = k3ch_master::masterPresetAt (index);
    if (p == nullptr)
        return;

    const juce::ScopedValueSetter<bool> sv (applyingPreset, true);
    setChoiceParam (kMasterPreset, index);
    const auto& s = p->settings;
    setFloatParam (kInput, s.inputDb);
    setFloatParam (kLowHz, s.lowShelfHz);
    setFloatParam (kLowG, s.lowShelfDb);
    setFloatParam (kHighHz, s.highShelfHz);
    setFloatParam (kHighG, s.highShelfDb);
    setBoolParam (kMidOn, s.midOn);
    setFloatParam (kMidHz, s.midHz);
    setFloatParam (kMidG, s.midGainDb);
    setFloatParam (kMidQ, s.midQ);
    setBoolParam (kSatOn, s.satOn);
    setFloatParam (kSatDrv, s.satDrive);
    setFloatParam (kSatMix, s.satMix);
    setFloatParam (kWidth, s.width);
    setFloatParam (kBassMono, s.bassMonoHz);
    setBoolParam (kClipOn, s.clipOn);
    setFloatParam (kClipCeil, s.clipCeilingDb);
    setBoolParam (kLimOn, s.limOn);
    setFloatParam (kLimCeil, s.limCeilingDb);
    setFloatParam (kLimRel, s.limReleaseMs);
    setFloatParam (kLimLook, s.limLookaheadMs);
    setFloatParam (kOutput, s.outputDb);
    setBoolParam (kTpOn, s.tpOn);
    setFloatParam (kTpCeil, s.tpCeilingDb);
    pushEngineFromParams();

    lastStatus = juce::String::fromUTF8 ("Preset master : ") + juce::String (p->name);
    if (! isMasterMode())
        lastStatus += juce::String::fromUTF8 (
            " \xe2\x80\x94 passe en mode Master pour l\xe2\x80\x99" "entendre");
}

void MasterProcessor::parameterChanged (const juce::String& parameterID, float)
{
    if (applyingPreset)
        return;
    if (parameterID == kMasterPreset)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (kMasterPreset)))
            loadMasterPreset (p->getIndex(), false);
    }
}

void MasterProcessor::pushEngineFromParams()
{
    auto gf = [this] (const juce::String& id, float fb) -> float
    {
        if (auto* v = apvts.getRawParameterValue (id))
            return v->load();
        return fb;
    };

    k3ch_master::MasterSettings s;
    s.inputDb = gf (kInput, 0.0f);
    s.lowShelfHz = gf (kLowHz, 120.0f);
    s.lowShelfDb = gf (kLowG, 0.0f);
    s.highShelfHz = gf (kHighHz, 8000.0f);
    s.highShelfDb = gf (kHighG, 0.0f);
    s.midOn = gf (kMidOn, 0.0f) >= 0.5f;
    s.midHz = gf (kMidHz, 2500.0f);
    s.midGainDb = gf (kMidG, 0.0f);
    s.midQ = gf (kMidQ, 1.0f);
    s.satOn = gf (kSatOn, 0.0f) >= 0.5f;
    s.satDrive = gf (kSatDrv, 0.0f);
    s.satMix = gf (kSatMix, 0.0f);
    s.width = gf (kWidth, 1.0f);
    s.bassMonoHz = gf (kBassMono, 120.0f);
    s.clipOn = gf (kClipOn, 1.0f) >= 0.5f;
    s.clipCeilingDb = gf (kClipCeil, -0.6f);
    s.limOn = gf (kLimOn, 1.0f) >= 0.5f;
    s.limCeilingDb = gf (kLimCeil, -1.0f);
    s.limReleaseMs = gf (kLimRel, 80.0f);
    s.limLookaheadMs = gf (kLimLook, 2.0f);
    s.outputDb = gf (kOutput, 0.0f);
    s.tpOn = gf (kTpOn, 1.0f) >= 0.5f;
    s.tpCeilingDb = gf (kTpCeil, -1.0f);
    engine.setSettings (s);
}

void MasterProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    pushEngineFromParams();
    peak[0].store (0.0f);
    peak[1].store (0.0f);
    rms[0].store (0.0f);
    rms[1].store (0.0f);
}

void MasterProcessor::releaseResources()
{
    engine.reset();
}

bool MasterProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return in == juce::AudioChannelSet::mono() || in == out;
}

void MasterProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
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

    if (isBypassed() || ! isMasterMode())
    {
        float pL = 0, pR = 0, sL = 0, sR = 0;
        const int ch = juce::jmin (2, buffer.getNumChannels());
        for (int c = 0; c < ch; ++c)
        {
            const auto* d = buffer.getReadPointer (c);
            float pk = 0, sum = 0;
            for (int i = 0; i < n; ++i)
            {
                pk = juce::jmax (pk, std::abs (d[i]));
                sum += d[i] * d[i];
            }
            if (c == 0) { pL = pk; sL = sum; }
            else { pR = pk; sR = sum; }
        }
        if (ch < 2) { pR = pL; sR = sL; }
        peak[0].store (pL);
        peak[1].store (pR);
        rms[0].store (std::sqrt (sL / (float) juce::jmax (1, n)));
        rms[1].store (std::sqrt (sR / (float) juce::jmax (1, n)));
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

bool MasterProcessor::isBypassed() const
{
    if (auto* p = apvts.getRawParameterValue (kBypass))
        return p->load() >= 0.5f;
    return false;
}

bool MasterProcessor::isMasterMode() const
{
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (kMode)))
        return p->getIndex() >= 1;
    return false;
}

int MasterProcessor::currentInsertIndex() const
{
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (kInsertPreset)))
        return p->getIndex();
    return 0;
}

int MasterProcessor::currentTargetChannel() const
{
    if (auto* p = dynamic_cast<juce::AudioParameterInt*> (apvts.getParameter (kTargetCh)))
        return p->get();
    return 1;
}

void MasterProcessor::getMeterLevels (float& peakL, float& peakR, float& rmsL, float& rmsR) const
{
    peakL = peak[0].load();
    peakR = peak[1].load();
    rmsL = rms[0].load();
    rmsR = rms[1].load();
}

float MasterProcessor::getLimiterGrDb() const
{
    return isMasterMode() ? engine.lastLimiterGrDb() : 0.0f;
}

juce::String MasterProcessor::currentPlanText() const
{
    if (const auto* p = insertLibrary.at (currentInsertIndex()))
        return p->planText (currentTargetChannel(), targetChannelName);
    return {};
}

void MasterProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("targetChannelName", targetChannelName, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void MasterProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
            targetChannelName = apvts.state.getProperty ("targetChannelName", targetChannelName).toString();
        }
    }
    pushEngineFromParams();
}

juce::AudioProcessorEditor* MasterProcessor::createEditor()
{
    return new MasterEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MasterProcessor();
}
