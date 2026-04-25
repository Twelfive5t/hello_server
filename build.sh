#!/bin/bash
set -x

# 如果没有传入参数，使用默认值 'Server_Release'
PRESET="${1:-Server_Release}"

BUILD_TYPE="Release" # 默认为 Release
if [[ "$PRESET" == *"Debug"* ]]; then
    BUILD_TYPE="Debug"
fi
BUILD_DIR="build/$BUILD_TYPE/"

cmake --preset "$PRESET"
cmake --build --preset "$PRESET" -j$(nproc)
cmake --install "$BUILD_DIR"