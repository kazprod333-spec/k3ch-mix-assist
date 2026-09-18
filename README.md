# K3CH plugins

**Maison K3CH Production — Alger**

This repo ships **two** VST3 products (same CMake project, unique plugin codes):

| Product | CMake target | What it does |
|---------|----------------|--------------|
| **K3CH Plugin** | `K3CHMixAssist` | Mix encyclopedia / recipe browser. True **pass-through** audio. Copy recipes and apply them by hand in the DAW. |
| **K3CH Presets** | `K3CHPresets` | Vocal-chain **preset loader** + **internal multi-FX sends** A/B. Real DSP (HPF/EQ → compressor → de-esser → sat, plus reverb/delay/parallel recipes). |

Natural-language commands in **K3CH Presets** (French or English) apply to **this plug-in’s own** processing and send slots. They cannot drive FL Studio’s mixer — host mixer control is not possible via VST3.

Knowledge is **local/embedded** (`Resources/mix-knowledge.json` for the encyclopedia, `Resources/presets-runtime.json` for DSP mappings). No network, no model API.

## K3CH Presets — v1

- **Chaîne (insert FL)** + **Appliquer la chaîne** : choosing ModernRap / Hamza / Weeknd / … applies the full ordered insert path (HPF → EQ bands → compressor → de-ess → sat) plus the chain’s internal Send A/B recipes
- Numbered chain steps show order + key settings (one insert you put on the FL slot you choose)
- **Plan d’inserts FL (manuel)** : checklist for CLA-76, RVox, etc. + **Copier**. A VST3 cannot insert or control third-party plugins on the FL mixer
- Internal **Send A** / **Send B** with **Placer sur** + **Preset FX** + **Placer**
- Full manual controls (tabs): HPF on/slope/Hz, EQ bands (freq/gain/Q), compressor + GR, de-esser, sat on/drive/mix, input/output/dry-wet/return, Send A/B preset + level + HPF/LPF/smash/de-ess/sat/reverb/delay/harsh
- Preset apply fills every control; then you can override anything
- Optional command bar (`charge ModernRap`, `mets Air parallèle sur send A`, …)

Runtime recipes are derived from the studio knowledge pack (ModernRap, Bodak, ALLTY5, JuiceWRLD, TrapDrill, PopUrbaine, BGV, VoixOff, Weeknd, Ninho, Hamza, Freeze, …).

## Download Windows VST3 (no Visual Studio)

GitHub Actions builds Release x64 on this branch. Open **Actions → Windows VST3**, pick the latest green run, download artifact **K3CH-Windows-VST3**, unzip, then copy **both** folders into `C:\Program Files\Common Files\VST3`:

- `K3CH Plugin.vst3`
- `K3CH Presets.vst3`

In FL Studio: **Options → Manage plugins → Find plugins**.

## Requirements (Windows / FL Studio)

