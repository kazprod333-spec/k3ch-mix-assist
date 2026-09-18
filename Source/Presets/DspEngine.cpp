#include "DspEngine.h"

#include <algorithm>

namespace k3ch_presets
{
namespace
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kMinHz = 20.0f;
constexpr float kMaxHz = 20000.0f;

float clampHz (float hz, double sr)
{
    const float ny = (float) sr * 0.45f;
    return std::clamp (hz, kMinHz, std::min (kMaxHz, ny));
}

float expCoeff (float ms, double sr)
{
    const float t = std::max (0.02f, ms) * 0.001f;
    return std::exp (-1.0f / (t * (float) sr));
}
} // namespace

void DspEngine::Biquad::setHighpass (double sampleRate, float hz, float q)
{
    hz = clampHz (hz, sampleRate);
    q = std::clamp (q, 0.3f, 8.0f);
    const float w = 2.0f * kPi * hz / (float) sampleRate;
    const float cw = std::cos (w);
    const float sw = std::sin (w);
    const float alpha = sw / (2.0f * q);
    const float a0 = 1.0f + alpha;
    b0 = ((1.0f + cw) * 0.5f) / a0;
    b1 = (-(1.0f + cw)) / a0;
    b2 = b0;
    a1 = (-2.0f * cw) / a0;
    a2 = (1.0f - alpha) / a0;
}

void DspEngine::Biquad::setLowpass (double sampleRate, float hz, float q)
{
    hz = clampHz (hz, sampleRate);
    q = std::clamp (q, 0.3f, 8.0f);
    const float w = 2.0f * kPi * hz / (float) sampleRate;
    const float cw = std::cos (w);
    const float sw = std::sin (w);
    const float alpha = sw / (2.0f * q);
    const float a0 = 1.0f + alpha;
    b0 = ((1.0f - cw) * 0.5f) / a0;
    b1 = (1.0f - cw) / a0;
    b2 = b0;
    a1 = (-2.0f * cw) / a0;
    a2 = (1.0f - alpha) / a0;
}

void DspEngine::Biquad::setPeak (double sampleRate, float hz, float gainDb, float q)
{
    if (std::abs (gainDb) < 0.05f)
    {
        setIdentity();
        return;
    }

    hz = clampHz (hz, sampleRate);
    q = std::clamp (q, 0.2f, 12.0f);
    const float A = std::pow (10.0f, gainDb / 40.0f);
    const float w = 2.0f * kPi * hz / (float) sampleRate;
    const float cw = std::cos (w);
    const float sw = std::sin (w);
    const float alpha = sw / (2.0f * q);
    const float a0 = 1.0f + alpha / A;
    b0 = (1.0f + alpha * A) / a0;
    b1 = (-2.0f * cw) / a0;
    b2 = (1.0f - alpha * A) / a0;
    a1 = (-2.0f * cw) / a0;
    a2 = (1.0f - alpha / A) / a0;
}

void DspEngine::Biquad::setHighShelf (double sampleRate, float hz, float gainDb)
{
    if (std::abs (gainDb) < 0.05f)
    {
        setIdentity();
        return;
    }

    hz = clampHz (hz, sampleRate);
    const float A = std::pow (10.0f, gainDb / 40.0f);
    const float w = 2.0f * kPi * hz / (float) sampleRate;
    const float cw = std::cos (w);
    const float sw = std::sin (w);
    const float beta = std::sqrt (A);
    const float a0 = (A + 1.0f) - (A - 1.0f) * cw + beta * sw;
    b0 = (A * ((A + 1.0f) + (A - 1.0f) * cw + beta * sw)) / a0;
    b1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * cw)) / a0;
    b2 = (A * ((A + 1.0f) + (A - 1.0f) * cw - beta * sw)) / a0;
    a1 = (2.0f * ((A - 1.0f) - (A + 1.0f) * cw)) / a0;
    a2 = ((A + 1.0f) - (A - 1.0f) * cw - beta * sw) / a0;
}

