# DB-WEAVE → Qt 6 port plan

Target: cross-platform (Windows/Linux/macOS) native C++/Qt 6 build producing single-file distributable binaries. Existing `@dbw3:file` file format must remain load-compatible.

This document was originally a forward-looking phase plan (Phases 0–12). Most of that work has landed; what remains is captured in the **Remaining work** section at the bottom. The phase history is preserved below as context for commit-message prefixes (`port(phase-N): ...`) and for anyone reviewing the port chronologically.

---

## Ground rules for the port

- **Don't rewrite, port.** Preserve existing algorithms, variable names, German domain terminology, and file structure. Translate VCL calls to Qt calls; do not refactor logic at the same time.
- **One module at a time.** Each commit should build and (where applicable) pass its tests.
- **Keep the legacy tree intact** in `legacy/` for diffing; new code lives in `src/`, tests in `tests/`, UI files in `ui/`.
- **No new features during the port.** Legacy bugs → reproduce, not fix.
- **German identifiers stay German** (`einzug`, `trittfolge`, `aufknuepfung`, …).

---

## Resolved decisions (from Phase 0, 2026-04-17)

1. **Minimum Qt version:** Qt 6.5 LTS.
2. **Build system:** CMake (≥ 3.21).
3. **Test framework:** Qt Test.
4. **Localization:** keep in-house `language.*` string table.
5. **Code style:** legacy style (tabs, brace-on-same-line) via `.clang-format`; Qt-style reformat post-port.
6. **Branching:** port on `master`, legacy code under `legacy/`.

## Resolved decisions (added during implementation)

7. **Splash screen:** not ported — intentionally dropped.
8. **Redraw-all menu (F5):** not ported — every edit triggers a full recalc/redraw, so a manual redraw is unnecessary.
9. **Printer setup menu entry:** dropped for now; Qt's print dialog covers the need via the main Print action.
10. **Help integration:** F1 → `https://www.brunoldsoftware.ch/<help-subpage>` (per-language pages, subpage TBD); "Brunold Software Online" menu → same site root. Both via `QDesktopServices::openUrl`.
11. **About-dialog easter egg:** dropped.
12. **TfGeradeZ / TfGeradeS trittfolge styles:** intentionally dropped (see `filesave.cpp:196` — file format keeps the slots null for compatibility).
13. **Einzug/Trittfolge "fixiert" feature:** dropped as obsolete.
14. **Three color pickers:** kept as three separate dialogs (`ChooseRGBDialog`, `ChooseHSVDialog`, `ChoosePaletteDialog`), matching legacy's three `farbauswahl*` forms.
15. **Silent-print command line:** kept. `dbweave /p <file>` loads the file, prints it without any dialog, and exits. Used for batch printing from shortcuts.

---

## Phase history (summary)

| Phase | Scope | Status |
|---|---|---|
| 0 | Repo reshape + CMake + empty Qt app | ✅ done (commit `aa6f237`) |
| 1 | VCL compatibility shim (`src/compat/`) | ✅ done |
| 2 | Pure domain core (`src/domain/`: colors, fileformat, felddef, zentralsymm, palette, settings, rangecolors) | ✅ done |
| 3 | File I/O round-trip | ✅ done (merged with Phase 5) |
| 4 | Rendering primitives (`draw_cell`, golden-pixel tests) | ✅ done |
| 5 | Main document window (TDBWFRM + every editor op, canvas, menus, MRU, …) | ✅ done |
| 6 | Color + tool palettes as dock widgets | ✅ done (three docks: palette, range, tools) |
| 7 | Modal dialogs | ✅ substantially done (see remaining gaps below) |
| 8 | Printing engine | ✅ engine done; satellite dialogs remaining |
| 9 | Settings / MRU / localization polish | ⏳ partial; per-platform verification outstanding |
| 10 | Cross-platform packaging + CI | ✅ Windows build + packaging working; macOS untested but expected to work; CI outstanding |
| 11 | Loom / Steuerung (serial-port weaving mode) | ❌ not started (near-term, pre-release) |
| 12 | Compat-shim removal | ❌ not started — `src/compat/` still contains 6 headers (`vcl_compat.h`, `tbitmap_compat.h`, `registry_compat.h`, `colors_compat.h`, `assert_compat.h`, `shift_compat.h`); `AnsiString` still used in ~14 `src/` files, `__fastcall` in 2 |

---

## Remaining work

Ordered approximately by user-visible value and by release-gating status. Items marked **[release blocker]** are needed for the first non-loom release; **[near-term]** for the following release.

### Stage 1 — Dead-menu wire-ups and trivial glue  [release blocker]

Menu entries currently `setEnabled(false)`:

