#!/bin/bash
set -euo pipefail

# Rebuild base AOSP exactly like the author used:
MANIFEST_URL="https://android.googlesource.com/platform/manifest"
TAG="android-14.0.0_r67"

AOSP_ROOT="$HOME/aosp/source"

echo "[1/4] Create AOSP directory: $AOSP_ROOT"
mkdir -p "$AOSP_ROOT"
cd "$AOSP_ROOT"

echo "[2/4] repo init (AOSP Android 14 tag: $TAG)"
repo init -u "$MANIFEST_URL" -b "$TAG"

echo "[3/4] repo sync"
repo sync -c -j"$(nproc)"

echo "[4/4] Done."
echo "Next:"
echo "  cd ~/GatewayIoT"
echo "  ./scripts/apply_patches.sh"
echo "  ./scripts/build.sh"