void DspEngine::SimpleReverb::prepare (double sampleRate)
{
    const float scale = (float) sampleRate / 44100.0f;
    const int combN[4] = { 1116, 1188, 1277, 1356 };
    const int combNOff[4] = { 1139, 1211, 1300, 1379 };
    const int apN[2] = { 556, 441 };
    const int apNOff[2] = { 579, 464 };

    for (int i = 0; i < 4; ++i)
    {
        combL[(size_t) i].setSize (std::max (1, (int) std::lround (combN[i] * scale)));
        combR[(size_t) i].setSize (std::max (1, (int) std::lround (combNOff[i] * scale)));
    }
    for (int i = 0; i < 2; ++i)
    {
        apL[(size_t) i].setSize (std::max (1, (int) std::lround (apN[i] * scale)));
        apR[(size_t) i].setSize (std::max (1, (int) std::lround (apNOff[i] * scale)));
    }
}

void DspEngine::SimpleReverb::set (float decayS, float dampAmt, double)
{
    decayS = std::clamp (decayS, 0.1f, 4.0f);
    // Map decay seconds to comb feedback. ~0.9s → ~0.78, 1.8s → ~0.88
    const float fb = std::clamp (0.55f + 0.18f * std::log2 (decayS + 0.2f), 0.4f, 0.93f);
    dampAmt = std::clamp (dampAmt, 0.05f, 0.9f);
    for (auto& c : combL)
    {
        c.feedback = fb;
        c.damp = dampAmt;
    }
    for (auto& c : combR)
    {
        c.feedback = fb;
        c.damp = dampAmt;
    }
}

void DspEngine::SimpleReverb::reset()
{
    for (auto& c : combL) { std::fill (c.buf.begin(), c.buf.end(), 0.0f); c.idx = 0; c.filter = 0; }
    for (auto& c : combR) { std::fill (c.buf.begin(), c.buf.end(), 0.0f); c.idx = 0; c.filter = 0; }
    for (auto& a : apL) { std::fill (a.buf.begin(), a.buf.end(), 0.0f); a.idx = 0; }
    for (auto& a : apR) { std::fill (a.buf.begin(), a.buf.end(), 0.0f); a.idx = 0; }
}

void DspEngine::SimpleReverb::process (float& L, float& R)
{
    float accL = 0, accR = 0;
    for (auto& c : combL) accL += c.tick (L);
    for (auto& c : combR) accR += c.tick (R);
    accL *= 0.35f;
    accR *= 0.35f;
    for (auto& a : apL) accL = a.tick (accL);
    for (auto& a : apR) accR = a.tick (accR);
    L = accL * 2.4f;
    R = accR * 2.4f;
}

void DspEngine::SimpleDelay::prepare (double sampleRate, float maxMs)
{
    delaySr = sampleRate > 1.0 ? sampleRate : 48000.0;
    const int n = std::max (64, (int) std::lround (delaySr * maxMs * 0.001) + 8);
    bufL.assign ((size_t) n, 0.0f);
    bufR.assign ((size_t) n, 0.0f);
    w = 0;
    hpL.setIdentity();
    hpR.setIdentity();
    lpL.setIdentity();
    lpR.setIdentity();
}

void DspEngine::SimpleDelay::reset()
{
    std::fill (bufL.begin(), bufL.end(), 0.0f);
    std::fill (bufR.begin(), bufR.end(), 0.0f);
    w = 0;
    hpL.reset(); hpR.reset(); lpL.reset(); lpR.reset();
}

void DspEngine::SimpleDelay::setFilters (double sampleRate, float hpHz, float lpHz)
{
    hpL.setHighpass (sampleRate, hpHz, 0.7f);
    hpR.b0 = hpL.b0; hpR.b1 = hpL.b1; hpR.b2 = hpL.b2; hpR.a1 = hpL.a1; hpR.a2 = hpL.a2;
    lpL.setLowpass (sampleRate, lpHz, 0.7f);
    lpR.b0 = lpL.b0; lpR.b1 = lpL.b1; lpR.b2 = lpL.b2; lpR.a1 = lpL.a1; lpR.a2 = lpL.a2;
}

void DspEngine::SimpleDelay::process (float inL, float inR, float delayMs, float fb, float& outL, float& outR)
{
    if (bufL.empty())
    {
        outL = 0;
        outR = 0;
        return;
    }

    const int maxDelay = (int) bufL.size();
    int d = (int) std::lround (delayMs * 0.001 * delaySr);
    d = std::clamp (d, 1, maxDelay - 2);
    int r = w - d;
    if (r < 0)
        r += maxDelay;

    float yL = bufL[(size_t) r];
    float yR = bufR[(size_t) r];
    yL = lpL.tick (hpL.tick (yL));
    yR = lpR.tick (hpR.tick (yR));

    fb = std::clamp (fb, 0.0f, 0.85f);
    bufL[(size_t) w] = inL + yL * fb;
    bufR[(size_t) w] = inR + yR * fb;
    if (++w >= maxDelay)
        w = 0;

    outL = yL;
    outR = yR;
}

