# DB-WEAVE — cross-platform packaging

Three scripts, one per platform. Each uses the regular CMake install
rules declared at the top of `CMakeLists.txt` (bundle target + desktop
entry + icon + LICENSE/PDFs) and then wraps the installed tree into
the native distributable.

By default the scripts build with loom / serial-port support (needs
Qt6SerialPort). Pass `--no-loom` (or `-NoLoom` on Windows) to set the
`DBWEAVE_NO_LOOM` CMake option and produce a build without the
Qt6SerialPort dependency.

Outputs land in `dist/` under the repository root.

## Linux — AppImage

```
packaging/linux/build_appimage.sh            # default, includes loom
packaging/linux/build_appimage.sh --no-loom  # skip loom/serial
```

Dependencies (Debian / Ubuntu):

```
sudo apt install qt6-base-dev qt6-base-dev-tools \
                 qt6-tools-dev qt6-tools-dev-tools \
                 libqt6serialport6-dev \
                 cmake ninja-build
# libqt6serialport6-dev can be omitted when building with --no-loom.
```

Download `linuxdeploy` and its Qt plugin (continuous releases):

```
curl -L https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage         -o linuxdeploy-x86_64.AppImage
curl -L https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage -o linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-*.AppImage
export PATH="$PWD:$PATH"
```

The script produces `dist/DB-WEAVE-<version>-x86_64.AppImage`. Run
anywhere with a glibc newer than the build host (AppImage handles
the Qt dependencies).

## Windows — NSIS installer

Run from a **Qt 6 x64 Developer Command Prompt** (or any shell where
`qmake.exe`, `windeployqt.exe`, `cmake.exe`, and `makensis.exe`
are on `PATH`):

```
powershell -File packaging\windows\build_installer.ps1
powershell -File packaging\windows\build_installer.ps1 -NoLoom
```

Prerequisites:

* Qt 6.5+ MSVC *or* MinGW install (the official installer from
  https://www.qt.io/download-qt-installer works).
* CMake 3.21+ and Ninja (Qt's installer bundles Ninja; otherwise
  `choco install ninja`).
* NSIS 3.x (`choco install nsis`).

Output: `dist\dbweave_setup.exe`. A minimal single dual-mode
installer (MUI2 + NSIS MultiUser): one page asks **per-user** vs
**per-machine**, then it installs. No welcome, license, components
or directory page; the install location is fixed by the chosen
mode.

* Per-user: installs to `%LOCALAPPDATA%\Programs\DB-WEAVE`, writes
  to `HKCU`, no UAC prompt.
* Per-machine: installs to `%ProgramFiles%\DB-WEAVE`, writes to
  `HKLM`, triggers UAC.

A previous install in either hive is auto-detected and uninstalled
silently before files are laid down, so upgrades don't accumulate
orphan Qt plugin DLLs. The installer/uninstaller and Add/Remove
Programs entry use the DB-WEAVE icon, and ARP carries
`DisplayIcon`, `URLInfoAbout`, `InstallLocation`, `EstimatedSize`,
`QuietUninstallString`.

## macOS — `.dmg`

Run on any supported macOS (11 Big Sur+). The script produces a
universal-ish `.dmg` matching the host architecture; run on an
Apple-silicon Mac for an arm64 build, on Intel for x86_64.

```
brew install qt cmake ninja        # create-dmg optional, for nicer layout
export PATH="$(brew --prefix qt)/bin:$PATH"
packaging/macos/build_dmg.sh
packaging/macos/build_dmg.sh --no-loom
packaging/macos/build_dmg.sh --sign "Developer ID Application: ..."
```

What it does:

1. CMake configure + build (no tests).
2. `cmake --install` lays `DB-WEAVE.app` into `dist/macos-stage/`.
3. `macdeployqt` copies the Qt frameworks into the bundle.
4. Docs + `LICENSE` copied next to the `.app`.
5. `create-dmg` (preferred) or `hdiutil create` (fallback) bakes the
   `.dmg`.
6. Optional `codesign --deep` pass when `--sign <identity>` is given.

Output: `dist/DB-WEAVE-<version>-<arch>.dmg`.

To notarise the resulting `.dmg`, do it separately with
`xcrun notarytool submit … --wait`. The script stops short of
notarisation because it requires an Apple Developer account.

## Re-using the scripts on CI

None of the scripts touch `packaging/`-specific CI infrastructure —
they run in whatever shell invokes them. Drop each onto the matching
runner (`ubuntu-latest`, `windows-latest`, `macos-latest`) and they
work. A ready-made GitHub Actions matrix is intentionally **not**
checked in (upstream requested no CI yaml); the scripts are set up to
make one trivial to add later.