| Tool | Notes |
|------|--------|
| Windows 10/11 x64 | FL Studio 20.8+ with VST3 enabled |
| Visual Studio 2022 | Workload **Desktop development with C++** (local builds only) |
| CMake 3.22+ | [cmake.org](https://cmake.org/download/) or `winget install Kitware.CMake` |
| Git | Needed so CMake can fetch JUCE 8.0.15 |

The first configure clones [JUCE](https://github.com/juce-framework/JUCE) (GPL unless you hold a JUCE commercial license). Our plugin sources are MIT; a **distributed binary linked with JUCE** must follow JUCE’s license (GPL **or** a paid JUCE license).

## Build on Windows

In **x64 Native Tools Command Prompt for VS 2022** or PowerShell (from the repo root):

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target K3CHMixAssist_VST3 K3CHPresets_VST3
cmake --build build --config Release --target K3CHMixAssist_Standalone K3CHPresets_Standalone
cmake --build build --config Release --target K3CHPresetsSmoke
```

Artefacts:

```
build\K3CHMixAssist_artefacts\Release\VST3\K3CH Plugin.vst3
build\K3CHMixAssist_artefacts\Release\Standalone\K3CH Plugin.exe
build\K3CHPresets_artefacts\Release\VST3\K3CH Presets.vst3
build\K3CHPresets_artefacts\Release\Standalone\K3CH Presets.exe
```

Optional local JUCE instead of FetchContent:

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64 -DJUCE_PATH=C:\src\JUCE
```

Run each Standalone `.exe` first: encyclopedia should list recipes; Presets should load **ModernRap** by default and change the sound (not pass-through).

## Install in FL Studio (Windows)

1. Copy the whole bundles into:

   `C:\Program Files\Common Files\VST3`

   - `K3CH Plugin.vst3` (encyclopedia)
   - `K3CH Presets.vst3` (vocal + internal sends)

   (Create the folder if it does not exist. This is the standard VST3 location; FL Studio scans it.)

2. Optional extra search path (FL Studio → **Options → File settings → VST plugins extra search folders**) only if you keep the `.vst3` somewhere else.

3. In FL Studio: **Options → Manage plugins** → **Find plugins** (wait for the scan).

4. Verify both **K3CH Plugin** and **K3CH Presets**, vendor **Maison K3CH Production**, type VST3, **enabled**.

5. Insert:
   - **K3CH Plugin** on a mixer insert to **browse/copy recipes**. It will not change the sound.
   - **K3CH Presets** on a vocal insert to **hear** a chain and assign Send A/B recipes inside the plug-in (not FL send slots).

If FL does not see them: confirm you built **x64 Release**, copied the **folder** `*.vst3` (not a lone `.dll`), and that you are not mixing a 32-bit host with a 64-bit plug-in.

## macOS (brief)

Needs Xcode command-line tools and CMake 3.22+.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target K3CHMixAssist_VST3 K3CHMixAssist_Standalone \
                      K3CHPresets_VST3 K3CHPresets_Standalone
```

Universal binary (Intel + Apple Silicon):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
```

Copy both `.vst3` bundles to `~/Library/Audio/Plug-Ins/VST3/` (or `/Library/Audio/Plug-Ins/VST3/` for all users). Rescan plugins in Logic, Ableton, Reaper, etc.

## Linux (developers)

```bash
sudo apt install g++ cmake ninja-build pkg-config libasound2-dev \
  libfreetype-dev libfontconfig1-dev libx11-dev libxcomposite-dev \
  libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \
  libglu1-mesa-dev

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target K3CHMixAssist_Standalone K3CHMixAssist_VST3 \
                      K3CHPresets_Standalone K3CHPresets_VST3 K3CHPresetsSmoke
./build/K3CHPresetsSmoke
```

Standalone:

- `build/K3CHMixAssist_artefacts/Release/Standalone/K3CH Plugin`
- `build/K3CHPresets_artefacts/Release/Standalone/K3CH Presets`

VST3:

- `build/K3CHMixAssist_artefacts/Release/VST3/K3CH Plugin.vst3`
- `build/K3CHPresets_artefacts/Release/VST3/K3CH Presets.vst3`

Copy to `~/.vst3`.

## Knowledge pack

Edit `Resources/mix-knowledge.json` (encyclopedia UI) and/or `Resources/presets-runtime.json` (DSP mappings) and rebuild. Files are compiled into the plugins via JUCE BinaryData.

- `ui_sections` drives the encyclopedia left-hand navigation
- **K3CH Presets** reads `vocal_presets[]` (`params`, `sends`, `applied_chain`, `fl_insert_plan`) and `fx_presets[]` from the runtime JSON (aliases feed the command bar)
- New encyclopedia sections: add a `ui_sections` row (see `NOTES.md`)

## License

Plugin sources: [MIT](LICENSE) © 2026 Maison K3CH Production.

JUCE is separate (GPL / commercial). Do not ship a closed-source VST3 built with GPL JUCE.
