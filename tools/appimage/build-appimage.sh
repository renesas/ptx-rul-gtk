#!/usr/bin/env bash
# Builds a portable AppImage of the PTX205R GTK4 application (aarch64, Raspberry Pi OS).
# The AppImage runs on systems with the same or a newer glibc than the build machine.
# Usage: tools/appimage/build-appimage.sh
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
PROJECT="$ROOT/PROJECTS/PTX2xxR/NFC_FORUM_BASIC_DISCOVERY"
BUILD="$HERE/build"
APPDIR="$BUILD/AppDir"
CACHE="$HERE/.cache"
ARCH="$(uname -m)"
GH="https://github.com/linuxdeploy"

VERSION="$(sed -n 's/^#define PTX_VERSION_\(MAJOR\|MINOR\|PATCH\) \([0-9]*\).*/\2/p' \
    "$ROOT/SRC/EXAMPLES/ptx_Example_NFCForumBasicDiscovery/ptx_Version.h" | head -3 | paste -sd. -)"
export VERSION

mkdir -p "$CACHE"
fetch() { # url file
    [ -x "$CACHE/$2" ] || { curl -fL -o "$CACHE/$2" "$1" && chmod +x "$CACHE/$2"; }
}
fetch "$GH/linuxdeploy/releases/download/continuous/linuxdeploy-$ARCH.AppImage" linuxdeploy
fetch "$GH/linuxdeploy-plugin-gtk/raw/master/linuxdeploy-plugin-gtk.sh" linuxdeploy-plugin-gtk.sh
fetch "$GH/linuxdeploy-plugin-appimage/releases/download/continuous/linuxdeploy-plugin-appimage-$ARCH.AppImage" linuxdeploy-plugin-appimage

# 1. Build and install into the AppDir
rm -rf "$BUILD"
cmake -S "$PROJECT" -B "$BUILD/cmake" -DCMAKE_BUILD_TYPE=Release \
    -DPTX_BUILD_LIN=ON -DPTX_ENABLE_SPI=ON -DPTX_UTILITY_PRINTING=Console -DPTX_LOGGING=File
cmake --build "$BUILD/cmake" -j"$(nproc)"
cmake --install "$BUILD/cmake" --prefix "$APPDIR/usr"
glib-compile-schemas "$APPDIR/usr/share/glib-2.0/schemas"

# 2. Environment for the running AppImage (sourced by AppRun, after the GTK plugin's hook).
#    The dconf GIO module is not bundled, so use the built-in keyfile backend for GSettings.
mkdir -p "$APPDIR/apprun-hooks"
cat > "$APPDIR/apprun-hooks/ptx-env.sh" <<'HOOK'
export GSETTINGS_BACKEND=keyfile
# The GTK plugin forces GTK_THEME="Adwaita:light" / "Adwaita:dark". Remove it so that GTK behaves as in the plain
# build and honours gtk-application-prefer-dark-theme (the app's dark-mode switch).
unset GTK_THEME
HOOK

# 3. Bundle the libraries and create the AppImage
export PATH="$CACHE:$PATH"
export APPIMAGE_EXTRACT_AND_RUN=1   # no FUSE needed for the build tools
export DEPLOY_GTK_VERSION=4
export LINUXDEPLOY_OUTPUT_VERSION="$VERSION"
cd "$BUILD"
"$CACHE/linuxdeploy" --appdir "$APPDIR" --plugin gtk \
    --executable "$APPDIR/usr/bin/NFC_FORUM_BASIC_DISCOVERY" \
    --library "$APPDIR/usr/lib/libRUL.so" \
    --desktop-file "$APPDIR/usr/share/applications/com.renesas.nfc.desktop" \
    --icon-file "$APPDIR/usr/share/icons/hicolor/256x256/apps/com.renesas.nfc.png" \
    --output appimage

OUT="$ROOT/PTX205R_Discovery-$VERSION-$ARCH.AppImage"
mv -f "$BUILD"/*.AppImage "$OUT"
echo "Created $OUT"
