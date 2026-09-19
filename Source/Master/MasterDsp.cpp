#include "MasterDsp.h"

#include <algorithm>

namespace k3ch_master
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

float smoothToward (float current, float target, float coeff)
{
    return target + coeff * (current - target);
}
} // namespace

void MasterDsp::Biquad::setLowShelf (double sampleRate, float hz, float gainDb)
{
    if (std::abs (gainDb) < 0.04f)
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
    const float a0 = (A + 1.0f) + (A - 1.0f) * cw + beta * sw;
    b0 = (A * ((A + 1.0f) - (A - 1.0f) * cw + beta * sw)) / a0;
    b1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * cw)) / a0;
    b2 = (A * ((A + 1.0f) - (A - 1.0f) * cw - beta * sw)) / a0;
    a1 = (-2.0f * ((A - 1.0f) + (A + 1.0f) * cw)) / a0;
    a2 = ((A + 1.0f) + (A - 1.0f) * cw - beta * sw) / a0;
}

void MasterDsp::Biquad::setHighShelf (double sampleRate, float hz, float gainDb)
{
    if (std::abs (gainDb) < 0.04f)
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

void MasterDsp::Biquad::setPeak (double sampleRate, float hz, float gainDb, float q)
{
    if (std::abs (gainDb) < 0.04f)
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

void MasterDsp::Biquad::setHighpass (double sampleRate, float hz, float q)
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

void MasterDsp::LookaheadLimiter::prepare (double sampleRate, float maxLookaheadMs)
{
    const int n = std::max (64, (int) std::lround (sampleRate * maxLookaheadMs * 0.001) + 16);
    delayL.assign ((size_t) n, 0.0f);
    delayR.assign ((size_t) n, 0.0f);
    peakBuf.assign ((size_t) n, 0.0f);
    delayN = n;
    reset();
}

void MasterDsp::LookaheadLimiter::reset()
{
    std::fill (delayL.begin(), delayL.end(), 0.0f);
    std::fill (delayR.begin(), delayR.end(), 0.0f);
    std::fill (peakBuf.begin(), peakBuf.end(), 0.0f);
    w = 0;
    env = 1.0f;
    lastGrDb = 0.0f;
}

void MasterDsp::LookaheadLimiter::process (float& L, float& R, float ceilingLin,
                                           float releaseMs, float lookaheadMs, double sampleRate)
{
    if (delayL.empty())
        return;

    const int maxN = (int) delayL.size();
    const int look = std::clamp ((int) std::lround (lookaheadMs * 0.001 * sampleRate),
                                 1, maxN - 2);

    const float inPeak = std::max (std::abs (L), std::abs (R));
    delayL[(size_t) w] = L;
    delayR[(size_t) w] = R;
    peakBuf[(size_t) w] = inPeak;

    float futurePeak = 0.0f;
    for (int i = 0; i < look; ++i)
    {
        int idx = w - i;
        if (idx < 0)
            idx += maxN;
        futurePeak = std::max (futurePeak, peakBuf[(size_t) idx]);
    }

    float target = 1.0f;
    if (futurePeak > ceilingLin && futurePeak > 1.0e-8f)
        target = ceilingLin / futurePeak;

    const float at = expCoeff (0.08f, sampleRate);
    const float rel = expCoeff (releaseMs, sampleRate);
    env = target + (target < env ? at : rel) * (env - target);
    env = std::clamp (env, 0.0f, 1.0f);

    int r = w - look;
    if (r < 0)
        r += maxN;
    L = delayL[(size_t) r] * env;
    R = delayR[(size_t) r] * env;

    const float grDb = env < 0.999f ? -20.0f * std::log10 (std::max (env, 1.0e-6f)) : 0.0f;
    lastGrDb += 0.2f * (grDb - lastGrDb);

    if (++w >= maxN)
        w = 0;
}

float MasterDsp::saturate (float x, float drive)
{
    const float d = 1.0f + std::clamp (drive, 0.0f, 1.0f) * 8.0f;
    const float den = std::tanh (d);
    if (den < 1.0e-6f)
        return x;
    return std::tanh (x * d) / den;
}

float MasterDsp::softClip (float x, float ceilingLin)
{
    ceilingLin = std::max (1.0e-4f, ceilingLin);
    return ceilingLin * std::tanh (x / ceilingLin);
}

void MasterDsp::prepare (double sampleRate, int maxBlock, int channels)
{
    sr = sampleRate > 1.0 ? sampleRate : 48000.0;
    maxN = std::max (32, maxBlock);
    chans = std::clamp (channels, 1, 2);
    limiter.prepare (sr, 12.0f);
    dirty = true;
    reset();
}

void MasterDsp::reset()
{
    lowShelf.reset();
    highShelf.reset();
    midPeak.reset();
    sideHp.reset();
    sideHp2.reset();
    limiter.reset();
    prevL = prevR = 0;
    peakL = peakR = rmsL = rmsR = 0;
    limiterGrDb = 0;
    primed = false;
    dirty = true;
}

void MasterDsp::setSettings (const MasterSettings& s)
{
    set = s;
    dirty = true;
}

void MasterDsp::updateCoeffs()
{
    Biquad c;
    c.setLowShelf (sr, set.lowShelfHz, set.lowShelfDb);
    lowShelf.copyCoeffsFrom (c);
    c.setHighShelf (sr, set.highShelfHz, set.highShelfDb);
    highShelf.copyCoeffsFrom (c);
    if (set.midOn)
        c.setPeak (sr, set.midHz, set.midGainDb, set.midQ);
    else
        c.setIdentity();
    midPeak.copyCoeffsFrom (c);
    sideHp.setHighpass (sr, set.bassMonoHz, 0.707f);
    sideHp2.setHighpass (sr, set.bassMonoHz, 0.707f);
    dirty = false;
}

void MasterDsp::processSample (float& L, float& R)
{
    const float inG = smIn;
    L *= inG;
    R *= inG;

    lowShelf.process (L, R);
    if (set.midOn)
        midPeak.process (L, R);
    highShelf.process (L, R);

    if (set.satOn && set.satMix > 0.001f)
    {
        const float wetL = saturate (L, set.satDrive);
        const float wetR = saturate (R, set.satDrive);
        L += (wetL - L) * set.satMix;
        R += (wetR - R) * set.satMix;
    }

    {
        float mid = 0.5f * (L + R);
        float side = 0.5f * (L - R);
        side = sideHp2.tick (sideHp.tick (side));
        const float width = std::clamp (smWidth, 0.0f, 2.0f);
        side *= width;
        L = mid + side;
        R = mid - side;
    }

    if (set.clipOn)
    {
        const float ceilLin = dbToLin (set.clipCeilingDb);
        L = softClip (L, ceilLin);
        R = softClip (R, ceilLin);
    }

    if (set.limOn)
    {
        limiter.process (L, R, dbToLin (set.limCeilingDb),
                         set.limReleaseMs, set.limLookaheadMs, sr);
        limiterGrDb = limiter.lastGrDb;
    }
    else
    {
        limiterGrDb += 0.2f * (0.0f - limiterGrDb);
    }

    if (set.tpOn)
    {
        const float tpCeil = dbToLin (set.tpCeilingDb);
        const float midL = 0.5f * (prevL + L);
        const float midR = 0.5f * (prevR + R);
        const float peak = std::max (std::max (std::abs (L), std::abs (R)),
                                     std::max (std::abs (midL), std::abs (midR)));
        if (peak > tpCeil && peak > 1.0e-8f)
        {
            const float g = tpCeil / peak;
            L *= g;
            R *= g;
        }
    }

    prevL = L;
    prevR = R;
    L *= smOut;
    R *= smOut;
}

void MasterDsp::measure (float* const* io, int ch, int n)
{
    float pL = 0, pR = 0, sL = 0, sR = 0;
    for (int i = 0; i < n; ++i)
    {
        const float aL = std::abs (io[0][i]);
        const float aR = ch > 1 ? std::abs (io[1][i]) : aL;
        pL = std::max (pL, aL);
        pR = std::max (pR, aR);
        sL += io[0][i] * io[0][i];
        if (ch > 1)
            sR += io[1][i] * io[1][i];
        else
            sR += io[0][i] * io[0][i];
    }
    peakL = pL;
    peakR = pR;
    rmsL = std::sqrt (sL / (float) std::max (1, n));
    rmsR = std::sqrt (sR / (float) std::max (1, n));
}

void MasterDsp::process (float* const* io, int numChannels, int numSamples)
{
    if (io == nullptr || numSamples <= 0 || numChannels <= 0)
        return;

    if (dirty)
        updateCoeffs();

    const float targetIn = dbToLin (set.inputDb);
    const float targetOut = dbToLin (set.outputDb);
    const float targetW = set.width;
    const float coeff = primed ? 0.92f : 0.0f;
    smIn = smoothToward (smIn, targetIn, coeff);
    smOut = smoothToward (smOut, targetOut, coeff);
    smWidth = smoothToward (smWidth, targetW, coeff);
    primed = true;

    const int ch = std::min (numChannels, 2);
    for (int i = 0; i < numSamples; ++i)
    {
        float L = io[0][i];
        float R = ch > 1 ? io[1][i] : L;
        processSample (L, R);
        io[0][i] = L;
        if (ch > 1)
            io[1][i] = R;
    }

    measure (io, ch, numSamples);
}

} // namespace k3ch_master
