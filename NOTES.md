# NOTES — later knowledge ingest

v1 ships a **hand-authored JSON** pack (`Resources/mix-knowledge.json`) compiled into the binary. The editor never calls a network API.

## Schema the UI already understands

| Field | Role |
|--------|------|
| `ui_sections[]` | `{ id, title, priority }` — sidebar order. **Add a row here to add a screen.** |
| `presets` | Map of named chains. New keys become cards (vocal + EQ views). |
| `studio`, `drums`, `mastering`, `compression_guide`, `vocal_chain_order` | Bound to known section ids, also reachable generically |
| `section_bindings` (optional) | `{ "section_id": ["path", "nested/path"] }` if `id` ≠ JSON key |
| `docs_pending_ingest[]` | Filenames only; not shown as recipes yet |
| `version`, `source` | Shown in footer / About |

Generic walker rules (so JSON can grow without rewriting UI):

- Keys ending `_hz` / `_khz` / `_db` / `_dbfs` / `_dbtp` / `_lufs` / `_s` / `_minutes` get units
- Two-number arrays render as ranges (`-12–-6 dBFS`)
- Objects with `freq_hz` + `gain_db` + `q` + `type` render as EQ bands
- Objects with `model` + `gr_db` render as compressors
- Nested objects split into extra cards when a node is mostly children

Known section ids with richer layout: `session_checklist`, `vocal_rap`, `eq_guide`, `drums_808`, `gain_staging`, `mastering`. Unknown ids use the walker.

## PDF / spreadsheet ingest (v2+)

Pending sources listed in the pack:

- `THE_kAz_MIX_METHOD_AND_WORKFLOW_V2.pdf`
- `FX_Chains_Recording_FLStudio.pdf`
- `FICHE_CHEATSHEET_MIX_SESSION_K3CH_2026.pdf`
- `BASE_DONNEES_MIX_K3CH_2026.xlsx`
- `Templates_Chaines_Rap_Pop_VoixOff.pdf`
- `Vocal_Chains_Complet_2026.pdf`

Suggested pipeline (keep it **offline**):

1. Extract text (pdftotext / local OCR) or tables (xlsx → CSV).
2. Map each recipe to a `presets.<id>` object using the same field names (`hpf_hz`, `eq[]`, `comp_fet.gr_db`, …).
3. Append `ui_sections` only when you need a **new** category; otherwise dump into `presets` or a new top-level map whose key matches a section `id`.
4. Bump `version` (ISO date). Rebuild so BinaryData updates.
5. Do **not** embed raw PDFs in the VST; keep JSON as the runtime format.

Optional later: load a sidecar `mix-knowledge.json` next to the `.vst3` for live edits without recompiling. v1 is embedded-only on purpose.

## DSP backlog (not v1)

- None required. Keep insert cheap.
- If a “match this recipe” helper is added, drive **parameters of other plugins** via the host, not by cloning Waves/CLA inside this binary.