- [x] **Cursor locked within rapport** (Ctrl+Q) — toggle wired to `cursorhandler->SetCursorLocked`; enabled-state follows `RappViewRapport` (auto-unchecks when repeat visibility is turned off, matching legacy `dbw3_form.cpp:865`).
- [x] **Help topics** (F1) → `QDesktopServices::openUrl` to `https://www.brunoldsoftware.ch/help` (EN) / `/hilfe` (GE). Subpage path is provisional and should be confirmed once the website adds the help section.
- [x] **Brunold Software Online** → `QDesktopServices::openUrl` to `https://www.brunoldsoftware.ch/`.
- [x] Removed **Redraw** menu entry and F5 shortcut.
- [x] Removed **Printer setup** menu entry.

### Stage 2 — Weft/warp ratio (Schuss/Kett-Verhältnis)  [release blocker]

Port `legacy/einstellverh_form.*` and the `faktor_kette` / `faktor_schuss` state.

- [x] `faktor_kette` / `faktor_schuss` members already on TDBWFRM (float, default 1.0); reset to 1.0 in `ResetDocument` (FileNew / NewFromTemplate).
- [x] Load (already present) + save in `general` section of `fileload.cpp` / `filesave.cpp`.
- [x] Layout in `patterncanvas.cpp:recomputeLayout`: branch on `faktor_schuss` vs `faktor_kette` and stretch the opposite axis (legacy `CalcGrid` formula). Feld*.gw/gh already respect the two locals.
- [x] Print sizing in `printinit.cpp` (already present) and export sizing in `exportbitmap.cpp` (PNG/JPEG/SVG/PDF) via new `applyRatio` helper. Overview dialog inherits from the canvas layout.
- [x] `DrawGewebeSimulation` already has the `faktor_*==1.0` fast path ported from legacy `draw.cpp:501`.
- [x] Dialog ported as `VerhaeltnisDialog` (two `QDoubleSpinBox`, 0.01..10.0, default 1.0). Wired to `actVerhaeltnis`; `setEnabled(false)` removed.
- [x] Round-trip test in `tests/test_fileload.cpp::ratio_round_trips_non_unit_values`.
- [x] Mouse hit-test / selection use per-Feld `gw`/`gh` so non-square cells work unchanged.

### Stage 3 — Overview print  [near-term]

- [ ] Port the overview dialog's Print button (legacy `overview_form.cpp:199 SBPrintClick`): renders a thumbnail fabric preview directly to `QPrinter`. Similar shape to the main print path; share the print-dialog setup.

### Stage 4 — Print satellite dialogs  [near-term]

- [ ] **Print part…** (`actPrintRange`, Ctrl+L): port `legacy/printselection_form` — range pickers for kette/schuss/einzug/trittfolge/aufknuepfung subsets.
- [ ] **Print preview**: wrap the existing print callback in `QPrintPreviewDialog`. Add a menu entry / toolbar button.

### Stage 5 — Highlight mode (F12)  [near-term]

- [ ] Port `legacy/highlight.cpp` as an overlay pass in `draw.cpp` (second painter pass with alpha tint).
- [ ] Add `highlightMode` flag + mouse source-cell tracking in `mousehandling.cpp`.
- [ ] Wire the existing `actHighlight` QAction (F12); remove `setEnabled(false)`.

### Stage 6 — Stale comment and genuine-stub cleanup  [release blocker]

Walk the "stub"/"deferred" comments identified in the 2026-04-21 audit. Almost all are stale; handle in one housekeeping commit.

Stale comments to remove / reword:
- [ ] `einzug.cpp:16–20` — Fixiert is obsolete, not "stubbed pending recalc".
- [ ] `cursor.cpp:670` — simple outline is intentional, drop "deferred" wording.
- [ ] `undoredo.cpp:18–22` — port note is outdated; most referenced methods now have real bodies.
- [ ] `range.cpp:13–15` — TDBWFRM stub methods were removed; rewrite.
- [ ] `trittfolge.cpp:16–17` — Tf*Click handlers are wired (via style QActions + Rearrange); TfGeradeZ/S intentionally dropped.
- [ ] `filehandling.cpp:17` / `filesave.cpp:23,199` — close-prompt is ported; reword.
- [ ] `importwif.cpp:533` — AskSave gate is ported; reword.
- [ ] `choosecolordialog.h:19` / `farbverlaufdialog.cpp:53` — the three real pickers are in use; drop "QColorDialog stubs" language.
- [ ] `blockmusterdialog.h:19` / `fixeinzugdialog.h:16` — keyboard nav is ported; check for missing key combinations vs. legacy then drop the "deferred" wording.
- [ ] `mainwindow.h:422` "Drawing stubs" — rendering is complete; drop or rewrite.
- [ ] `aboutdialog.h:14` — easter egg is dropped by decision; remove the note.

