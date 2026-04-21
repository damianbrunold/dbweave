#!/usr/bin/env bash
# Builds DB-WEAVE.app on macOS and wraps it into a .dmg via
# macdeployqt + hdiutil. Runs unchanged on Apple Silicon and Intel.
#
# Usage:     packaging/macos/build_dmg.sh [--no-loom] [--sign IDENTITY]
# Output:    dist/DB-WEAVE-<version>-<arch>.dmg
#
# Prerequisites:
#     brew install qt cmake ninja create-dmg      # create-dmg optional;
#                                                 # script falls back to hdiutil.
#     export PATH="$(brew --prefix qt)/bin:$PATH"
#     # Qt6SerialPort is shipped with the full brew install.

set -euo pipefail

NO_LOOM=OFF
SIGN_IDENT=""
for arg in "$@"; do
    case "$arg" in
        --no-loom)   NO_LOOM=ON ;;
        --sign)      shift; SIGN_IDENT="${1:?--sign requires an identity}"; shift ;;
        --sign=*)    SIGN_IDENT="${arg#--sign=}" ;;
        -h|--help)
            sed -n '2,15p' "$0"
            exit 0
            ;;
        *) echo "unknown arg: $arg" >&2; exit 2 ;;
    esac
done

SOURCE_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="$SOURCE_DIR/build-macos"
STAGE_DIR="$SOURCE_DIR/dist/macos-stage"
DIST_DIR="$SOURCE_DIR/dist"
ARCH="$(uname -m)"

mkdir -p "$BUILD_DIR" "$DIST_DIR"
rm -rf "$STAGE_DIR"

echo "==> configure (no-loom=$NO_LOOM)"
cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DDBWEAVE_BUILD_TESTS=OFF \
      -DDBWEAVE_NO_LOOM="$NO_LOOM"

echo "==> build"
cmake --build "$BUILD_DIR"

echo "==> install DB-WEAVE.app into stage dir"
mkdir -p "$STAGE_DIR"
cmake --install "$BUILD_DIR" --prefix "$STAGE_DIR"

APP_BUNDLE="$STAGE_DIR/dbweave.app"
# CMake's MACOSX_BUNDLE name follows the target; rename so the user
# sees the product name in Finder.
if [[ -d "$APP_BUNDLE" ]]; then
    mv "$APP_BUNDLE" "$STAGE_DIR/DB-WEAVE.app"
    APP_BUNDLE="$STAGE_DIR/DB-WEAVE.app"
fi
if [[ ! -d "$APP_BUNDLE" ]]; then
    echo "error: no .app bundle was installed into $STAGE_DIR" >&2
    exit 1
fi

echo "==> macdeployqt"
# -always-overwrite so repeated runs don't choke on leftover plugin
# copies. Skip -dmg here; we build the .dmg ourselves so the docs
# end up alongside the app.
macdeployqt "$APP_BUNDLE" -always-overwrite

# Copy docs next to the .app so the final disk image carries them.
for f in LICENSE dbw_manual.pdf dbw_handbuch.pdf; do
    if [[ -f "$SOURCE_DIR/$f" ]]; then
        cp "$SOURCE_DIR/$f" "$STAGE_DIR/$f"
    fi
done

if [[ -n "$SIGN_IDENT" ]]; then
    echo "==> codesign"
    codesign --deep --force --options runtime --sign "$SIGN_IDENT" "$APP_BUNDLE"
fi

VERSION="$(awk -F'"' '/VERSION 0\.[0-9]/ { print $2; exit }' "$SOURCE_DIR/CMakeLists.txt" 2>/dev/null \
           || awk '/project\(dbweave/,/LANGUAGES/' "$SOURCE_DIR/CMakeLists.txt" | awk '/VERSION/ { print $2; exit }')"
VERSION="${VERSION:-0.1.0}"
DMG_NAME="DB-WEAVE-${VERSION}-${ARCH}.dmg"
DMG_OUT="$DIST_DIR/$DMG_NAME"
rm -f "$DMG_OUT"

echo "==> create .dmg ($DMG_NAME)"
if command -v create-dmg >/dev/null 2>&1; then
    create-dmg \
        --volname "DB-WEAVE $VERSION" \
        --window-size 520 380 \
        --icon-size 96 \
        --icon "DB-WEAVE.app" 140 190 \
        --app-drop-link 380 190 \
        "$DMG_OUT" "$STAGE_DIR"
else
    # Fallback: hdiutil. The result is functional but less polished.
    hdiutil create -volname "DB-WEAVE $VERSION" \
        -srcfolder "$STAGE_DIR" \
        -ov -format UDZO "$DMG_OUT"
fi

echo "==> done. DMG at $DMG_OUT"
