#!/bin/bash
set -e

# Canonical build script (GUI-first)
# Usage:
#   ./scripts/create_build.sh            # Release build
#   BUILD_TYPE=Debug ./scripts/create_build.sh

BUILD_DIR=${BUILD_DIR:-build-gui}
BUILD_TYPE=${BUILD_TYPE:-Release}

mkdir -p "${BUILD_DIR}"
cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
cmake --build "${BUILD_DIR}" -j"$(nproc)"

echo "Build complete."
echo "Run: ./${BUILD_DIR}/library_gui"