void DspEngine::DeEsser::process (float& L, float& R, float hz, float amount, double sampleRate)
{
    if (amount <= 0.001f)
        return;

    const float wc = std::exp (-2.0f * kPi * clampHz (hz, sampleRate) / (float) sampleRate);
    lpL = wc * lpL + (1.0f - wc) * L;
    lpR = wc * lpR + (1.0f - wc) * R;
    const float hpL = L - lpL;
    const float hpR = R - lpR;
    const float det = std::max (std::abs (hpL), std::abs (hpR));
    const float at = expCoeff (2.0f, sampleRate);
    const float rel = expCoeff (40.0f, sampleRate);
    const float coeff = det > env ? at : rel;
    env = det + coeff * (env - det);

    const float thresh = 0.045f;
    float red = 0.0f;
    if (env > thresh)
        red = std::clamp ((env - thresh) / (thresh + 0.15f), 0.0f, 1.0f) * std::clamp (amount, 0.0f, 1.0f);

    L -= hpL * red;
    R -= hpR * red;
}

void DspEngine::Compressor::process (float& L, float& R, const VocalSettings& v, double sampleRate)
{
    const float peak = std::max (std::abs (L), std::abs (R));
    const float at = expCoeff (v.compAttackMs, sampleRate);
    const float rel = expCoeff (v.compReleaseMs, sampleRate);
    const float coeff = peak > env ? at : rel;
    env = peak + coeff * (env - peak);

    const float envDb = 20.0f * std::log10 (env + 1.0e-8f);
    const float over = envDb - v.compThresholdDb;
    float grDb = 0.0f;
    if (over > 0.0f)
    {
        const float ratio = std::max (1.0f, v.compRatio);
        grDb = over * (1.0f - 1.0f / ratio);
    }
    const float gr = dbToLin (-grDb) * dbToLin (v.compMakeupDb);
    L *= gr;
    R *= gr;
}

void DspEngine::Compressor::processSmash (float& L, float& R, float amount, double sampleRate)
{
    amount = std::clamp (amount, 0.0f, 1.0f);
    if (amount <= 0.001f)
        return;

    VocalSettings v;
    v.compThresholdDb = -24.0f + (1.0f - amount) * 14.0f;
    v.compRatio = 2.0f + amount * 8.0f;
    v.compAttackMs = 1.0f;
    v.compReleaseMs = 50.0f;
    v.compMakeupDb = amount * 8.0f;
    process (L, R, v, sampleRate);
}

float DspEngine::saturate (float x, float drive)
{
    const float d = 1.0f + std::clamp (drive, 0.0f, 1.0f) * 9.0f;
    const float den = std::tanh (d);
    if (den < 1.0e-6f)
        return x;
    return std::tanh (x * d) / den;
}

void DspEngine::setSend (int slot, const SendSettings& s)
{
    if (slot < 0 || slot > 1)
        return;
    sends[(size_t) slot].settings = s;
    sends[(size_t) slot].dirty = true;
}

void DspEngine::prepare (double sampleRate, int maxBlock, int channels)
{
    sr = sampleRate > 1.0 ? sampleRate : 48000.0;
    maxN = std::max (32, maxBlock);
    chans = std::clamp (channels, 1, 2);

    dryL.assign ((size_t) maxN, 0.0f);
    dryR.assign ((size_t) maxN, 0.0f);
    procL.assign ((size_t) maxN, 0.0f);
    procR.assign ((size_t) maxN, 0.0f);

    for (auto& slot : sends)
    {
        slot.reverb.prepare (sr);
        slot.delay.prepare (sr, 2000.0f);
        slot.tmpL.assign ((size_t) maxN, 0.0f);
        slot.tmpR.assign ((size_t) maxN, 0.0f);
        slot.dirty = true;
    }

    vocalDirty = true;
    reset();
}

