#!/bin/bash
# Build ElephantCrush (AU + VST3, native arm64 + x86_64) and install it.
# Requires: Xcode + CMake  (brew install cmake)
set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

echo "==== ElephantCrush build ===="
if ! command -v cmake >/dev/null 2>&1; then
    echo "CMake not found. Install it first:  brew install cmake"
    read -r -p "Press Return to close..." _; exit 1
fi

echo "Configuring (first run downloads JUCE, ~1-2 min)..."
cmake -B build -G Xcode -DCMAKE_BUILD_TYPE=Release

echo "Building AU + VST3..."
cmake --build build --config Release --parallel

ART="build/ElephantCrush_artefacts/Release"
DIST="$DIR/Dist"
rm -rf "$DIST"; mkdir -p "$DIST"
[ -d "$ART/VST3/ElephantCrush.vst3" ]    && cp -R "$ART/VST3/ElephantCrush.vst3"    "$DIST/"
[ -d "$ART/AU/ElephantCrush.component" ] && cp -R "$ART/AU/ElephantCrush.component" "$DIST/"

echo
echo "Done. Installed to your user plug-in folders (this Mac, native arm64):"
echo "  ~/Library/Audio/Plug-Ins/Components/ElephantCrush.component"
echo "  ~/Library/Audio/Plug-Ins/VST3/ElephantCrush.vst3"
echo
echo "Copies for another Mac are in:  $DIST"
read -r -p "Press Return to close..." _
