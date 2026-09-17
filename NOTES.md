# NOTES — later knowledge ingest

Runtime knowledge is **`Resources/mix-knowledge.json`**, compiled into the VST via JUCE BinaryData. The editor never calls a network API.

## Current pack (2026-09-17 enrichi)

Pulled from 12 PDFs + `BASE_DONNEES_MIX_K3CH_2026.xlsx` (see `EXTRACT_NOTES`). Counts:

| Collection | Count |
|------------|------:|
| `presets[]` | 10 (ModernRap, Bodak, ALLTY5, Juice, BGV, Trap/Drill, Pop urbaine, Voix-off, PARA_AIR, PARA_BODY) |
| `checklists[]` | 4 (25 steps) |
| `eq_guide[]` | 20 |
| `plugin_stacks[]` | 5 |
| `parallel_buses[]` | 6 |

`docs_pending_ingest` is empty after this ingest. Remaining gaps (artist PDF fiches, PARA/HARSH standalone PDFs) are listed in JSON `gaps`.

## Schema the UI understands

| Field | Role |
|--------|------|
| `ui_sections[]` | `{ id, title, priority }` — sidebar. **Add a row to add a screen.** |
| `presets[]` | Array of chain objects (`id`, `name`, `eq_bands`, `comp`, `chain_steps`, …). Also still accepts the old `{ "id": {…} }` map. |
| `checklists[]` | `{ title, daw, steps: [{ order, title, detail }] }` |
| `eq_guide[]` | `{ problem, freq_hz, action, notes }` |
| `eq_recipes_by_style[]`, `hpf_defaults` | Style starting points |
| `plugin_stacks[]`, `parallel_buses[]` | Insert order / para buses |
| `studio`, `drums`, `eight_oh_eight`, `mastering`, `compression_guide`, `vocal_chain_order`, `pipeline_rules`, `autotune_guide`, `sends_guide` | Bound to section ids + generic walker |
| `section_bindings` (optional) | `{ "section_id": ["path"] }` if `id` ≠ JSON key |
| `version`, `source` | Footer / About |

Generic walker:

- `_hz` / `_khz` / `_db` / `_dbfs` / `_dbtp` / `_lufs` / `_s` / `_ms` / `_pct` / `_minutes` → units
- Two-number arrays → ranges (`-12–-6 dBFS`)
- `eq_bands[]` / `{ freq_hz, gain_db, q, type }` → EQ rows (freq may be a range)
- `{ plugin, gr_db \| setting, notes }` → insert steps
- Checklist `{ order, title, detail }`
- Arrays of objects → **one card each**
- Split into extra cards only when a node is *only* nested groups (no leftover scalars)

Known section ids: `session_checklist`, `vocal_rap`, `eq_guide`, `drums_808`, `gain_staging`, `mastering`, `fl_recording`, `parallel_buses`, `plugin_stacks`. Unknown ids still walk a matching top-level key.

## How to ingest more PDFs later

1. Extract offline (`pdftotext -layout`, pandas/openpyxl).
2. Append objects to `presets[]` / `checklists[]` / `eq_guide[]` using the same field names.
3. Add a `ui_sections` row only for a **new** category whose `id` matches a top-level key (or `section_bindings`).
4. Bump `version`. Rebuild so BinaryData updates.
5. Do **not** embed raw PDFs in the VST.

Optional later: sidecar `mix-knowledge.json` next to the `.vst3`. v1 remains embedded-only.

## DSP backlog (not v1)

- Keep the insert cheap (pass-through).
- Do not clone Waves/CLA inside this binary; recipes are applied by hand in FL / Studio One.
