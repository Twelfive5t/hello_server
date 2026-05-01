#!/bin/bash
set -e

# 如果没有传入参数，使用默认值 'Server_Release'
PRESET="${1:-Server_Release}"

BUILD_TYPE="Release" # 默认为 Release
if [[ "$PRESET" == *"Debug"* ]]; then
    BUILD_TYPE="Debug"
fi
BUILD_DIR="build/$BUILD_TYPE/"

# 根据 PRESET 配置并安装依赖
CONAN_PROFILE=""  # Conan 配置文件
ARCH=$(uname -m)

case "$PRESET" in
  Server_Debug)
    CONAN_PROFILE="./conanfile/linux_${ARCH}_debug"
    ;;
  Server_Release)
    CONAN_PROFILE="./conanfile/linux_${ARCH}_release"
    ;;
  *)
    echo "未知的 PRESET: $PRESET, 跳过特殊操作"
    ;;
esac

if [ ! -f "$BUILD_DIR/generators/conan_toolchain.cmake" ]; then
  echo "Running conan install..."
  conan install . --profile:all="$CONAN_PROFILE" --build=missing -c tools.system.package_manager:mode=install
else
  echo "Skipping conan install as '$BUILD_DIR/conan_toolchain.cmake' already exists."
fi

cmake --preset "$PRESET"
cmake --build --preset "$PRESET" -j$(nproc)
cmake --install "$BUILD_DIR"

./build_product_image.sh
