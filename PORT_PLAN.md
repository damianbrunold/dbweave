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

- [x] Port the overview dialog's Print button (legacy `overview_form.cpp:199 SBPrintClick`): renders a thumbnail fabric preview directly to `QPrinter`. Ported as `OverviewDialog::doPrint` — honours GewebeNormal / Farbeffekt / Simulation colour modes, the weft/warp ratio, the grid overlay toggle, and page margins/header/footer from File > Page setup. Ctrl+P toolbar shortcut.

### Stage 4 — Print satellite dialogs  [near-term]

- [x] **Print part…** (`actPrintRange`, Ctrl+L): ported `legacy/printselection_form` as `PrintRangeDialog` — four `QSpinBox` pairs (warp / weft / shaft / treadle). Shaft / treadle groups auto-disable when the matching View toggle is off. Wired through `PrPrinterPrint::PrintRange`.
- [x] **Print preview** (`actPreview`) was already wired via `QPrintPreviewDialog` in `pagesetupdialog.cpp:FilePrintpreviewClick`; confirmed present.

### Stage 5 — Highlight mode (F12)  [near-term]

- [x] Ported `legacy/highlight.cpp` as `src/ui/highlight.cpp::TDBWFRM::DrawHighlight`. Second painter pass lays a translucent red rectangle on every dependent cell; called from `PatternCanvas::paintEvent` between the selection and cursor passes.
- [x] Source cell is the current **keyboard** cursor position (matches legacy) -- no separate mouse tracking needed. `highlight` flag on TDBWFRM toggles the overlay.
- [x] `actHighlight` QAction is now checkable; F12 / toolbar click flips `highlight` and calls `refresh()`.
- Note: legacy UX was press-and-hold on the toolbar button; port uses a checkable toggle so the user can move the kbd cursor around and see dependencies live. Cleaner for Qt.

### Stage 6 — Stale comment and genuine-stub cleanup  [release blocker]

Stale comments rewritten:
- [x] `einzug.cpp` — note Fixiert is an obsolete-feature stub pending full excision (Stage 6b).
- [x] `cursor.cpp` — simple white outline is intentional.
- [x] `undoredo.cpp` — obsolete port note removed.
- [x] `range.cpp` — "stub methods removed" wording dropped.
- [x] `trittfolge.cpp` — notes that click handlers are wired (via bereiche.cpp and style QActions).
- [x] `filehandling.cpp` / `filesave.cpp` — close-prompt / AskSave confirmed ported.
- [x] `importwif.cpp` — AskSave gate added (prompt before replacing an unsaved document on WIF import) plus the comment rewritten.
- [x] `choosecolordialog.h` / `farbverlaufdialog.cpp` — "QColorDialog stubs" wording dropped.
- [x] `blockmusterdialog.h` / `fixeinzugdialog.h` — keyboard nav confirmed ported.
- [x] `mainwindow.h` "Drawing stubs" — rewritten.
- [x] `aboutdialog.h` — easter egg note removed; techinfo button confirmed wired.
- [x] `setblatteinzug.cpp` — deferred-update note rewritten (refresh() → paintEvent now wired).

Genuine empty bodies:
- [x] `TDBWFRM::SetCursor(int, int)` now forwards to `cursorhandler->SetCursor(kbd_field, i, j, true)` (matches legacy kbdhandling.cpp:36). Restores the expected behaviour on the undoredo restore path.
- [x] `TDBWFRM::UpdateScrollbars()` now forwards to `pattern_canvas->syncScrollbarsFromFrm()`. Restores scrollbar sync after ExtendTritte / ExtendSchaefte.

### Stage 6b — Clarify the Fixiert / Benutzerdefiniert distinction

Initial reading mistook the two features for one. They are separate and both correct as-is:

- **`EzFixiert`** (label: "User defined…" / "Benutzerdefiniert") — the still-valid fix-einzug template workflow driven by FixeinzugDialog. `RcRecalcAll::RecalcEinzugFixiert` at `recalc.cpp:203` is fully ported, and `TDBWFRM::RecalcAll` dispatches through it whenever `EzFixiert` is checked, so the normal editing path handles benutzerdefiniert correctly. Keep the dialog, the `fixeinzug`/`fixsize`/`firstfree` state, the file-format section, the undoredo slot, the red-mismatch rendering in `draw.cpp`, and the menu entry.
- **`EzBelassen`** (legacy label: "Fixed" / "Fi&xiert") — obsolete. Already collapsed into `NormalZ` in `EinzugRearrangeImpl::Rearrange`; file-format style-index slot preserved for interop with older DB-WEAVE versions.

Action taken: deleted the dead `EinzugRearrangeImpl::Fixiert()` stub and its `Rearrange()` branch — `RcRecalcAll::Recalc` already guards the dispatcher with `!EzFixiert`, so the branch was unreachable. Header comments in `einzug.cpp` / `einzugimpl.h` rewritten to document both items.

