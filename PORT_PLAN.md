# DB-WEAVE → Qt 6 port plan

Target: cross-platform (Windows/Linux/macOS) native C++/Qt 6 build producing single-file distributable binaries. Existing `@dbw3:file` file format must remain load-compatible. Loom/serial-port feature is optional and deferred to the last phase.

Work will be executed primarily by Claude Code in session-sized chunks. Each phase below has a clear deliverable and verification step so a session can stop at any phase boundary without leaving the tree broken.

---

## Ground rules for the port

- **Don't rewrite, port.** Preserve existing algorithms, variable names, German domain terminology, and file structure. Translate VCL calls to Qt calls; do not refactor logic at the same time. Refactors come later, after the port compiles and round-trips files.
- **One module at a time.** Each commit should build and (where applicable) pass its tests.
- **Keep the legacy tree intact** in a `legacy/` subdirectory for reference, so we can diff behaviors side by side. New code lives in `src/`, tests in `tests/`, UI files in `ui/`.
- **No new features during the port.** If something is broken in the legacy code, reproduce the bug, not the fix.
- **German identifiers stay German.** `einzug`, `trittfolge`, `aufknuepfung`, etc. — preserve them in class and file names.

---

## Phase 0 — Repository reshape and tooling (0.5 day)

Goal: make the tree ready for a Qt/CMake project without losing the old code.

Steps:
1. Move all legacy source files into `legacy/` (git mv). Keep `samples/`, docs (`dbw_manual.pdf`, `dbw_handbuch.pdf`), `LICENSE`, `README.txt` at the root.
2. Create top-level directory layout:
   ```
   src/            # new Qt sources
     domain/       # pure domain logic (no Qt UI, only QtCore)
     io/           # file format, import/export
     ui/           # QMainWindow, custom widgets, dialogs
     print/        # QPrinter-based output
     loom/         # QSerialPort loom driver (optional, Phase 11)
     compat/       # VCL→Qt shim headers (short-lived)
   ui/             # .ui files from Qt Designer
   tests/          # Qt Test / Catch2 unit tests
   samples/        # existing sample files, unchanged
   cmake/          # helper modules
   ```
3. Create top-level `CMakeLists.txt` requiring Qt 6.5+ modules: `Core`, `Gui`, `Widgets`, `PrintSupport`, `SerialPort` (optional via `option()`), `Test`.
4. Add `.gitignore` for CMake build dirs, Qt Creator project files, `build/`, `*.user`.
5. Add `.clang-format` matching existing style (tabs + brace-on-same-line) so the port remains visually similar.
6. Verify: `cmake -S . -B build && cmake --build build` runs successfully and produces an empty `dbweave` binary that opens a blank `QMainWindow` titled "DB-WEAVE".

Deliverable: empty Qt app that builds on Linux.

---

## Phase 1 — VCL compatibility shim (1 day)

Goal: a tiny header set that lets legacy `.cpp` files compile under Qt without rewriting every line. This is a **translation scaffold**, not a permanent abstraction — it gets deleted module-by-module as code is modernized.

Files to create under `src/compat/`:
- `vcl_compat.h` — maps the following:
  - `AnsiString` → `QString` (typedef); provide free-function wrappers for `c_str()`, `Length()`, `SubString()`, `Pos()`, `+` concatenation.
  - `__fastcall`, `__published` → empty macros.
  - `True`/`False` → `true`/`false`.
  - `NULL` stays.
  - `TColor` (VCL 0x00BBGGRR) → helper `qColorFromTColor(TColor)` / `tColorFromQColor(QColor)` in `colors_compat.h`.
- `tstringlist_compat.h` — `TStringList` thin wrapper around `QStringList` with `Add`, `Count`, `Strings[]`, `LoadFromFile`, `SaveToFile`.
- `tcanvas_compat.h` — **do not emulate `TCanvas`.** Instead, each draw routine will be ported to take a `QPainter&` directly. The shim intentionally stops here to force the real port of drawing code.
- `tbitmap_compat.h` — `TBitmap` → `QImage` wrapper used only by import/export.
- `registry_compat.h` — tiny wrapper around `QSettings` exposing `ReadString/WriteString/ReadInteger/WriteInteger` mirroring the legacy `TRegistry` calls.