Genuine empty bodies to decide on (keep empty / delete / wire):
- [ ] `TDBWFRM::SetCursor(int, int) {}` (mainwindow.cpp:1805) — the port uses `SetKeyboardPos` directly; likely deletable.
- [ ] `TDBWFRM::UpdateScrollbars() {}` (mainwindow.cpp:1818) — likely absorbed by Qt's automatic scrollbars in `patterncanvas`; likely deletable.
- [ ] `setblatteinzug.cpp:15` deferred `update()` — investigate: does the current code end up repainting correctly without the deferred call?

### Stage 7 — Loom / Steuerung (weaving mode)  [near-term, pre-release]

Entire Phase 11. Biggest remaining slice. Recommended sub-order:

- [ ] `comutil.cpp` protocol helpers (packet framing, acks) on top of the existing `serialport.cpp`.
- [ ] `steuerung_pos.cpp` — pure position math (testable headless).
- [ ] `steuerung.cpp` — non-UI state engine.
- [ ] `steuerung_form.cpp` as a secondary window, with its six split units (`_draw`, `_kbd`, `_mouse`, `_popup`, `_pos`, `_weben`).
- [ ] Satellite dialogs: `strgoptloom_form`, `strgpatronicindparms_form`, `strggoto_form`, `strginfo_form`.
- [ ] Wire the existing `actWeave` menu entry (Ctrl+W); remove `setEnabled(false)`.
- [ ] Gate behind the existing `DBWEAVE_BUILD_LOOM` CMake option.

### Stage 8 — Command-line + packaging + CI

- [ ] Port legacy `/p <file>` silent-print mode via `QCommandLineParser` in `main.cpp`: load, print without dialog, exit (legacy `HandleCommandlinePrint` in `commandline.cpp`).
- [x] Linux build.
- [x] Windows build + packaging.
- [ ] macOS build + `macdeployqt` + (ad-hoc) signed `.dmg` — expected to work but not yet verified on a mac.
- [ ] GitHub Actions workflow building all three platforms on push to `master`.

### Stage 9 — Settings polish (Phase 9 carry-over)

- [ ] Verify `QSettings` persists correctly on Windows (registry), Linux (`.conf`), macOS (plist).
- [ ] Confirm MRU list works end-to-end on each platform.
- [ ] Language files: confirm they load from the right resource path on each platform (embed via `.qrc` if not already).

### Stage 10 — Compat shim removal (Phase 12)

Delete `src/compat/` module by module after everything ships:
- [ ] `AnsiString` → `QString` directly across the tree.
- [ ] `TStringList` wrapper → `QStringList` directly.
- [ ] `TRegistry` wrapper → `QSettings` directly.
- [ ] `TBitmap` wrapper → `QImage` directly.
- [ ] Strip remaining `__fastcall` / `__published` macros.
- [ ] Reformat to Qt style via `.clang-format`.
- [ ] Archive `legacy/` to a separate branch once no `src/` file references it.

---

## Effort summary (remaining)

| Stage | Rough effort |
|---|---|
| 1 Dead-menu wire-ups | 0.5 d |
| 2 Weft/warp ratio | 1–2 d |
| 3 Overview print | 0.5–1 d |
| 4 Print satellite dialogs | 2–3 d |
| 5 Highlight mode | 2–3 d |
| 6 Stub/comment cleanup | 1 d |
| 7 Loom / Steuerung | 2–3 w |
| 8 Command-line + packaging + CI | 1–1.5 w |
| 9 Settings polish | 2–3 d |
| 10 Shim removal | 1 w |
| **Total** | **≈ 6–9 weeks** |

Stages 1–6 alone get to a shippable non-loom release in ≈ 1–2 weeks; Stage 7 gates the pre-release feature-complete milestone.

---

## Per-session workflow for Claude Code

Each session should:
1. Pick the next unchecked item from a Stage above.
2. Work through it — port + test + build green — without spilling into adjacent items.
3. Tick the checkbox and commit with a message pinning the stage: `port(stage-2): add Schuss/Kett-Verhältnis dialog and wiring`.

Stop criteria per session:
- Build stays green.
- New tests pass (if the slice warrants one).
- No cross-module half-ports — finish the unit you started.

---

## Host prerequisites (one-time)

Debian/Ubuntu dev dependencies:

```
sudo apt install cmake ninja-build \
    qt6-base-dev qt6-base-dev-tools \
    qt6-tools-dev qt6-tools-dev-tools
```

For Stage 7 (loom) add: `libqt6serialport6-dev`.