### Stage 7 — Loom / Steuerung (weaving mode)  [near-term, pre-release]

Already in the tree (built under `DBWEAVE_BUILD_LOOM`):
- `src/loom/serialport.{cpp,h}` — QSerialPort wrapper matching legacy `SerialPort` API.
- `src/loom/serialcontrollers.{cpp,h}` — five real controllers ported (Patronic, Patronic indirect, Designer, SLIPS, AVL Compu-Dobby III) + `makeLoomController` factory.
- `src/loom/loomsettings.h` — PORT / PARITY / BAUDRATE / PORTINIT enums.
- `src/ui/loom.{cpp,h}` — `StWeaveController` base + `DummyController`.
- `src/ui/loomoptionsdialog.{cpp,h}` — loom options (interface / port / delay / slips-bytes).

What exists today for the weaving UI (`src/ui/loomdialog.{cpp,h}`, ~550 lines): a bare-bones textual modal with a klammer spinbox editor, Start / Stop / Step / Reset buttons, and a QPlainTextEdit log. This is *not* a faithful port of `TSTRGFRM`.

**Gap vs. legacy**: the real TSTRGFRM is a ~3600-line secondary window with a graphical weaving canvas: schlagpatrone / trittfolge rendered in patrone / farbeffekt / simulation modes, a vertical scrollbar, nine interactive vertical klammer bars next to the weave area that the user drags to size / move, an animated weave-position marker that scrolls as the loom advances, and a last-position marker for "Goto last". It has its own menu + toolbar + popup menu, keyboard shortcuts for navigation, and the weaving loop that drives the controller `WeaveSchuss` for each schuss.

Sub-plan for a faithful port (each sub-stage builds + runs; sub-stages are session-sized):

