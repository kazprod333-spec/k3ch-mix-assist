#pragma once

#include <array>
#include <cmath>
#include <vector>

namespace k3ch_master
{

struct MasterSettings
{
    float inputDb = 0.0f;
    float lowShelfHz = 120.0f;
    float lowShelfDb = 0.0f;
    float highShelfHz = 8000.0f;
    float highShelfDb = 0.0f;
    bool midOn = false;
    float midHz = 2500.0f;
    float midGainDb = 0.0f;
    float midQ = 1.0f;
    bool satOn = false;
    float satDrive = 0.12f;
    float satMix = 0.15f;
    float width = 1.0f;
    float bassMonoHz = 120.0f;
    bool clipOn = true;
    float clipCeilingDb = -0.6f;
    bool limOn = true;
    float limCeilingDb = -1.0f;
    float limReleaseMs = 80.0f;
    float limLookaheadMs = 2.0f;
    float outputDb = 0.0f;
    bool tpOn = true;
    float tpCeilingDb = -1.0f;
};

class MasterDsp
{
public:
    void prepare (double sampleRate, int maxBlock, int channels);
    void reset();
    void setSettings (const MasterSettings& s);

    void process (float* const* io, int numChannels, int numSamples);

    float lastPeakL() const { return peakL; }
    float lastPeakR() const { return peakR; }
    float lastRmsL() const { return rmsL; }
    float lastRmsR() const { return rmsR; }
    float lastLimiterGrDb() const { return limiterGrDb; }

private:
    struct Biquad
    {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
        float z1 = 0, z2 = 0;

        void reset() { z1 = z2 = 0; }
        void setIdentity() { b0 = 1; b1 = b2 = a1 = a2 = 0; }

        float tick (float x)
        {
            const float y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }

        void setLowShelf (double sr, float hz, float gainDb);
        void setHighShelf (double sr, float hz, float gainDb);
        void setPeak (double sr, float hz, float gainDb, float q);
        void setHighpass (double sr, float hz, float q);
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

    struct LookaheadLimiter
    {
        std::vector<float> delayL, delayR, peakBuf;
        int w = 0;
        int delayN = 96;
        float env = 1.0f;
        float lastGrDb = 0.0f;

        void prepare (double sr, float maxLookaheadMs);
        void reset();
        void process (float& L, float& R, float ceilingLin, float releaseMs,
                      float lookaheadMs, double sr);
    };

    void updateCoeffs();
    void processSample (float& L, float& R);
    void measure (float* const* io, int ch, int n);

    static float dbToLin (float db) { return std::pow (10.0f, db / 20.0f); }
    static float saturate (float x, float drive);
    static float softClip (float x, float ceilingLin);

    double sr = 48000.0;
    int maxN = 512;
    int chans = 2;

    MasterSettings set;
    bool dirty = true;

    StereoBiquad lowShelf, highShelf, midPeak;
    Biquad sideHp, sideHp2;
    LookaheadLimiter limiter;

    float prevL = 0, prevR = 0;
    float smIn = 1, smOut = 1, smWidth = 1;
    float peakL = 0, peakR = 0, rmsL = 0, rmsR = 0;
    float limiterGrDb = 0;
    bool primed = false;
};

} // namespace k3ch_master
