#!/bin/bash
set -e

cd "$HOME/aosp/source"

source build/envsetup.sh
lunch aosp_rpi4_tv-ap2a-userdebug

make -j$(nproc)