void DspEngine::reset()
{
    hpf.reset(); hpf2.reset(); eqLow.reset(); eqMid.reset(); eqAir.reset();
    comp.reset();
    deess.reset();
    for (auto& slot : sends)
    {
        slot.hpf.reset(); slot.lpf.reset(); slot.harsh.reset();
        slot.deess.reset();
        slot.smash.reset();
        slot.reverb.reset();
        slot.delay.reset();
    }
    std::fill (dryL.begin(), dryL.end(), 0.0f);
    std::fill (dryR.begin(), dryR.end(), 0.0f);
    peakL = peakR = rmsL = rmsR = 0;
    primed = false;
}

void DspEngine::updateVocalCoeffs()
{
    Biquad c;
    c.setHighpass (sr, vocal.hpfHz, 0.707f);
    hpf.copyCoeffsFrom (c);
    hpf2.copyCoeffsFrom (c);
    c.setPeak (sr, vocal.eqLowHz, vocal.eqLowGainDb, vocal.eqLowQ);
    eqLow.copyCoeffsFrom (c);
    c.setPeak (sr, vocal.eqMidHz, vocal.eqMidGainDb, vocal.eqMidQ);
    eqMid.copyCoeffsFrom (c);
    c.setHighShelf (sr, vocal.eqAirHz, vocal.eqAirGainDb);
    eqAir.copyCoeffsFrom (c);
    vocalDirty = false;
}

void DspEngine::updateSendCoeffs (SendSlot& slot)
{
    Biquad c;
    const auto& s = slot.settings;
    c.setHighpass (sr, s.hpfHz, 0.707f);
    slot.hpf.copyCoeffsFrom (c);
    c.setLowpass (sr, s.lpfHz, 0.707f);
    slot.lpf.copyCoeffsFrom (c);
    c.setPeak (sr, s.harshHz, s.harshCutDb, 2.2f);
    slot.harsh.copyCoeffsFrom (c);
    slot.reverb.set (s.reverbDecayS, s.reverbDamp, sr);
    slot.delay.setFilters (sr, s.hpfHz, s.lpfHz);
    slot.dirty = false;
}

void DspEngine::processVocalSample (float& L, float& R)
{
    hpf.process (L, R);
    hpf2.process (L, R);
    eqLow.process (L, R);
    eqMid.process (L, R);
    eqAir.process (L, R);
    comp.process (L, R, vocal, sr);
    deess.process (L, R, vocal.deessHz, vocal.deessAmount, sr);

    if (vocal.satMix > 0.001f)
    {
        const float wetL = saturate (L, vocal.satDrive);
        const float wetR = saturate (R, vocal.satDrive);
        L += (wetL - L) * vocal.satMix;
        R += (wetR - R) * vocal.satMix;
    }
}

void DspEngine::processSend (SendSlot& slot, float* L, float* R, int n, float sendGain)
{
    const auto& s = slot.settings;
    if (! s.active || sendGain <= 1.0e-6f)
    {
        std::fill (L, L + n, 0.0f);
        std::fill (R, R + n, 0.0f);
        return;
    }

    if (slot.dirty)
        updateSendCoeffs (slot);

    for (int i = 0; i < n; ++i)
    {
        float xL = L[i] * sendGain;
        float xR = R[i] * sendGain;
        slot.hpf.process (xL, xR);
        slot.lpf.process (xL, xR);
        slot.harsh.process (xL, xR);
        slot.smash.processSmash (xL, xR, s.smash, sr);
        slot.deess.process (xL, xR, s.deessHz, s.deessAmount, sr);

        if (s.satMix > 0.001f)
        {
            const float wetL = saturate (xL, s.satDrive);
            const float wetR = saturate (xR, s.satDrive);
            xL += (wetL - xL) * s.satMix;
            xR += (wetR - xR) * s.satMix;
        }

        float wetL = xL;
        float wetR = xR;
        const bool hasVerb = s.reverbMix > 0.001f;
        const bool hasDelay = s.delayMix > 0.001f;

        if (hasVerb || hasDelay)
        {
            float vL = xL, vR = xR, dL = 0, dR = 0;
            if (hasVerb)
                slot.reverb.process (vL, vR);
            if (hasDelay)
                slot.delay.process (xL, xR, s.delayMs, s.delayFeedback, dL, dR);

            const float rm = std::clamp (s.reverbMix, 0.0f, 1.0f);
            const float dm = std::clamp (s.delayMix, 0.0f, 1.0f);
            const float dryKeep = std::max (0.0f, 1.0f - rm - dm);
            wetL = xL * dryKeep + vL * rm + dL * dm;
            wetR = xR * dryKeep + vR * rm + dR * dm;
        }

        L[i] = wetL;
        R[i] = wetR;
    }
}