Verify: a smoke test that constructs a shim `TStringList`, adds strings, saves/loads to a temp file.

Deliverable: shim headers + unit tests.

---

## Phase 2 — Domain core port (1–2 weeks)

Goal: every pure-domain unit compiles against Qt only (no VCL, no UI) and has basic unit tests.

Order (strictly bottom-up by dependency, to minimize forward references):

1. `utilities` — generic helpers.
2. `colors`, `rangecolors`, `palette` — color model.
3. `felddef` — field definition (core data structure).
4. `cursor` (+ `cursorimpl.h`) — editing cursor.
5. `einzug` (+ `einzugimpl.h`) — threading.
6. `trittfolge` — treadling.
7. `aufknuepfung` — tie-up.
8. `rapport` (+ `rapportimpl.h`), `rapportieren` — repeats.
9. `blockmuster`, `blockmuster_muster` — block patterns.
10. `zentralsymm` — central symmetry.
11. `hilfslinien` — guide lines.
12. `schlagpatrone`, `steigung` — cartridge/gradient.
13. `bereiche`, `range` — ranges/regions (note: `range.cpp` is editor-level, may defer to Phase 5).
14. `undoredo` — keep interface identical; replace VCL containers with `QList`/`std::vector`.
15. `settings` — swap `TRegistry` for `QSettings`.
16. `language`, `lang_main`, `dbw3_strings.h` — keep the in-house string table; it's already portable.

For each unit:
- Copy `legacy/<unit>.cpp,.h` → `src/domain/<unit>.cpp,.h`.
- Strip `#include <vcl\vcl.h>`, `#pragma hdrstop`, `__fastcall`, `__published`.
- Replace VCL types with shim or Qt equivalents.
- Write a minimal Qt Test exercising one happy path and one edge case.
- Commit per unit.

Verify at end of phase: `tests/` runs green; `src/domain/` links as a static library with no Qt Widgets/Gui dependency (Core only, plus Gui for `QColor`).

Deliverable: `dbweave-domain` static library + tests.

---

## Phase 3 — File I/O round-trip (3–5 days)

Goal: load and save every sample file in `samples/` byte-identically (or semantically equivalent if whitespace differs).

Steps:
1. Port `fileformat.cpp/.h` → `src/io/fileformat.*`. The `@dbw3:file` format is a simple indented text format; `FfBuffer` becomes a thin `QByteArray` wrapper.
2. Port `fileload.cpp`, `filesave.cpp`, `filehandling.cpp`.
3. Port `loadmap.h`, `loadoptions.h`.
4. Port `mru.cpp` against `QSettings`.
5. Defer `import.cpp`, `importbmp.cpp`, `export.cpp`, `exportbitmap.cpp` until Phase 8 (they touch UI/dialog code).
6. Write a round-trip test: for every file in `samples/`, load → save to temp → re-load → assert domain equality. Use domain-level equality operators (add them as needed).

Verify: all sample files round-trip cleanly.

Deliverable: `dbweave-io` static library + round-trip tests.

---

## Phase 4 — Rendering core (1 week)

Goal: render a weaving pattern (loaded via Phase 3) into a `QPainter` on an off-screen `QImage`, save as PNG, visually compare to a reference image.

Steps:
1. Port `draw.cpp` → `src/ui/draw.cpp`. The legacy code takes a `TCanvas*`; the new signature is `void draw(QPainter& p, const Entwurf& e, const DrawOptions& opts)`. Do not preserve the `TDBWFRM::` method form — make these free functions on a pattern object. This is the one deliberate structural change during the port; everything else stays member-shaped.
2. Port color translation (`TColor` → `QColor`) in the rendering path.
3. Port `highlight.cpp`, `hilfslinien.cpp` (rendering side).
4. Do **not** port `redraw.cpp`, `invalidate.cpp` yet — those are tied to the live widget and come in Phase 5.
5. Write a golden-image test: render a sample file to PNG and compare to a checked-in reference (pixel-exact or with small tolerance).

