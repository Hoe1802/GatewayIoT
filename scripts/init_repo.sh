#!/bin/bash
set -e

AOSP_ROOT=$HOME/aosp

echo "[1/3] Create AOSP directory"
mkdir -p "$AOSP_ROOT"
cd "$AOSP_ROOT"

echo "[2/3] Init repo with locked manifest"
repo init -u ../GatewayIoT/manifest/manifest.xml

echo "[3/3] Sync AOSP source"
repo sync -j$(nproc)

echo "AOSP source initialized successfully"
