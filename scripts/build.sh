#!/usr/bin/env bash

set -e

cat << 'EOF'
   _____ _       _ ___      ____  __ 
  / ____(_)     (_) \ \    / /  \/  |
 | (___  _  __ _ _| |\ \  / /| \  / |
  \___ \| |/ _` | | | \ \/ / | |\/| |
  ____) | | (_| | | |  \  /  | |  | |
 |_____/|_|\__, |_|_|   \/   |_|  |_|
            __/ |                    
           |___/                     
EOF

# -------------------------------------------------
# parse args
# -------------------------------------------------
BUILD_TYPE="Debug"
RUN_TESTS=0
DO_CLEAN=0

for arg in "$@"; do
    case "$arg" in
        --release)
            BUILD_TYPE="Release"
            ;;
        --debug)
            BUILD_TYPE="Debug"
            ;;
        --test)
            RUN_TESTS=1
            ;;
        --clean)
            DO_CLEAN=1
            ;;
    esac
done

# -------------------------------------------------
# resolve repo root
# -------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# -------------------------------------------------
# clean mode
# -------------------------------------------------
if [[ "$DO_CLEAN" -eq 1 ]]; then
    TARGETS=(
        "$REPO_ROOT/.cache/vm"
        "$REPO_ROOT/build"
    )

    for t in "${TARGETS[@]}"; do
        if [[ -d "$t" ]]; then
            echo "[clean] removing $t"
            rm -rf "$t"
        else
            echo "[clean] skipped (not present): $t"
        fi
    done

    echo "[clean] done."
    exit 0
fi

# Auto-enable tests for Release builds
if [[ "$BUILD_TYPE" == "Release" ]]; then
    RUN_TESTS=1
fi

echo "[build] repo root: $REPO_ROOT"

# -------------------------------------------------
# update ImGui (docking branch)
# -------------------------------------------------
IMGUI_DIR="$REPO_ROOT/extern/imgui"

if [[ -d "$IMGUI_DIR" ]]; then
    echo "[build] updating ImGui (docking branch)"
    pushd "$IMGUI_DIR" > /dev/null
    git fetch origin
    git checkout docking
    git pull --ff-only origin docking
    popd > /dev/null
else
    echo "[build] ERROR: extern/imgui is missing"
    exit 1
fi

# -------------------------------------------------
# prepare build directory
# -------------------------------------------------
BUILD_DIR="$REPO_ROOT/.cache/vm"
mkdir -p "$BUILD_DIR"
echo "[build] build directory: $BUILD_DIR"

# -------------------------------------------------
# run CMake configure + build
# -------------------------------------------------
echo "[build] configuring (BUILD_TYPE=$BUILD_TYPE)"
cmake -S "$REPO_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

echo "[build] compiling"
cmake --build "$BUILD_DIR" --parallel "$(nproc)"

# -------------------------------------------------
# run tests if requested
# -------------------------------------------------
if [[ "$RUN_TESTS" -eq 1 ]]; then
    echo "[build] running tests"
    ctest --test-dir "$BUILD_DIR" --output-on-failure
fi

echo "[build] done."