Verify: golden-image tests pass on Linux.

Deliverable: headless rendering + golden-image CI job.

---

## Phase 5 — Main document window (2–3 weeks — the biggest phase)

Goal: a working interactive document window that can open a file, display it, scroll, zoom, select, and save.

Steps (order matters — each step leaves a usable app):

1. Create `PatternCanvas : QWidget` with `paintEvent` calling the Phase 4 `draw()` function. Hook up scroll bars via `QScrollArea` (or manual `QAbstractScrollArea`).
2. Create `MainWindow : QMainWindow` hosting the canvas. Menu bar and toolbars stubbed.
3. Wire File → Open / Save / Save As / Recent Files to Phase 3 I/O.
4. Port `zoom.cpp`, `scrolling.cpp`, `kbdscroll.cpp` — adapt to Qt scroll events.
5. Port `redraw.cpp`, `invalidate.cpp` — collapse into `update()` / `update(QRect)` calls. This is a simplification; VCL's manual invalidation is unnecessary with Qt.
6. Port `mousehandling.cpp`, `kbdhandling.cpp` — translate `TMouseEvent`/`TKeyEvent` to `QMouseEvent`/`QKeyEvent`.
7. Port `selection.cpp`, `range.cpp` — editing selection + rubber-band.
8. Port `undoredo.cpp` wiring (the logic was already ported in Phase 2; now hook menu actions).
9. Port `tools.cpp` — drawing tools (line, rectangle, ellipse, constrained).
10. Port editing ops: `move`, `insert`, `insertbindung`, `delete`, `clear`, `recalc`, `idle`.
11. Port state operations: `setaufknuepfung`, `setblatteinzug`, `setcolors`, `seteinzug`, `setfarbe`, `setgewebe`, `settrittfolge`.
12. Port `toolbar.cpp`, `statusbar.cpp`, `popupmenu.cpp` as Qt equivalents (`QToolBar`, `QStatusBar`, context menus).
13. Port `init.cpp` (startup) — becomes `MainWindow` constructor + app bootstrap in `main.cpp`.

Verify at end of phase: user can open any sample file, see it rendered correctly, make an edit, undo/redo, save. This is the minimum viable port.

Deliverable: working editor (no dialogs yet — they use the existing `QInputDialog`/`QColorDialog` stand-ins or are stubbed with TODOs).

---

## Phase 6 — Color palette and tool palette (3–5 days)

These are floating "palette" windows used constantly during editing, so port them before the rest of the dialogs.

Steps:
1. `FarbPalette` (`farbpalette_form`) → `PalettePanel` as a `QDockWidget` in the main window.
2. `ToolpaletteForm` (`toolpalette_form`) → `ToolPanel` as a `QDockWidget`.
3. Port `colors.cpp` UI wiring and the three color-picker forms (`farbauswahl_form`, `farbauswahl1_form`, `farbauswahl2_form`). Consider replacing the HSV picker with `QColorDialog` — decision point: keep custom picker for fidelity, or adopt Qt's for speed? Default: keep custom picker.

Deliverable: palette/tool panels dockable in main window.

---

## Phase 7 — Dialogs batch port (2 weeks)

Goal: every modal dialog working via `.ui` files.