- [x] **7a — Foundation**: deleted the old textual `LoomDialog`; added `TSTRGFRM : QDialog` (`src/ui/steuerung.{h,cpp}`) with `SteuerungCanvas : QWidget` (`src/ui/steuerungcanvas.{h,cpp}`). QMenuBar + QToolBar + QStatusBar laid out around a canvas/scrollbar central area, matching the legacy TSTRGFRM structure. All the Start / Stop / Reverse / Options / View / Zoom / Position / Goto klammer / Shafts QActions are created (disabled -- wired in 7b-7f). `TDBWFRM::LoomControlClick` rewritten to open TSTRGFRM modally, pull/push klammer + weave-position state between the two windows (ported from legacy WeaveClick). Ctrl+W / Extras > Weave now opens it; the temporary top-level "Loom" menu has been removed. Both non-loom and loom CMake builds green.
- [x] **7b — Pos/layout**: ported `CalcSizes` / `CalcTritte` / `UpdateScrollbar` / `AutoScroll` / `UpdateStatusbar` / `_ResetCurrentPos` / `ValidateWeavePosition` / `IsValidWeavePosition` / `WeaveKlammerRight` / `WeaveKlammerLeft` / `WeaveRepetitionInc` / `WeaveRepetitionDec` / `GotoKlammer` / `GotoLastPosition` / `UpdateLastPosition` from `steuerung_pos.cpp` + the layout half of `steuerung_form.cpp`. `SteuerungCanvas::resizeEvent` calls `CalcSizes`; new `refresh()` entry point combines it with `UpdateStatusbar` + goto-enable sweep + canvas repaint. Zoom in / normal / out (Ctrl+I / N / U), the nine goto-klammer QActions + toolbar buttons, Goto last position, Loop, ReverseSchaft, and the eight shaft-count buckets are all wired. Vertical scrollbar is connected through the legacy `scrollbarScroll` formula (`scrolly = MAXY2 - maxj - value`, advancing `weave_position` to track). `tr` renamed to `trittCols` to avoid colliding with `QObject::tr`. Goto-klammer enable-state replaces the legacy `IdleHandler` via `refreshGotoActions()`.
- [x] **7c — Drawing**: ported `steuerung_draw.cpp` as `src/ui/steuerung_draw.cpp`. `DrawGrid` + `DrawData` + three gewebe render modes (Normal / Farbeffekt / Simulation) + `DrawKlammern` + `DrawKlammer` / `_DrawKlammer` (vertical stem + top/bottom caps + repetition count) + `DrawSelection` + `_DrawPositionSelected` / `_DrawKlammerSelected` + `DrawLastPos` / `_DrawLastPos` (triangle pointing at last-position row). `SteuerungCanvas::paintEvent` sets `currentPainter` on TSTRGFRM and calls `paintAll`. Cell painting reuses the main-window `PaintCell` helper. The legacy XOR-style draw-then-clear pattern is dropped: Qt's double buffering means a single-shot full redraw on every refresh is cheap and eliminates the stale-pixel hazard. `IsEmptyTrittfolge(j)` queries TDBWFRM instead of the legacy `trittfolge.isempty` member.
- [x] **7d — Mouse**: ported `steuerung_mouse.cpp` as `src/ui/steuerung_mouse.cpp`. `FormMouseDown` handles click-in-schlagpatrone (sets `weave_position`, `schussselected=true`, validates) and click-in-klammer (sets `current_klammer`, `schussselected=false`; starts drag with `DRAG_TOP` / `DRAG_MIDDLE` / `DRAG_BOTTOM` depending on the click's row relative to the klammer's first / last). `FormMouseMove` resizes or shifts the klammer (clamped to `[0, MAXY2-1]`, `last > first`); a just-touched empty klammer gets `repetitions=1` so it becomes usable. `FormMouseUp` ends the drag. `SteuerungCanvas` forwards `mousePressEvent` / `mouseMoveEvent` / `mouseReleaseEvent` into these methods and calls `setFocus()` on press for keyboard handling in 7e.
- [x] **7e — Keyboard + popup**: ported `steuerung_kbd.cpp` as `src/ui/steuerung_kbd.cpp` (Qt collapses FormKeyPress + FormKeyDown into one `keyPressEvent`). Schuss-mode: Up / Down step `weave_position` by 1 (Ctrl = 8), PgUp / PgDown scroll a page and place the cursor at the edge, Left / Right = `WeaveKlammer{Left,Right}` (Ctrl = `WeaveRepetition{Dec,Inc}`). Klammer-mode: Left / Right cycle `current_klammer` mod MAXKLAMMERN; Up extends the top edge (Shift shifts up; Ctrl inc first; Shift+Ctrl shifts by 8), Down shrinks / shifts down with the same modifier matrix. Enter toggles `schussselected`; 0..9 set the current klammer's repetitions. Popup menu built once from the existing main-menu QActions (they are attached to both menus) and shown from `SteuerungCanvas::contextMenuEvent`.
- [x] **7f — Weaving loop**: ported `steuerung_weben.cpp` as `src/ui/steuerung_weben.cpp`. `AllocInterface` constructs the controller via the `makeLoomController` factory (when `DBWEAVE_HAVE_LOOM` is defined) or falls back to `StDummyController`; the SLIPS controller gets its bytes-per-schuss knob set from `numberOfShafts` via a downcast. `WeaveStartClick` validates the position / klammer, calls `controller->Initialize(INITDATA)`, flips the Start / Stop / Options toolbar enabled-state, then spins `Weben`. `WeaveStopClick` calls `controller->Abort`. `Weben` loops `GetSchaftDaten → WeaveSchuss → NextTritt / PrevTritt` with `QCoreApplication::processEvents` between picks so the Stop button and canvas repaint stay responsive. `GetSchaftDaten` reproduces the four per-loom bit orderings (arm_patronic / arm_designer / varpapuu / default) plus the `reverse` shaft flip. `AtBegin` detects a completed single pass so Weben exits when `Loop` is unchecked. `actReverse` (weave-direction toggle) is hidden for SLIPS / LIPS where the loom drives direction. Options > Loom opens `LoomOptionsDialog` and rebuilds the controller on change.
- [x] **7g — Satellite dialogs**:
  - `StrgGotoDialog` (`src/ui/strggotodialog.{h,cpp}`): port of legacy `strggoto_form`. Three inputs (schuss spinbox, klammer combobox, repetition combobox); repetition list rebuilds when the klammer changes. Wired to TSTRGFRM's `Position > Set current position` entry (Ctrl+S).
  - `PatronicIndParmsDialog` (`src/ui/patronicindparmsdialog.{h,cpp}`): port of legacy `strgpatronicindparms_form`. Two spinboxes (1..220) for starting position + max picks. Wired into `WeaveStartClick` for `intrf_arm_patronic_indirect`: dialog result feeds `controller->SetSpecialData(pos)` and `maxweave`, with a confirm prompt when `pos+count > 220`.
  - `LoomInfoDialog` (`src/ui/loominfodialog.{h,cpp}`): port of legacy `strginfo_form`. Not a schuss-progress window (my earlier note was wrong) -- it's a read-only driver-info viewer showing manufacturer / information / description per `LOOMINTERFACE`. Legacy shipped the content as RTF blobs; the port uses HTML via `QTextBrowser`. Wired via a new `Info...` button on `LoomOptionsDialog`.
  - `strgoptloom_form` is already covered by `LoomOptionsDialog` (confirmed pre-existing).
- [ ] **7h — Settings persistence**: port `LoadSettings` / `SaveSettings` (QSettings under "Steuerung/*"): interface, port, delay, reverse, schaftcount, zoom, current klammern, last position.
- [ ] **7i — Wrap-up**: remove the temporary top-level "Loom" menu; verify both the non-loom and loom CMake builds pass; update PORT_PLAN.

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
