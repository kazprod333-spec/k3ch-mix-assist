# K3CH plugins

**Maison K3CH Production — Alger**

This repo ships **three** VST3 products (same CMake project, unique plugin codes):

| Product | CMake target | What it does |
|---------|----------------|--------------|
| **K3CH Plugin** | `K3CHMixAssist` | Mix encyclopedia / recipe browser. True **pass-through** audio. Copy recipes and apply them by hand in the DAW. |
| **K3CH Presets** | `K3CHPresets` | Vocal-chain **preset loader** + **internal multi-FX sends** A/B. Real DSP (HPF/EQ → compressor → de-esser → sat, plus reverb/delay/parallel recipes). |
| **K3CH Master** | `K3CHMaster` | Place on the **Studio Pro** Master bus. **Mode Master** = real mastering DSP + one-click presets. **Mode Inserts / Mix** = FX Chain *plans* (checklist + copy). A VST3 cannot load plugins onto other Console channels. |

Host mixer control is not possible via VST3. **K3CH Master** is written for **Studio Pro 8** (Fender; formerly Studio One): use native **FX Chains** and **Macro Organizer** to recall insert stacks on other channels. See [`StudioOne/README.md`](StudioOne/README.md).

Knowledge is **local/embedded** (`Resources/mix-knowledge.json`, `Resources/presets-runtime.json`, `Resources/master-inserts.json`). No network, no model API.

## K3CH Master — v1 (Studio Pro 8)

Pose **K3CH Master** sur le canal **Master** (Console).

- **Mode Inserts / Mix** (défaut) : audio **pass-through**. Bibliothèque de plans (voix lead, doubles, backs, bus voix, drums, 808, mélo), numéro de canal 1–125, nom mémo, **Copier le plan**. Le texte rappelle le nom de FX Chain à enregistrer (`K3CH Voix Lead`, etc.). Ce mode **ne charge rien** sur les autres inserts.
- **Mode Master** (fin de mix) : chaîne DSP sur **ce** bus, dans l’ordre : gain d’entrée → EQ (shelf grave / aigu + médium optionnel) → sat / glue → largeur M/S (basse mono) → soft clip → limiteur lookahead → gain de sortie → plafond true-peak (interpolation 2× conservative).
- Presets master (un clic, vrais paramètres) : Streaming -14 LUFS, Club / Loud, Radio / Broadcast, Transparent Glue, Hip-Hop / Trap Master, Bypass / Neutral.

Companion pack (hors VST) : dossiers stub + mode d’emploi FX Chains / macros dans `StudioOne/`.

## K3CH Presets — v1

- **Chaîne (insert)** + **Appliquer la chaîne** : choosing ModernRap / Hamza / Weeknd / … applies the full ordered insert path (HPF → EQ bands → compressor → de-ess → sat) plus the chain’s internal Send A/B recipes
- Numbered chain steps show order + key settings (one insert you put on the channel you choose)
- **Plan d’inserts (manuel)** : checklist for CLA-76, RVox, etc. + **Copier**. A VST3 cannot insert or control third-party plugins on the host mixer
- Internal **Send A** / **Send B** with **Placer sur** + **Preset FX** + **Placer**
- Full manual controls (tabs): HPF on/slope/Hz, EQ bands (freq/gain/Q), compressor + GR, de-esser, sat on/drive/mix, input/output/dry-wet/return, Send A/B preset + level + HPF/LPF/smash/de-ess/sat/reverb/delay/harsh
- Preset apply fills every control; then you can override anything
- Optional command bar (`charge ModernRap`, `mets Air parallèle sur send A`, …)

Runtime recipes are derived from the studio knowledge pack (ModernRap, Bodak, ALLTY5, JuiceWRLD, TrapDrill, PopUrbaine, BGV, VoixOff, Weeknd, Ninho, Hamza, Freeze, …).

## Download Windows VST3 (no Visual Studio)

GitHub Actions builds Release x64 on the `cursor/k3ch-mix-assist-vst3-aa90` branch. Open **Actions → Windows VST3**, pick the latest green run, download artifact **K3CH-Windows-VST3**, unzip, then copy the folders into `C:\Program Files\Common Files\VST3`:

- `K3CH Plugin.vst3`
- `K3CH Presets.vst3`
- `K3CH Master.vst3`

In Studio Pro 8: **Studio Pro → Options → Locations** (confirm VST3 path) then rescan, or drag the `.vst3` bundle into the Browser.

## Requirements