Group dialogs and port in batches of ~5 per sitting. For each dialog:
- Copy legacy `.dfm` into a text file for reference (it's human-readable).
- Recreate the layout in Qt Designer → `ui/<name>.ui`.
- Port the `_form.cpp` logic into a `QDialog` subclass using `Ui::<Name>` generated class.
- Wire signals via `connect(...)`.

Recommended batches:
1. **Info dialogs**: `about`, `splash_form`, `techinfo_form`, `entwurfsinfo_form`, `strginfo_form`.
2. **Navigation/goto**: `cursordir_form`, `cursordirection_form`, `cursorgoto_form`, `strggoto_form`, `overview_form`.
3. **Properties/options**: `properties_form`, `envoptions_form`, `xoptions_form`, `einstellverh_form`, `entervv_form`.
4. **Editing assistants**: `blockmuster_form`, `einzugassistent_form`, `fixeinzug_form`, `rapport_form`, `farbverlauf_form`.
5. **Color selection**: `selcolor_form` (plus the three color-picker forms already done in Phase 6).
6. **User-defined patterns**: `userdef_entername_form`, `userdefselect_form`, `userdef.cpp`.
7. **Loom control shell**: `strgoptloom_form`, `strgpatronicindparms_form` (UI only; serial I/O comes in Phase 11).
8. **Import/export**: `importbmp_form`, `loadparts_form`; also port `import.cpp`, `importbmp.cpp`, `export.cpp`, `exportbitmap.cpp`.

Verify: each dialog opens, accepts input, applies its effect identically to the legacy version. Manual QA against the legacy binary isn't possible (no build env), so cross-check against the PDF manual and the `.dfm` source for expected behavior.

Deliverable: feature-complete UI minus printing and loom.

---

## Phase 8 — Printing (1–1.5 weeks)

Steps:
1. Port `print.*`, `print_base.h`, `printinit.cpp`, `printdraw.cpp`, `printextent.cpp`, `printprint.cpp` against `QPrinter` + `QPainter`. The legacy code is structurally compatible: it draws to a canvas; `QPainter` works on both screen and printer.
2. Port `pagesetup_form` → use `QPageSetupDialog` if possible, else custom.
3. Port `printpreview_form` → `QPrintPreviewDialog`.
4. Port `printselection_form`, `printcancel_form`.
5. Verify print output matches the legacy binary via PDF comparison (print to PDF on both platforms, diff pages).

Deliverable: print + preview + page setup.

---

## Phase 9 — Settings, MRU, localization polish (2–3 days)

- Verify `QSettings`-backed settings persist across sessions on all three platforms (Windows registry, Linux `.conf`, macOS plist).
- Verify MRU list works.
- Port the language files: keep `language.*` as-is internally; ensure files load from the right resource path on each platform (embed via Qt Resources `.qrc`).

Deliverable: settings persist, language switching works.

---

## Phase 10 — Cross-platform packaging (1 week)

Steps:
1. Linux: build an AppImage via `linuxdeploy` + `linuxdeploy-plugin-qt`. Test on Ubuntu LTS + Debian stable.
2. Windows: cross-build with MSVC or MinGW, package with `windeployqt` + `NSIS` (reuse the existing `legacy/dbw.nsi` script, adjusted file names).
3. macOS: build on a mac (or GitHub Actions macOS runner), package with `macdeployqt` and create a signed `.dmg` if a developer cert is available; otherwise ad-hoc signing.
4. Add a GitHub Actions workflow building all three on every push to `master`.

Deliverable: three signed (or ad-hoc signed) installers per release.

---

## Phase 11 — Loom/serial port (optional, 1 week)

Only start once everything else ships.

Steps:
1. Port `comutil`, `combase.h`, `loominterface.h` against `QSerialPort`. The `SerialPort` class is small (open/close/send/receive/purge) — a 1:1 map.
2. Port `steuerung_form.*` as a `QMainWindow`-like secondary window, with its six split units (`steuerung_draw`, `_kbd`, `_mouse`, `_popup`, `_pos`, `_weben`).
3. Gate the feature behind the Qt6 `SerialPort` optional CMake dependency. If the user builds without it, the loom menu is hidden.

Deliverable: loom-control feature restored.

---

## Phase 12 — Cleanup and shim removal (1 week)

Delete the `src/compat/` shims module by module and replace the remaining references with idiomatic Qt:
- `AnsiString` → `QString` directly everywhere.
- Custom `TStringList` wrapper → `QStringList` directly.
- Remove any straggling `__fastcall`/`__published` macros.

After this, the `legacy/` folder can be moved out of the primary repo (archived on a branch or separate repo).

---

## Effort summary

| Phase | Rough effort |
|---|---|
| 0 Bootstrap | 0.5 d |
| 1 Compat shim | 1 d |
| 2 Domain core | 1–2 w |
| 3 File I/O | 3–5 d |
| 4 Rendering core | 1 w |
| 5 Main window | 2–3 w |
| 6 Palettes | 3–5 d |
| 7 Dialogs | 2 w |
| 8 Printing | 1–1.5 w |
| 9 Settings polish | 2–3 d |
| 10 Packaging | 1 w |
| 11 Loom (optional) | 1 w |
| 12 Cleanup | 1 w |
| **Total** | **≈ 3–5 months** of Claude-driven work with your review |

---

## Per-session workflow for Claude Code

Each session should:
1. Read this plan, identify the current phase and next unstarted step.
2. Work through one or two steps — a step = one module ported + its test written + build green.
3. Update the checklist at the bottom of this file (add it as checkboxes once Phase 0 starts).
4. Commit with a message pinning the phase: `port(phase-2): migrate einzug.cpp to Qt`.

Stop criteria per session:
- Build stays green.
- New tests pass.
- No cross-module half-ports (finish the unit you started).

---

## Resolved decisions

Settled at the start of Phase 0 on 2026-04-17:

1. **Minimum Qt version:** Qt 6.5 LTS.
2. **Build system:** CMake (≥ 3.21).
3. **Test framework:** Qt Test.
4. **Localization:** keep in-house `language.*` string table; revisit in Phase 12.
5. **Code style:** legacy style (tabs, brace-on-same-line) via `.clang-format`; Qt-style reformat in Phase 12.
6. **Branching:** port on `master`, with legacy code relocated under `legacy/`.

---

## Progress checklist

### Phase 0 — Repository reshape and tooling
- [x] All 316 legacy source/resource files moved to `legacy/`.
- [x] New directory tree created: `src/{domain,io,ui,print,loom,compat}`, `ui/`, `tests/`, `cmake/`.
- [x] Top-level `CMakeLists.txt` with Qt 6.5 LTS requirement and `DBWEAVE_BUILD_TESTS` / `DBWEAVE_BUILD_LOOM` options.
- [x] `src/CMakeLists.txt` building a placeholder `dbweave` executable.
- [x] `src/main.cpp` opens an empty `QMainWindow` titled "DB-WEAVE".
- [x] `tests/CMakeLists.txt` + `tests/test_smoke.cpp` (Qt Test).
- [x] `.gitignore` and `.clang-format` in place.
- [x] `CLAUDE.md` updated to document the new layout and Qt build commands.
- [x] **Build verification** — configure + compile + link + ctest all green on Debian 13 / Qt 6.8.2 / CMake 3.31 / Ninja.
- [x] First commit (`aa6f237`).

### Phase 1 — VCL compatibility shim
- [x] `src/compat/CMakeLists.txt` (INTERFACE library `dbweave_compat`).
- [x] `src/compat/vcl_compat.h` — `AnsiString`/`String` → `QString`, `__fastcall`/`__published`/`True`/`False` macros.
- [x] `src/compat/colors_compat.h` — `TColor` ↔ `QColor` with VCL's BBGGRR byte order preserved.
- [x] `src/compat/tstringlist_compat.h` — `TStringList` wrapper over `QStringList` with `Add`/`Count`/`Clear`/`[]`/`Strings[]`/`LoadFromFile`/`SaveToFile`.
- [x] `src/compat/tbitmap_compat.h` — `TBitmap` wrapper over `QImage` (sizes + file I/O; pixel drawing deliberately not shimmed).
- [x] `src/compat/registry_compat.h` — `TRegistry` wrapper over `QSettings` with `OpenKey`/`CloseKey`/`ValueExists`/`ReadInteger`/`WriteInteger`/`ReadString`/`WriteString`.
- [x] `src/compat/assert_compat.h` — `dbw3_assert` → `Q_ASSERT`; `dbw3_trace` → `qDebug()`.
- [x] `tests/test_compat.cpp` — five test cases across all shims, all green.

### Phase 2 — Domain core port

Revised ordering as of commit bringing in `fileformat`: `utilities.cpp`
is actually a batch of `TDBWFRM::` main-window editor handlers (despite
the name) and has been moved to Phase 5. `fileformat` moved up from
Phase 3 because `felddef`'s `Read`/`Write` methods depend on its
`FfReader`/`FfWriter` classes.

- [x] `colors` (RGB ↔ HSV).
- [x] `fileformat` (`@dbw3:file` text reader/writer; `FfFile` replaced Win32 `HANDLE` with `std::FILE*`; `FieldHexToBinary` lifted out of legacy `fileload.cpp`).
- [x] `loadmap.h` (FIELD_MAP / SECTION_MAP DSL macros used by every `Read()`).
- [x] `felddef` (Feld{Vector,Grid}{Char,Short,Bool} containers, file round-trip via fileformat).
- [x] `zentralsymm` (pure-algo central-symmetry searcher).
- [x] `palette` (236-colour palette with forward-compatible format-3.7 save and legacy-format read; `COLORREF`/`BYTE`/`RGB`/`GetRValue`/`GetGValue`/`GetBValue` added to `compat/colors_compat.h`; `LOGPALETTE` kept byte-exact).
- [x] `settings` (over the `TRegistry` shim → `QSettings`).
- [ ] **Remainder moved to Phase 5.** Triage showed every remaining `legacy/*.cpp` file on the Phase 2 list either (a) contains `TDBWFRM::` or `TBlockmusterForm::` method bodies (`utilities`, `bereiche`, `blockmuster_muster`, `clear`, `init`), or (b) references global singletons `DBWFRM`/`Data` from within its core logic (`cursor`, `einzug`, `trittfolge`, `aufknuepfung`, `rapport`, `rapportieren`, `blockmuster`, `hilfslinien`, `schlagpatrone`, `steigung`, `undoredo`, `language`, `lang_main`). Porting these without the main window requires a dependency-injection refactor that the ground rules disallow during the port. They will be ported in Phase 5 alongside `TDBWFRM`.
- [ ] `dbw3_base.h` foundation types (`PT`, `SZ`, `RANGE`, `RAPPORT`, `FeldBase*`) — the type definitions are pure but the virtual `Clear()`/`ScrollX()`/`ScrollY()` bodies in `bereiche.cpp`/`clear.cpp`/`init.cpp` depend on `Data->` and `DBWFRM->`. Moved to Phase 5.
- [ ] `rangecolors` — depends on `dbw3_base.h` constants plus `TCanvas` for `GetDeviceCaps`. Moved to Phase 4 (rendering) where the Qt `QScreen` replacement naturally belongs.
- [ ] `dbw3_strings.h` — pure string-table header; port early in Phase 5 when `language`/`lang_main` lands.

### Phase 3 — File I/O round-trip

With `fileformat` already ported in Phase 2, this phase becomes smaller:

- [ ] `fileload`, `filesave`, `filehandling`.
- [ ] `loadmap.h`, `loadoptions.h`.
- [ ] `mru`.
- [ ] Round-trip test against every file in `samples/`.

### Phase 4 — Rendering core

Triage on first try showed that `draw.cpp` (580 lines) and every other
Phase-4 candidate is a pile of `TDBWFRM::` methods that reach into the
main window's instance state (scroll offsets, `righttoleft`,
`toptobottom`, `Canvas`, etc.). The only truly standalone rendering
primitives are `PaintCell` / `ClearCell`, which appear as free `inline`
functions in `legacy/dbw3_form.h`. They are the leaves every higher-
level Draw* method dispatches to.

