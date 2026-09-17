# K3CH Mix Assist

**Maison K3CH Production — Alger**

VST3 mix-helper insert for FL Studio (and other VST3 hosts). Engineers drop it on a track, browse studio recipes (vocal chains, EQ starting points, gain staging, kick/808, session checklist, mastering targets), and copy a plain-text recipe to apply manually in the DAW.

Audio is **true pass-through** (in → out) with an optional peak/RMS meter. There is no heavy DSP in v1. Knowledge is **local/embedded** (`Resources/mix-knowledge.json`) — no network, no model API.

## Features

- Navigate `ui_sections` from the knowledge pack (new sections/presets show up without rewriting the UI)
- Concrete numbers: Hz, dB, dB GR, LUFS, dBTP
- Search/filter across titles and values
- **Copier la recette** → clipboard as numbered/plain-text steps
- Bypass + stereo peak/RMS meter
- Standalone app for testing without a DAW

## Requirements (Windows / FL Studio)

| Tool | Notes |
|------|--------|
| Windows 10/11 x64 | FL Studio 20.8+ with VST3 enabled |
| Visual Studio 2022 | Workload **Desktop development with C++** |
| CMake 3.22+ | [cmake.org](https://cmake.org/download/) or `winget install Kitware.CMake` |
| Git | Needed so CMake can fetch JUCE 8.0.15 |

The first configure clones [JUCE](https://github.com/juce-framework/JUCE) (GPL unless you hold a JUCE commercial license). Our plugin sources are MIT; a **distributed binary linked with JUCE** must follow JUCE’s license (GPL **or** a paid JUCE license).

## Build on Windows

In **x64 Native Tools Command Prompt for VS 2022** or PowerShell (from the repo root):

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target K3CHMixAssist_VST3
cmake --build build --config Release --target K3CHMixAssist_Standalone
```

Artefacts:

```
build\K3CHMixAssist_artefacts\Release\VST3\K3CH Mix Assist.vst3
build\K3CHMixAssist_artefacts\Release\Standalone\K3CH Mix Assist.exe
```

Optional local JUCE instead of FetchContent:

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64 -DJUCE_PATH=C:\src\JUCE
```

Run the Standalone `.exe` first to confirm the editor opens and recipes appear.

## Install in FL Studio (Windows)

1. Copy the whole bundle `K3CH Mix Assist.vst3` into:

   `C:\Program Files\Common Files\VST3`

   (Create the folder if it does not exist. This is the standard VST3 location; FL Studio scans it.)

2. Optional extra search path (FL Studio → **Options → File settings → VST plugins extra search folders**) only if you keep the `.vst3` somewhere else.

3. In FL Studio: **Options → Manage plugins** → **Find plugins** (wait for the scan).

4. Verify **K3CH Mix Assist** is in the list, vendor **Maison K3CH Production**, type VST3. Make sure it is **enabled**.

5. Insert it like any effect: mixer insert slot → select **K3CH Mix Assist**. It will not change the sound (pass-through). Use the editor to copy recipes, then set EQ/comp/sends on your usual plugins.

If FL does not see it: confirm you built **x64 Release**, copied the **folder** `K3CH Mix Assist.vst3` (not a lone `.dll`), and that you are not mixing a 32-bit host with a 64-bit plug-in.

## macOS (brief)

Needs Xcode command-line tools and CMake 3.22+.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target K3CHMixAssist_VST3 K3CHMixAssist_Standalone
```

Universal binary (Intel + Apple Silicon):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
```

Copy `K3CH Mix Assist.vst3` to `~/Library/Audio/Plug-Ins/VST3/` (or `/Library/Audio/Plug-Ins/VST3/` for all users). Rescan plugins in Logic, Ableton, Reaper, etc.

## Linux (developers)

```bash
sudo apt install g++ cmake ninja-build pkg-config libasound2-dev \
  libfreetype-dev libfontconfig1-dev libx11-dev libxcomposite-dev \
  libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \
  libglu1-mesa-dev

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target K3CHMixAssist_Standalone K3CHMixAssist_VST3
```

Standalone smoke-test: `build/K3CHMixAssist_artefacts/Release/Standalone/K3CH\ Mix\ Assist`

VST3 bundle: `build/K3CHMixAssist_artefacts/Release/VST3/K3CH Mix Assist.vst3` — copy to `~/.vst3`.

## Knowledge pack

Edit `Resources/mix-knowledge.json` and rebuild. The file is compiled into the plugin via JUCE BinaryData.

- `ui_sections` drives the left-hand navigation (id, title, priority)
- New **presets** under `presets` appear in vocal/EQ views automatically
- New **top-level objects** can be exposed by adding a `ui_sections` entry with a matching `id`
- Optional `section_bindings` map (see `NOTES.md`) if the id does not match a JSON key
- Unit suffixes (`_hz`, `_db`, `gr_db`, `_lufs`, …) are formatted for display without UI changes

## License

Plugin sources: [MIT](LICENSE) © 2026 Maison K3CH Production.

JUCE is separate (GPL / commercial). Do not ship a closed-source VST3 built with GPL JUCE.