| Tool | Notes |
|------|--------|
| Windows 10/11 x64 | **Studio Pro 8** (VST3). Other VST3 hosts also load the plugs. |
| Visual Studio 2022 | Workload **Desktop development with C++** (local builds only) |
| CMake 3.22+ | [cmake.org](https://cmake.org/download/) or `winget install Kitware.CMake` |
| Git | Needed so CMake can fetch JUCE 8.0.15 |

The first configure clones [JUCE](https://github.com/juce-framework/JUCE) (GPL unless you hold a JUCE commercial license). Our plugin sources are MIT; a **distributed binary linked with JUCE** must follow JUCE’s license (GPL **or** a paid JUCE license).

## Build on Windows

In **x64 Native Tools Command Prompt for VS 2022** or PowerShell (from the repo root):

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target K3CHMixAssist_VST3 K3CHPresets_VST3 K3CHMaster_VST3
cmake --build build --config Release --target K3CHMixAssist_Standalone K3CHPresets_Standalone K3CHMaster_Standalone
cmake --build build --config Release --target K3CHPresetsSmoke K3CHMasterSmoke
```

Artefacts:

```
build\K3CHMixAssist_artefacts\Release\VST3\K3CH Plugin.vst3
build\K3CHMixAssist_artefacts\Release\Standalone\K3CH Plugin.exe
build\K3CHPresets_artefacts\Release\VST3\K3CH Presets.vst3
build\K3CHPresets_artefacts\Release\Standalone\K3CH Presets.exe
build\K3CHMaster_artefacts\Release\VST3\K3CH Master.vst3
build\K3CHMaster_artefacts\Release\Standalone\K3CH Master.exe
```

Optional local JUCE instead of FetchContent:

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64 -DJUCE_PATH=C:\src\JUCE
```

Run each Standalone `.exe` first: encyclopedia should list recipes; Presets should load **ModernRap** by default and change the sound; Master defaults to **Inserts / Mix** (pass-through) — switch to **Master** and pick **Club / Loud** to hear the chain.

## Install in Studio Pro 8 (Windows)

1. Copy the whole bundles into `C:\Program Files\Common Files\VST3` (or your VST3 path in **Options → Locations**):

   - `K3CH Plugin.vst3` (encyclopedia)
   - `K3CH Presets.vst3` (vocal + internal sends)
   - `K3CH Master.vst3` (master bus)

2. In Studio Pro: rescan plug-ins / restart. Vendor **Maison K3CH Production**, type VST3.

3. Insert:

   - **K3CH Plugin** on any channel to **browse/copy recipes**. It will not change the sound.
   - **K3CH Presets** on a vocal (or other) channel to **hear** a chain and assign Send A/B recipes *inside* the plug-in.
   - **K3CH Master** on the **Master** Console channel. Mix with **Inserts / Mix** (thru + plans). At the end, switch to **Master** and pick a preset.

FX Chains for other tracks are built **once** in Studio Pro and recalled via Browser / Macro Organizer — not by this VST. See `StudioOne/README.md`.

If Studio Pro does not see them: confirm you built **x64 Release**, copied the **folder** `*.vst3` (not a lone `.dll`), and that VST3 scanning is enabled.

## macOS (brief)

Needs Xcode command-line tools and CMake 3.22+.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target K3CHMixAssist_VST3 K3CHMixAssist_Standalone \
                      K3CHPresets_VST3 K3CHPresets_Standalone \
                      K3CHMaster_VST3 K3CHMaster_Standalone
```

Universal binary (Intel + Apple Silicon):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
```

Copy `.vst3` bundles to `~/Library/Audio/Plug-Ins/VST3/` (or `/Library/Audio/Plug-Ins/VST3/` for all users). Rescan in Studio Pro.

## Linux (developers)

```bash
sudo apt install g++ cmake ninja-build pkg-config libasound2-dev \
  libfreetype-dev libfontconfig1-dev libx11-dev libxcomposite-dev \
  libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \
  libglu1-mesa-dev

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target K3CHMixAssist_Standalone K3CHMixAssist_VST3 \
                      K3CHPresets_Standalone K3CHPresets_VST3 K3CHPresetsSmoke \
                      K3CHMaster_Standalone K3CHMaster_VST3 K3CHMasterSmoke
./build/K3CHPresetsSmoke
./build/K3CHMasterSmoke
```

Standalone:

- `build/K3CHMixAssist_artefacts/Release/Standalone/K3CH Plugin`
- `build/K3CHPresets_artefacts/Release/Standalone/K3CH Presets`
- `build/K3CHMaster_artefacts/Release/Standalone/K3CH Master`

VST3:

- `build/K3CHMixAssist_artefacts/Release/VST3/K3CH Plugin.vst3`
- `build/K3CHPresets_artefacts/Release/VST3/K3CH Presets.vst3`
- `build/K3CHMaster_artefacts/Release/VST3/K3CH Master.vst3`

Copy to `~/.vst3`.

## Knowledge pack

Edit `Resources/mix-knowledge.json` (encyclopedia UI), `Resources/presets-runtime.json` (Presets DSP mappings), and/or `Resources/master-inserts.json` (Master insert plans) and rebuild. Files are compiled into the plugins via JUCE BinaryData.

- `ui_sections` drives the encyclopedia left-hand navigation
- **K3CH Presets** reads `vocal_presets[]` (`params`, `sends`, `applied_chain`, `fl_insert_plan`) and `fx_presets[]` from the runtime JSON (aliases feed the command bar)
- **K3CH Master** reads `insert_presets[]` (FX Chain name + ordered plugin plan)
- New encyclopedia sections: add a `ui_sections` row (see `NOTES.md`)

## License

Plugin sources: [MIT](LICENSE) © 2026 Maison K3CH Production.

JUCE is separate (GPL / commercial). Do not ship a closed-source VST3 built with GPL JUCE.
