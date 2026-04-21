#!/usr/bin/env bash
# Builds a relocatable DB-WEAVE AppImage via linuxdeploy +
# linuxdeploy-plugin-qt. Requires both binaries on $PATH and
# a Qt 6.5+ development environment.
#
# Usage:  packaging/linux/build_appimage.sh [--no-loom]
# Output: dist/DB-WEAVE-<version>-x86_64.AppImage
#
# Prerequisites (Debian/Ubuntu):
#     sudo apt install qt6-base-dev qt6-base-dev-tools qt6-tools-dev \
#                      qt6-tools-dev-tools libqt6serialport6-dev \
#                      cmake ninja-build
#     # with --no-loom libqt6serialport6-dev can be omitted.
#     # Grab linuxdeploy + the Qt plugin:
#     wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
#     wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
#     chmod +x linuxdeploy-*.AppImage
#     export PATH="$PWD:$PATH"

set -euo pipefail

NO_LOOM=OFF
for arg in "$@"; do
    case "$arg" in
        --no-loom) NO_LOOM=ON ;;
        -h|--help)
            sed -n '2,20p' "$0"
            exit 0
            ;;
        *) echo "unknown arg: $arg" >&2; exit 2 ;;
    esac
done

SOURCE_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="$SOURCE_DIR/build-appimage"
DIST_DIR="$SOURCE_DIR/dist"
APPDIR="$BUILD_DIR/AppDir"

mkdir -p "$BUILD_DIR" "$DIST_DIR"

echo "==> configure (no-loom=$NO_LOOM)"
cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DDBWEAVE_BUILD_TESTS=OFF \
      -DDBWEAVE_NO_LOOM="$NO_LOOM"

echo "==> build"
cmake --build "$BUILD_DIR"

echo "==> install into AppDir"
rm -rf "$APPDIR"
DESTDIR="$APPDIR" cmake --install "$BUILD_DIR" --prefix /usr

echo "==> linuxdeploy + plugin-qt"
# linuxdeploy relocates libraries, copies Qt runtime, bundles the
# desktop + icon, and emits the AppImage. The Qt plugin discovers
# Qt modules by running ldd on the executable.
QMAKE="${QMAKE:-$(command -v qmake6 || command -v qmake)}"
if [[ -z "$QMAKE" ]]; then
    echo "qmake/qmake6 not on PATH; install qt6-base-dev-tools." >&2
    exit 1
fi
export QMAKE

linuxdeploy --appdir "$APPDIR" \
            --desktop-file "$APPDIR/usr/share/applications/dbweave.desktop" \
            --icon-file "$APPDIR/usr/share/icons/hicolor/scalable/apps/dbweave.svg" \
            --plugin qt \
            --output appimage

# linuxdeploy writes the .AppImage into the current directory.
mv -v DB-WEAVE*.AppImage "$DIST_DIR/" || mv -v dbweave*.AppImage "$DIST_DIR/"

echo "==> done. AppImage in $DIST_DIR/"
ls -la "$DIST_DIR"/*.AppImage 2>/dev/null || true
