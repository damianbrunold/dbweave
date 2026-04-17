# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

DB-WEAVE is a textile CAD/CAM system (weaving pattern designer) — http://www.brunoldsoftware.ch. It is a Windows desktop application written in C++ for **Borland C++Builder 5** using the VCL framework. Licensed under GPL v3 or later.

External dependency: **ComPort 3.3** from WinSoft (http://www.winsoft.sk/comport.htm) — used for serial communication with looms.

Much of the code uses **German domain terminology** (the original language of the project and its users). Preserve German names when editing:
- `einzug` — threading / warp draft
- `trittfolge` — treadling
- `aufknuepfung` — tie-up
- `rapport` / `rapportieren` — repeat / make repeating
- `schlagpatrone` — weave cartridge / pattern
- `steuerung` — loom control
- `gewebe` — fabric
- `farbe` / `farbauswahl` / `farbpalette` / `farbverlauf` — color / color selection / palette / gradient
- `blockmuster` — block pattern
- `hilfslinien` — guide lines
- `zentralsymm` — central symmetry
- `felddef` — field definition
- `steigung` — slope / gradient
- `entwurf` — design

## Port status (Qt 6)

An incremental port from Borland C++Builder 5 / VCL to C++ / Qt 6.5 LTS is in progress. See `PORT_PLAN.md` for the phased plan and progress checklist. The repository layout reflects the in-progress port:

```
src/         # new Qt 6 code (executable + sub-libraries by phase)
  domain/    # pure domain logic (Phase 2)
  io/        # file format, import/export (Phase 3)
  ui/        # widgets, main window, dialogs (Phase 4–7)
  print/     # QPrinter output (Phase 8)
  loom/      # QSerialPort loom driver (Phase 11, optional)
  compat/    # VCL → Qt shim, deleted module by module in Phase 12
tests/       # Qt Test unit tests
ui/          # .ui files from Qt Designer
cmake/       # helper modules
legacy/      # untouched original C++Builder source — reference only
samples/     # .dbw sample files (used by file-I/O round-trip tests)
```

**Only `src/`, `tests/`, and `ui/` are built.** `legacy/` is frozen and must not be edited — it exists for diffing and reference during the port.

## Build / run (Qt 6)

- Requires **Qt 6.5 LTS or newer**, CMake 3.21+, a C++17 compiler.
- Linux dev-package names (Debian/Ubuntu): `qt6-base-dev`, `qt6-base-dev-tools`, `qt6-tools-dev`, `qt6-tools-dev-tools`, `libqt6serialport6-dev` (only if `-DDBWEAVE_BUILD_LOOM=ON`), plus `cmake` and `ninja-build`.
- Build:
  ```
  cmake -S . -B build -G Ninja
  cmake --build build
  ctest --test-dir build --output-on-failure
  ```
- CMake options: `-DDBWEAVE_BUILD_TESTS=ON/OFF` (default ON), `-DDBWEAVE_BUILD_LOOM=ON/OFF` (default OFF).
- Running: `./build/src/dbweave`.

## Build / run (legacy)

The legacy C++Builder 5 project in `legacy/` is not buildable on Linux or macOS and requires Borland C++Builder 5 + ComPort 3.3 on Windows (tooling is no longer available). Kept for reference only.

## Architecture

### Entry point
`dbw.cpp` — `WinMain` initializes the VCL `Application` and creates the startup forms: `Data` (datamodule), `DBWFRM` (main document window), `FarbPalette`, `ToolpaletteForm`, `STRGFRM` (loom control). The file also lists every unit/form/resource in the project via `USEFORM`/`USEUNIT`/`USERC` macros.

### Form convention
Each UI form is a triple `<name>_form.cpp` + `.h` + `.dfm`. The `.dfm` is the C++Builder form-designer resource (text-format Delphi stream); C++Builder regenerates it, so hand-edits are risky. Non-form units are plain `.cpp` + `.h`.

### Main pattern editor — `TDBWFRM` (`dbw3_form.*`)
The document window's logic is spread across many small concern-focused units rather than a monolithic class body:

- Rendering: `draw.cpp`, `redraw.cpp`, `invalidate.cpp`, `highlight.cpp`
- Viewport: `scrolling.cpp`, `zoom.cpp`, `kbdscroll.cpp`
- Input: `kbdhandling.cpp`, `mousehandling.cpp`
- Editing: `selection.cpp`, `range.cpp`, `move.cpp`, `insert.cpp`, `insertbindung.cpp`, `delete.cpp`, `clear.cpp`, `tools.cpp`
- UI frame: `toolbar.cpp`, `statusbar.cpp`, `popupmenu.cpp`
- Lifecycle: `init.cpp`, `idle.cpp`, `recalc.cpp`
- History: `undoredo.*` — central undo/redo mechanism; editing operations must push entries here

### Weaving domain model
`cursor.*`, `einzug.*` (threading), `trittfolge.cpp` (treadling), `aufknuepfung.cpp` (tie-up), `rapport.*` (repeats), `felddef.*` (field definitions), `palette.*` / `colors.*` / `rangecolors.*` (color), `blockmuster*` (block patterns), `zentralsymm.*` (central symmetry), `hilfslinien.*` (guide lines), `schlagpatrone.cpp`, `steigung.cpp`.

The `*impl.h` files (e.g. `cursorimpl.h`, `einzugimpl.h`, `rapportimpl.h`) hold private implementation details kept out of the public headers.

`set*.cpp` units (`setaufknuepfung.cpp`, `setblatteinzug.cpp`, `setcolors.cpp`, `seteinzug.cpp`, `setfarbe.cpp`, `setgewebe.cpp`, `settrittfolge.cpp`) implement the corresponding "apply" operations.

### Persistence
`fileformat.*`, `fileload.*`, `filesave.*`, `filehandling.cpp`, `mru.cpp` (recent-files list), `import.cpp`, `importbmp.cpp` (+ `importbmp_form`), `export.cpp`, `exportbitmap.cpp`. `loadmap.h` and `loadoptions.h` hold load-configuration structures.

### Printing
`print.*` + `print_base.h`, split by concern into `printinit.cpp`, `printdraw.cpp`, `printextent.cpp`, `printprint.cpp`. Supporting forms: `pagesetup_form`, `printpreview_form`, `printselection_form`, `printcancel_form`.

### Loom control — `TSTRGFRM` (`steuerung_form.*`)
Split analogously to the main editor: `steuerung_draw.cpp`, `steuerung_kbd.cpp`, `steuerung_mouse.cpp`, `steuerung_popup.cpp`, `steuerung_pos.cpp`, `steuerung_weben.cpp`. `loominterface.h` defines the loom abstraction; `comutil.*` wraps ComPort for serial I/O.

### Settings & metadata
`settings.*`, `properties.*`, plus forms `envoptions_form`, `xoptions_form`, `properties_form`, `entwurfsinfo_form`, `techinfo_form`.

### Localization
`language.*` + `lang_main.cpp` + `dbw3_strings.h` — in-house multi-language string table. Add user-visible strings there, not inline.

### Debug helpers
`assert.*`, `debugchecks.cpp` — custom assert macros. Prefer these over raw `assert()`.

## Working on this codebase (during the port)

- Do not edit anything under `legacy/`. Treat it as a read-only reference snapshot.
- New code goes into `src/<sub-module>/`. When adding a source file, add it to the matching `CMakeLists.txt` under `src/` or its sub-module.
- Code style during the port matches the legacy style (tabs, brace-on-same-line); see `.clang-format`. Full Qt-style reformat happens in Phase 12.
- Preserve German domain identifiers (see glossary above).
- Every ported module should ship with at least a minimal Qt Test under `tests/`.
- Docs: `dbw_manual.pdf` (English), `dbw_handbuch.pdf` (German). License: `LICENSE`.
