# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

DB-WEAVE is a textile CAD/CAM system (weaving pattern designer) — http://www.brunoldsoftware.ch. Cross-platform desktop application written in C++17 against Qt 6.5 LTS. Licensed under GPL v3 or later.

Optional dependency: **Qt6SerialPort** — used for serial communication with looms. Disabled with `-DDBWEAVE_NO_LOOM=ON`.

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

## Repository layout

```
src/
  domain/    # pure domain logic
  io/        # file format, import/export
  ui/        # widgets, main window, dialogs
  print/     # QPrinter output
  loom/      # QSerialPort loom driver (optional)
  compat/    # small residual helpers
tests/       # Qt Test unit tests
ui/          # .ui files from Qt Designer
cmake/       # helper modules
samples/     # .dbw sample files (used by file-I/O round-trip tests)
packaging/   # per-platform bundle scripts
resources/   # icons, desktop file, Info.plist template, etc.
```

The original Borland C++Builder 5 / VCL source from which this project was ported is preserved on the `legacy` branch (and `origin/legacy`). Check it out when you need to diff behavior against the historical implementation. It is not buildable on modern systems.

## Build / run

- Requires **Qt 6.5 LTS or newer**, CMake 3.21+, a C++17 compiler.
- Linux dev-package names (Debian/Ubuntu): `qt6-base-dev`, `qt6-base-dev-tools`, `qt6-tools-dev`, `qt6-tools-dev-tools`, `libqt6serialport6-dev`, plus `cmake` and `ninja-build`. Drop `libqt6serialport6-dev` if you build with `-DDBWEAVE_NO_LOOM=ON`.
- Build:
  ```
  cmake -S . -B build -G Ninja
  cmake --build build
  ctest --test-dir build --output-on-failure
  ```
- CMake options: `-DDBWEAVE_BUILD_TESTS=ON/OFF` (default ON), `-DDBWEAVE_NO_LOOM=ON/OFF` (default OFF; set ON to skip loom/serial-port support).
- Running: `./build/src/dbweave`.

## Architecture

### Entry point
`src/main.cpp` bootstraps `QApplication` and the main document window.

### Main pattern editor — `MainWindow` (`src/ui/mainwindow.*`)
The document window's logic is spread across many small concern-focused units rather than a monolithic class body:

- Rendering: `draw.cpp`, `redraw.cpp`, `invalidate.cpp`, `highlight.cpp`, `patterncanvas.cpp`, `draw_cell.cpp`
- Viewport: `scrolling.cpp`, `zoom.cpp`, `kbdscroll.cpp`
- Input: `kbdhandling.cpp`, `mousehandling.cpp`
- Editing: `selection.cpp`, `range.cpp`, `move.cpp`, `insert.cpp`, `insertbindung.cpp`, `delete.cpp`, `clear.cpp`, `tools.cpp`, `edit.cpp`
- UI frame: `toolbar.cpp`, `statusbar.cpp`, `popupmenu.cpp`
- Lifecycle: `init.cpp`, `idle.cpp`, `recalc.cpp`
- History: `undoredo.*` — central undo/redo mechanism; editing operations must push entries here

### Weaving domain model
`cursor.*`, `einzug.*` (threading), `trittfolge.cpp` (treadling), `aufknuepfung.cpp` (tie-up), `rapport.*` (repeats), `felddef.*` (field definitions), `palette.*` / `colors.*` / `rangecolors.*` (color), `blockmuster*` (block patterns), `zentralsymm.*` (central symmetry), `hilfslinien.*` (guide lines), `schlagpatrone.cpp`, `steigung.cpp`.

The `*impl.h` files (e.g. `cursorimpl.h`, `einzugimpl.h`, `rapportimpl.h`) hold private implementation details kept out of the public headers.

`set*.cpp` units (`setaufknuepfung.cpp`, `setblatteinzug.cpp`, `setcolors.cpp`, `seteinzug.cpp`, `setfarbe.cpp`, `setgewebe.cpp`, `settrittfolge.cpp`) implement the corresponding "apply" operations.

### Persistence
`fileformat.*`, `fileload.*`, `filesave.*`, `filehandling.cpp`, `mru.cpp` (recent-files list), `import.cpp`, `importbmp.cpp` (+ `importbmpdialog`), `export.cpp`, `exportbitmap.cpp`, `importwif.cpp`, `exportwif.cpp`. `loadmap.h` and `loadoptions.h` hold load-configuration structures.

### Printing
`src/print/` + `src/ui/print*.cpp` split by concern into `printinit.cpp`, `printdraw.cpp`, `printextent.cpp`, `printprint.cpp`. Supporting dialogs: `pagesetupdialog`, `printpreviewdialog`, `printrangedialog`.

### Loom control — `Steuerung` (`src/ui/steuerung*.*`)
Split analogously to the main editor: `steuerung_draw.cpp`, `steuerung_kbd.cpp`, `steuerung_mouse.cpp`, `steuerung_popup.cpp`, `steuerung_pos.cpp`, `steuerung_weben.cpp`, `steuerungcanvas.cpp`. `src/loom/` contains the `QSerialPort`-based loom driver (built only when `DBWEAVE_NO_LOOM=OFF`).

### Settings & metadata
`settings.*`, `properties.*`, plus dialogs `envoptionsdialog`, `xoptionsdialog`, `propertiesdialog`, `entwurfsinfodialog`, `techinfodialog`.

### Localization
`language.*` + `dbw3_strings.h` — in-house multi-language string table. Add user-visible strings there, not inline.

### Debug helpers
`assert.*`, `debugchecks.cpp` — custom assert macros. Prefer these over raw `assert()`.

## Working on this codebase

- New code goes into `src/<sub-module>/`. When adding a source file, add it to the matching `CMakeLists.txt` under `src/` or its sub-module.
- Preserve German domain identifiers (see glossary above).
- Docs: `dbw_manual.pdf` (English), `dbw_handbuch.pdf` (German). License: `LICENSE`.
- Before committing, run clang-format on the changed files.