- [x] `PaintCell` / `ClearCell` ported as free functions in
  `src/ui/draw_cell.{h,cpp}`. Signature change (the one deliberate
  structural change of Phase 4): takes `QPainter&` + `QColor` instead
  of `TCanvas*` + `TColor`, and the `NUMBER` darstellung accepts the
  font height as an explicit parameter instead of reading
  `DBWFRM->currentzoom`. Anti-aliasing is disabled for deterministic
  output.
- [x] `DARSTELLUNG` enum moved from `dbw3_base.h` into
  `src/domain/enums.h` (it is a pure-value enum used both by the
  domain Feld* types and by the renderer).
- [x] Golden-pixel tests in `tests/test_draw_cell.cpp` cover
  `ClearCell` + all nine geometric darstellung variants +
  `NUMBER` fallback-to-filled-rect (font-rendered path not tested
  because font rasterisation is not portable enough for exact
  pixel checks).
- [ ] Everything else in `draw`, `highlight`, `hilfslinien` remains
  tangled with `TDBWFRM::` state and moves to Phase 5, where it
  will be ported against the reconstructed `MainWindow` member
  variables. A full-pattern golden-image test that renders a loaded
  `samples/*.dbw` file end-to-end will be written once the
  document-model types (FeldGewebe etc.) are available from
  Phase 5.

