#!/bin/bash
set -e

AOSP_ROOT=$HOME/aosp/source
PATCH_ROOT=$(pwd)/patches

apply() {
  local proj_path=$1
  local patch_dir=$2

  if [ -d "$PATCH_ROOT/$patch_dir" ]; then
    echo "Applying patches for $proj_path"
    cd "$AOSP_ROOT/$proj_path"
    git am --whitespace=fix "$PATCH_ROOT/$patch_dir"/*.patch
    cd - >/dev/null
  else
    echo "No patches for $proj_path"
  fi
}

apply device/brcm/rpi4 device_brcm_rpi4

echo "All patches applied successfully"
