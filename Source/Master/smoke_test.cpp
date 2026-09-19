#include "MasterDsp.h"
#include "MasterPresets.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using k3ch_master::MasterDsp;
using k3ch_master::MasterSettings;
using k3ch_master::allMasterPresets;
using k3ch_master::masterPresetAt;
using k3ch_master::findMasterPresetIndex;

namespace
{
constexpr double kSr = 48000.0;
constexpr int kBlock = 512;
constexpr float kPi = 3.14159265f;

float rmsOf (const std::vector<float>& x, int start, int n)
{
    double s = 0;
    int c = 0;
    for (int i = start; i < start + n && i < (int) x.size(); ++i)
    {
        s += (double) x[(size_t) i] * (double) x[(size_t) i];
        ++c;
    }
    return c > 0 ? std::sqrt ((float) (s / c)) : 0.0f;
}

float peakOf (const std::vector<float>& x, int start, int n)
{
    float p = 0;
    for (int i = start; i < start + n && i < (int) x.size(); ++i)
        p = std::max (p, std::abs (x[(size_t) i]));
    return p;
}

std::vector<float> renderSine (MasterDsp& eng, const MasterSettings& set,
                               float hz, float ampL, float ampR, int samples)
{
    eng.prepare (kSr, kBlock, 2);
    eng.setSettings (set);

    std::vector<float> L ((size_t) samples), R ((size_t) samples);
    for (int i = 0; i < samples; ++i)
    {
        const float s = std::sin (2.0f * kPi * hz * (float) i / (float) kSr);
        L[(size_t) i] = ampL * s;
        R[(size_t) i] = ampR * s;
    }
    for (int i = 0; i < samples; i += kBlock)
    {
        const int n = std::min (kBlock, samples - i);
        float* ch[2] = { L.data() + i, R.data() + i };
        eng.process (ch, 2, n);
    }
    return L;
}

bool expect (bool cond, const char* msg)
{
    std::cout << (cond ? "  OK  " : "  FAIL  ") << msg << "\n";
    return cond;
}
} // namespace

int main()
{
    int fails = 0;
    MasterDsp eng;
    const int n = (int) kSr;
    const int measureAt = n / 2;
    const int measureN = n / 4;

    std::cout << "K3CH Master DSP smoke\n";

    const auto* streaming = masterPresetAt (findMasterPresetIndex ("streaming_14"));
    const auto* club = masterPresetAt (findMasterPresetIndex ("club_loud"));
    const auto* trap = masterPresetAt (findMasterPresetIndex ("trap_k3ch"));
    const auto* neutral = masterPresetAt (findMasterPresetIndex ("bypass_neutral"));
    const auto* glue = masterPresetAt (findMasterPresetIndex ("transparent_glue"));
    const auto* radio = masterPresetAt (findMasterPresetIndex ("radio"));

    if (! expect (streaming && club && trap && neutral && glue && radio
                  && allMasterPresets().size() >= 6,
                  "six named master presets exist with distinct settings"))
        ++fails;

    if (streaming && club && neutral)
    {
        if (! expect (club->settings.inputDb > streaming->settings.inputDb + 2.0f
                      && club->settings.limCeilingDb > streaming->settings.limCeilingDb
                      && std::abs (neutral->settings.inputDb) < 0.01f
                      && ! neutral->settings.limOn && ! neutral->settings.satOn,
                      "presets set real chain parameters (not just names)"))
            ++fails;
    }

    if (neutral && club)
    {
        auto hotN = renderSine (eng, neutral->settings, 200.0f, 0.95f, 0.95f, n);
        auto hotC = renderSine (eng, club->settings, 200.0f, 0.95f, 0.95f, n);
        const float pN = peakOf (hotN, measureAt, measureN);
        const float pC = peakOf (hotC, measureAt, measureN);
        std::cout << "  200 Hz peak neutral=" << pN << "  club=" << pC << "\n";
        if (! expect (pC < 0.85f && pC < pN * 0.95f,
                      "Club/Loud limiter+clip pulls hot peaks below Neutral"))
            ++fails;
    }

    if (trap && streaming)
    {
        auto lowT = renderSine (eng, trap->settings, 70.0f, 0.25f, 0.25f, n);
        auto lowS = renderSine (eng, streaming->settings, 70.0f, 0.25f, 0.25f, n);
        const float rT = rmsOf (lowT, measureAt, measureN);
        const float rS = rmsOf (lowS, measureAt, measureN);
        std::cout << "  70 Hz RMS trap=" << rT << "  streaming=" << rS << "\n";
        if (! expect (rT > rS * 1.08f, "Trap Master low shelf louder at 70 Hz than Streaming"))
            ++fails;
    }

    if (neutral)
    {
        MasterSettings wide = neutral->settings;
        wide.width = 1.6f;
        MasterSettings mono = neutral->settings;
        mono.width = 0.2f;
        auto w = renderSine (eng, wide, 1000.0f, 0.35f, -0.35f, n);
        auto m = renderSine (eng, mono, 1000.0f, 0.35f, -0.35f, n);
        const float rW = rmsOf (w, measureAt, measureN);
        const float rM = rmsOf (m, measureAt, measureN);
        std::cout << "  side 1 kHz RMS wide=" << rW << "  narrow=" << rM << "\n";
        if (! expect (rW > rM * 1.5f, "stereo width increases side-only energy"))
            ++fails;

        MasterSettings bassMono = neutral->settings;
        bassMono.width = 1.6f;
        bassMono.bassMonoHz = 180.0f;
        auto sideBass = renderSine (eng, bassMono, 50.0f, 0.4f, -0.4f, n);
        const float rB = rmsOf (sideBass, measureAt, measureN);
        std::cout << "  50 Hz side-only after bass-mono RMS=" << rB << "\n";
        if (! expect (rB < 0.08f, "bass-mono collapses 50 Hz side content"))
            ++fails;
    }

    if (fails != 0)
    {
        std::cerr << fails << " smoke check(s) failed\n";
        return 1;
    }
    std::cout << "All smoke checks passed\n";
    return 0;
}