### Phase 5 — Main document window
- [ ] `PatternCanvas : QWidget`
- [ ] `MainWindow : QMainWindow`
- [ ] File → Open / Save / Save As / Recent Files
- [ ] `zoom`, `scrolling`, `kbdscroll`
- [ ] `redraw`, `invalidate` (collapse into `update()`)
- [ ] `mousehandling`, `kbdhandling`
- [ ] `selection`, `range`
- [ ] `undoredo` menu wiring
- [ ] `tools`
- [ ] `move`, `insert`, `insertbindung`, `delete`, `clear`, `recalc`, `idle`
- [ ] `setaufknuepfung`, `setblatteinzug`, `setcolors`, `seteinzug`, `setfarbe`, `setgewebe`, `settrittfolge`
- [ ] `toolbar`, `statusbar`, `popupmenu`
- [ ] `init` → `MainWindow` ctor

### Phase 6 — Palettes
- [ ] `FarbPalette` → `PalettePanel` (`QDockWidget`)
- [ ] `ToolpaletteForm` → `ToolPanel` (`QDockWidget`)
- [ ] Color-picker forms (`farbauswahl`, `farbauswahl1`, `farbauswahl2`)

### Phase 7 — Dialogs
- [ ] Batch 1: info dialogs
- [ ] Batch 2: navigation/goto
- [ ] Batch 3: properties/options
- [ ] Batch 4: editing assistants
- [ ] Batch 5: color selection
- [ ] Batch 6: user-defined patterns
- [ ] Batch 7: loom-control shell (UI only)
- [ ] Batch 8: import/export

