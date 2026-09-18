#pragma once

#include <array>
#include <cmath>
#include <vector>

namespace k3ch_presets
{

struct VocalSettings
{
    float hpfHz = 80.0f;
    float eqLowHz = 280.0f;
    float eqLowGainDb = 0.0f;
    float eqLowQ = 1.0f;
    float eqMidHz = 3500.0f;
    float eqMidGainDb = 0.0f;
    float eqMidQ = 1.0f;
    float eqAirHz = 12000.0f;
    float eqAirGainDb = 0.0f;
    float compThresholdDb = -18.0f;
    float compRatio = 4.0f;
    float compAttackMs = 5.0f;
    float compReleaseMs = 80.0f;
    float compMakeupDb = 2.0f;
    float deessHz = 6500.0f;
    float deessAmount = 0.0f;
    float satDrive = 0.0f;
    float satMix = 0.0f;
};

struct SendSettings
{
    bool active = false;
    float hpfHz = 20.0f;
    float lpfHz = 20000.0f;
    float smash = 0.0f;
    float deessHz = 6500.0f;
    float deessAmount = 0.0f;
    float satDrive = 0.0f;
    float satMix = 0.0f;
    float reverbMix = 0.0f;
    float reverbDecayS = 0.8f;
    float reverbDamp = 0.5f;
    float delayMix = 0.0f;
    float delayMs = 125.0f;
    float delayFeedback = 0.2f;
    float harshHz = 6500.0f;
    float harshCutDb = 0.0f;
};

struct MixSettings
{
    float dryWet = 1.0f;
    float returnMix = 1.0f;
    float outputGain = 1.0f;
    float sendGainA = 0.0f;
    float sendGainB = 0.0f;
};

class DspEngine
{
public:
    void prepare (double sampleRate, int maxBlock, int channels);
    void reset();

    void setVocal (const VocalSettings& s) { vocal = s; vocalDirty = true; }
    void setSend (int slot, const SendSettings& s);
    void setMix (const MixSettings& s) { mix = s; }

    void process (float* const* io, int numChannels, int numSamples);

    float lastPeakL() const { return peakL; }
    float lastPeakR() const { return peakR; }
    float lastRmsL() const { return rmsL; }
    float lastRmsR() const { return rmsR; }

private:
    struct Biquad
    {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
        float z1 = 0, z2 = 0;

        void reset() { z1 = z2 = 0; }

        float tick (float x)
        {
            const float y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }

        void setIdentity()
        {
            b0 = 1; b1 = b2 = a1 = a2 = 0;
        }

        void setHighpass (double sr, float hz, float q);
        void setLowpass (double sr, float hz, float q);
        void setPeak (double sr, float hz, float gainDb, float q);
        void setHighShelf (double sr, float hz, float gainDb);
    };

    struct StereoBiquad
    {
        Biquad l, r;
        void reset() { l.reset(); r.reset(); }
        void copyCoeffsFrom (const Biquad& c)
        {
            l.b0 = r.b0 = c.b0; l.b1 = r.b1 = c.b1; l.b2 = r.b2 = c.b2;
            l.a1 = r.a1 = c.a1; l.a2 = r.a2 = c.a2;
        }
        void process (float& L, float& R)
        {
            L = l.tick (L);
            R = r.tick (R);
        }
    };

    struct Comb
    {
        std::vector<float> buf;
        int idx = 0;
        float filter = 0;
        float feedback = 0.5f;
        float damp = 0.4f;

        void setSize (int n)
        {
            buf.assign ((size_t) std::max (1, n), 0.0f);
            idx = 0;
            filter = 0;
        }

        float tick (float in)
        {
            float y = buf[(size_t) idx];
            filter = y * (1.0f - damp) + filter * damp;
            buf[(size_t) idx] = in + filter * feedback;
            if (++idx >= (int) buf.size())
                idx = 0;
            return y;
        }
    };

    struct Allpass
    {
        std::vector<float> buf;
        int idx = 0;
        float g = 0.5f;

        void setSize (int n)
        {
            buf.assign ((size_t) std::max (1, n), 0.0f);
            idx = 0;
        }

        float tick (float in)
        {
            const float bufout = buf[(size_t) idx];
            const float y = -in + bufout;
            buf[(size_t) idx] = in + bufout * g;
            if (++idx >= (int) buf.size())
                idx = 0;
            return y;
        }
    };

    struct SimpleReverb
    {
        std::array<Comb, 4> combL {}, combR {};
        std::array<Allpass, 2> apL {}, apR {};

        void prepare (double sr);
        void set (float decayS, float damp, double sr);
        void reset();
        void process (float& L, float& R);
    };

    struct SimpleDelay
    {
        std::vector<float> bufL, bufR;
        int w = 0;
        double delaySr = 48000.0;
        Biquad hpL, hpR, lpL, lpR;

        void prepare (double sr, float maxMs);
        void reset();
        void setFilters (double sr, float hpHz, float lpHz);
        void process (float inL, float inR, float delayMs, float fb, float& outL, float& outR);
    };

    struct DeEsser
    {
        float lpL = 0, lpR = 0, env = 0;
        void reset() { lpL = lpR = env = 0; }
        void process (float& L, float& R, float hz, float amount, double sr);
    };

    struct Compressor
    {
        float env = 0;
        void reset() { env = 0; }
        void process (float& L, float& R, const VocalSettings& v, double sr);
        void processSmash (float& L, float& R, float amount, double sr);
    };

    struct SendSlot
    {
        SendSettings settings;
        StereoBiquad hpf, lpf, harsh;
        DeEsser deess;
        Compressor smash;
        SimpleReverb reverb;
        SimpleDelay delay;
        std::vector<float> tmpL, tmpR;
        bool dirty = true;
    };

    void updateVocalCoeffs();
    void updateSendCoeffs (SendSlot& slot);
    void processVocalSample (float& L, float& R);
    void processSend (SendSlot& slot, float* L, float* R, int n, float sendGain);
    void measure (float* const* io, int ch, int n);

    static float dbToLin (float db) { return std::pow (10.0f, db / 20.0f); }
    static float saturate (float x, float drive);

    double sr = 48000.0;
    int maxN = 512;
    int chans = 2;

    VocalSettings vocal;
    MixSettings mix;
    bool vocalDirty = true;

    StereoBiquad hpf, hpf2, eqLow, eqMid, eqAir;
    Compressor comp;
    DeEsser deess;

    std::array<SendSlot, 2> sends;
    std::vector<float> dryL, dryR, procL, procR;

    float peakL = 0, peakR = 0, rmsL = 0, rmsR = 0;
    float smDryWet = 1, smRet = 1, smOut = 1, smSendA = 0, smSendB = 0;
    bool primed = false;
};

} // namespace k3ch_presets