void DspEngine::measure (float* const* io, int ch, int n)
{
    float pL = 0, pR = 0, sL = 0, sR = 0;
    const float* a = io[0];
    const float* b = ch > 1 ? io[1] : io[0];
    for (int i = 0; i < n; ++i)
    {
        const float x = std::abs (a[i]);
        const float y = std::abs (b[i]);
        pL = std::max (pL, x);
        pR = std::max (pR, y);
        sL += a[i] * a[i];
        sR += b[i] * b[i];
    }
    peakL = pL;
    peakR = pR;
    rmsL = std::sqrt (sL / (float) std::max (1, n));
    rmsR = std::sqrt (sR / (float) std::max (1, n));
}

void DspEngine::process (float* const* io, int numChannels, int numSamples)
{
    if (io == nullptr || numSamples <= 0 || numChannels <= 0)
        return;

    const int n = std::min (numSamples, maxN);

    if ((int) dryL.size() < n)
    {
        dryL.resize ((size_t) n, 0.0f);
        dryR.resize ((size_t) n, 0.0f);
        procL.resize ((size_t) n, 0.0f);
        procR.resize ((size_t) n, 0.0f);
    }

    const float* inL = io[0];
    const float* inR = numChannels > 1 ? io[1] : io[0];
    for (int i = 0; i < n; ++i)
    {
        dryL[(size_t) i] = inL[i];
        dryR[(size_t) i] = inR[i];
        procL[(size_t) i] = inL[i];
        procR[(size_t) i] = inR[i];
    }

    if (vocalDirty)
        updateVocalCoeffs();

    for (int i = 0; i < n; ++i)
        processVocalSample (procL[(size_t) i], procR[(size_t) i]);

    auto& a = sends[0];
    auto& b = sends[1];
    if ((int) a.tmpL.size() < n)
    {
        a.tmpL.resize ((size_t) n); a.tmpR.resize ((size_t) n);
        b.tmpL.resize ((size_t) n); b.tmpR.resize ((size_t) n);
    }

    for (int i = 0; i < n; ++i)
    {
        a.tmpL[(size_t) i] = procL[(size_t) i];
        a.tmpR[(size_t) i] = procR[(size_t) i];
        b.tmpL[(size_t) i] = procL[(size_t) i];
        b.tmpR[(size_t) i] = procR[(size_t) i];
    }

    const float smooth = 1.0f - expCoeff (20.0f, sr);
    if (! primed)
    {
        smDryWet = mix.dryWet;
        smRet = mix.returnMix;
        smOut = mix.outputGain;
        smSendA = mix.sendGainA;
        smSendB = mix.sendGainB;
        primed = true;
    }
    else
    {
        smDryWet += smooth * (mix.dryWet - smDryWet);
        smRet += smooth * (mix.returnMix - smRet);
        smOut += smooth * (mix.outputGain - smOut);
        smSendA += smooth * (mix.sendGainA - smSendA);
        smSendB += smooth * (mix.sendGainB - smSendB);
    }

    processSend (a, a.tmpL.data(), a.tmpR.data(), n, smSendA);
    processSend (b, b.tmpL.data(), b.tmpR.data(), n, smSendB);

    float* outL = io[0];
    float* outR = numChannels > 1 ? io[1] : nullptr;
    for (int i = 0; i < n; ++i)
    {
        const float wetL = procL[(size_t) i] + (a.tmpL[(size_t) i] + b.tmpL[(size_t) i]) * smRet;
        const float wetR = procR[(size_t) i] + (a.tmpR[(size_t) i] + b.tmpR[(size_t) i]) * smRet;
        float L = (dryL[(size_t) i] * (1.0f - smDryWet) + wetL * smDryWet) * smOut;
        float R = (dryR[(size_t) i] * (1.0f - smDryWet) + wetR * smDryWet) * smOut;
        L = std::clamp (L, -1.8f, 1.8f);
        R = std::clamp (R, -1.8f, 1.8f);
        outL[i] = L;
        if (outR != nullptr)
            outR[i] = R;
    }

    measure (io, numChannels, n);
}

} // namespace k3ch_presets