### Phase 8 — Printing
- [ ] `print`, `print_base.h`, `printinit`, `printdraw`, `printextent`, `printprint`
- [ ] `pagesetup_form`, `printpreview_form`, `printselection_form`, `printcancel_form`

### Phase 9 — Settings, MRU, localization polish
- [ ] `QSettings` per-platform verification
- [ ] Language files resource-embedded

### Phase 10 — Packaging
- [ ] Linux: AppImage
- [ ] Windows: `windeployqt` + NSIS
- [ ] macOS: `macdeployqt` + `.dmg`
- [ ] GitHub Actions workflow

### Phase 11 — Loom / serial port (optional)
- [ ] `comutil`, `combase.h`, `loominterface.h` → `QSerialPort`
- [ ] `steuerung_form` and its split units

### Phase 12 — Shim removal
- [ ] Delete `src/compat/` module by module
- [ ] Switch clang-format to Qt style
- [ ] Archive `legacy/`

---

## Host prerequisites (one-time)

The current dev host (Debian 13) has Qt 6.8 runtime libraries but is missing CMake and the Qt 6 dev packages. To verify the Phase 0 build, install:

```
sudo apt install cmake ninja-build \
    qt6-base-dev qt6-base-dev-tools \
    qt6-tools-dev qt6-tools-dev-tools
```

For Phase 11 add: `libqt6serialport6-dev`.
