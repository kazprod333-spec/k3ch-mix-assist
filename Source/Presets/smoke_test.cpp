#include "DspEngine.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

using k3ch_presets::DspEngine;
using k3ch_presets::MixSettings;
using k3ch_presets::SendSettings;
using k3ch_presets::VocalSettings;

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

std::vector<float> renderSine (DspEngine& eng, const VocalSettings& vocal, float hz, float amp, int samples)
{
    eng.prepare (kSr, kBlock, 2);
    eng.setVocal (vocal);
    MixSettings mix;
    mix.dryWet = 1.0f;
    mix.returnMix = 1.0f;
    mix.outputGain = 1.0f;
    mix.sendGainA = 0.0f;
    mix.sendGainB = 0.0f;
    eng.setMix (mix);
    SendSettings off;
    off.active = false;
    eng.setSend (0, off);
    eng.setSend (1, off);

    std::vector<float> L ((size_t) samples), R ((size_t) samples);
    for (int i = 0; i < samples; ++i)
    {
        const float s = amp * std::sin (2.0f * kPi * hz * (float) i / (float) kSr);
        L[(size_t) i] = s;
        R[(size_t) i] = s;
    }
    for (int i = 0; i < samples; i += kBlock)
    {
        const int n = std::min (kBlock, samples - i);
        float* ch[2] = { L.data() + i, R.data() + i };
        eng.process (ch, 2, n);
    }
    return L;
}

VocalSettings modernRap()
{
    VocalSettings s;
    s.hpfHz = 85;
    s.eqLowHz = 280;
    s.eqLowGainDb = -3;
    s.eqLowQ = 1.2f;
    s.eqMidHz = 3500;
    s.eqMidGainDb = 2;
    s.eqMidQ = 1.4f;
    s.eqAirHz = 11000;
    s.eqAirGainDb = 1.8f;
    s.compThresholdDb = -18;
    s.compRatio = 4;
    s.compAttackMs = 3;
    s.compReleaseMs = 80;
    s.compMakeupDb = 4;
    s.deessHz = 6400;
    s.deessAmount = 0.42f;
    s.satDrive = 0.22f;
    s.satMix = 0.28f;
    return s;
}

VocalSettings clean()
{
    VocalSettings s;
    s.hpfHz = 20;
    s.eqLowGainDb = 0;
    s.eqMidGainDb = 0;
    s.eqAirGainDb = 0;
    s.compRatio = 1.0f;
    s.compThresholdDb = 0;
    s.compMakeupDb = 0;
    s.deessAmount = 0;
    s.satMix = 0;
    return s;
}

VocalSettings hamza()
{
    auto s = clean();
    s.eqAirHz = 14000;
    s.eqAirGainDb = 3.5f;
    return s;
}

VocalSettings freeze()
{
    auto s = clean();
    s.eqAirHz = 10000;
    s.eqAirGainDb = -2.5f;
    return s;
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
    DspEngine eng;

    std::cout << "K3CH Presets DSP smoke\n";

    const int n = (int) kSr; // 1 second
    auto lowClean = renderSine (eng, clean(), 50.0f, 0.3f, n);
    auto lowRap = renderSine (eng, modernRap(), 50.0f, 0.3f, n);
    const float rClean = rmsOf (lowClean, n / 2, n / 4);
    const float rRap = rmsOf (lowRap, n / 2, n / 4);
    std::cout << "  50 Hz RMS clean=" << rClean << "  modernRap=" << rRap << "\n";
    if (! expect (rRap < rClean * 0.55f, "ModernRap HPF 85 attenuates 50 Hz vs clean"))
        ++fails;

    auto airH = renderSine (eng, hamza(), 12000.0f, 0.2f, n);
    auto airF = renderSine (eng, freeze(), 12000.0f, 0.2f, n);
    const float hR = rmsOf (airH, n / 2, n / 4);
    const float fR = rmsOf (airF, n / 2, n / 4);
    std::cout << "  12 kHz RMS hamza=" << hR << "  freeze=" << fR << "\n";
    if (! expect (hR > fR * 1.15f, "Hamza air shelf louder at 12 kHz than Freeze"))
        ++fails;

    // Send A plate vs off — impulse tail energy
    auto renderImpulse = [&] (bool plate) {
        eng.prepare (kSr, kBlock, 2);
        eng.setVocal (clean());
        MixSettings mix;
        mix.dryWet = 1.0f;
        mix.returnMix = 1.0f;
        mix.outputGain = 1.0f;
        mix.sendGainA = plate ? 0.5f : 0.0f;
        mix.sendGainB = 0.0f;
        eng.setMix (mix);
        SendSettings fx;
        fx.active = plate;
        fx.hpfHz = 350;
        fx.lpfHz = 6500;
        fx.reverbMix = 1.0f;
        fx.reverbDecayS = 0.9f;
        fx.reverbDamp = 0.45f;
        eng.setSend (0, fx);
        SendSettings off;
        off.active = false;
        eng.setSend (1, off);

        const int samples = (int) kSr;
        std::vector<float> L ((size_t) samples, 0.0f), R ((size_t) samples, 0.0f);
        L[0] = R[0] = 1.0f;
        for (int i = 0; i < samples; i += kBlock)
        {
            const int nb = std::min (kBlock, samples - i);
            float* ch[2] = { L.data() + i, R.data() + i };
            eng.process (ch, 2, nb);
        }
        return L;
    };

    auto tailOff = renderImpulse (false);
    auto tailOn = renderImpulse (true);
    const float eOff = rmsOf (tailOff, 6000, 12000);
    const float eOn = rmsOf (tailOn, 6000, 12000);
    std::cout << "  impulse tail RMS off=" << eOff << "  plate=" << eOn << "\n";
    if (! expect (eOn > eOff * 4.0f && eOn > 1.0e-5f, "Plate on send A leaves audible tail"))
        ++fails;

    if (fails != 0)
    {
        std::cerr << fails << " smoke check(s) failed\n";
        return 1;
    }
    std::cout << "All smoke checks passed\n";
    return 0;
}
