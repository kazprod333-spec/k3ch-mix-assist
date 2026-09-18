# NOTES — K3CH Plugin knowledge ingest

Runtime knowledge is **`Resources/mix-knowledge.json`**, compiled into the VST via JUCE BinaryData. The editor never calls a network API.

## Current pack (2026-09-17b · batch2)

Merge of batch1 PDFs/xlsx + MIX fiches + artist PDFs + scène FR (see EXTRACT_NOTES).

| Collection | Count |
|------------|------:|
| `presets[]` | **25** (10 MIX/FX + 7 artist/alias + 8 scène FR also listed in `french_scene`) |
| `checklists[]` | 4 |
| `eq_guide[]` | 20 |
| `plugin_stacks[]` | 6 |
| `parallel_buses[]` | 6 |
| `french_scene[]` | 8 |
| `ui_sections[]` | **15** |

New top-level guides: `sidechains`, `harsh_guide`, `compressor_circuits`, `sat_equivalents`, `bass_vs_808`, `backs_guide`, `pense_bete`, `preset_creation_guide`.

`docs_pending_ingest` is empty. Remaining gaps are in JSON `gaps`.

## Schema the UI understands

| Field | Role |
|--------|------|
| `ui_sections[]` | `{ id, title, priority }` — sidebar. **Add a row to add a screen.** |
| `presets[]` | Chain objects. Studio `MIX_*`/`FX_*` → **Chaîne voix rap**; other ids (except scène FR) → **Chaînes artistes** |
| `french_scene[]` | Scène FR recipes (`ninho` … `rap_fr_generique`) |
| `checklists[]` | `{ title, daw, steps[] }` |
| `eq_guide[]`, `eq_recipes_by_style[]`, `hpf_defaults` | EQ |
| `plugin_stacks[]`, `parallel_buses[]`, `sidechains` | Buses / SC |
| `compressor_circuits` | UI id **`compressors`** |
| `harsh_guide` | UI id **`harsh`** |
| `pense_bete`, `preset_creation_guide` | UI id **`pense_bete`** |
| `studio`, `drums`, `eight_oh_eight`, `bass_vs_808`, `mastering`, … | Bound + generic walker |
| `section_bindings` (optional) | `{ "section_id": ["path"] }` |

Ids that do not match a JSON key: `compressors` → `compressor_circuits`, `harsh` → `harsh_guide`. Unknown ids still walk a matching top-level key.

Generic walker: unit suffixes, ranges, `eq_bands`, `{ plugin, setting/gr_db }`, checklist steps, `{ tip }`, `{ symptom, fix }`, `{ si, alors }`, `{ hg2, k3ch }`, `{ logic_model, chez_toi }`, arrays of objects → cards.

## How to ingest more PDFs later

1. Extract offline (`pdftotext -layout`, pandas/openpyxl).
2. Append to `presets[]` / `french_scene[]` / guide objects using the same field names.
3. Add a `ui_sections` row only for a **new** category (`id` = JSON key, or alias in MixKnowledge).
4. Bump `version`. Rebuild so BinaryData updates.

## DSP backlog

**K3CH Plugin** (encyclopedia) stays a cheap pass-through insert. Recipes are copied and applied by hand in FL / Studio One.

**K3CH Presets** (separate VST3, plugin code `K3pr`) is the DSP product: vocal insert chain + internal sends A/B. Runtime map: `Resources/presets-runtime.json` (`applied_chain`, `sends`, `fl_insert_plan`). Selecting a vocal preset applies the full ordered DSP path inside this plug-in. The FL insert plan is a manual checklist — a VST3 cannot insert/control third-party plugins on the host mixer.
