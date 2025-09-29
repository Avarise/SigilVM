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

# ============================================================
# Argument Parsing
# ============================================================
MODE="build"                 # default mode
BUILD_TYPE="Debug"
RUN_TESTS=0
PACKAGE_TARGET="all"         # default: package everything

for arg in "$@"; do
    case "$arg" in
        --release)
            BUILD_TYPE="Release"
            ;;
        --debug)
            BUILD_TYPE="Debug"
            ;;
        --package)
            MODE="package"
            BUILD_TYPE="Release"
            RUN_TESTS=1
            ;;
        core|desktop|extra)
            PACKAGE_TARGET="$arg"
            ;;
        --test)
            MODE="test"
            RUN_TESTS=1
            ;;
        --clean)
            MODE="clean"
            ;;
        *)
            echo "[error] unknown argument: $arg"
            exit 1
            ;;
    esac
done


# ============================================================
# Repository root, and other paths
# ============================================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR" && pwd)"

BUILD_DIR="$REPO_ROOT/.cache/build"
PKG_CACHE="$REPO_ROOT/.cache/pkg"
OUT_DIR="$REPO_ROOT/out"


echo "[info] repo root: $REPO_ROOT"
echo "[info] mode: $MODE"
echo "[info] build type: $BUILD_TYPE"


# ============================================================
# Cleaning up the project
# ============================================================
if [[ "$MODE" == "clean" ]]; then
    TARGETS=(
        "$BUILD_DIR"
        "$OUT_DIR"
        "$PKG_CACHE"
    )

    for t in "${TARGETS[@]}"; do
        if [[ -d "$t" ]]; then
            echo "[clean] removing $t"
            rm -rf "$t"
        else
            echo "[clean] skipped: $t"
        fi
    done

    echo "[clean] done."
    exit 0
fi


# ============================================================
# Update Dear ImGui submodule
# ============================================================
IMGUI_DIR="$REPO_ROOT/extern/imgui"
if [[ -d "$IMGUI_DIR" ]]; then
    echo "[build] updating ImGui (docking branch)"
    pushd "$IMGUI_DIR" > /dev/null
    git fetch origin
    git checkout docking
    git pull --ff-only origin docking
    popd > /dev/null
else
    echo "[error] extern/imgui is missing"
    exit 1
fi


# ============================================================
# CMake based build step
# ============================================================
if [[ "$MODE" != "test" ]]; then
    echo "[build] creating build directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"

    echo "[build] configuring CMake"
    cmake -S "$REPO_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

    echo "[build] compiling"
    cmake --build "$BUILD_DIR" --parallel "$(nproc)"
fi


# ============================================================
# Testing based on CMake + GTest
# ============================================================
if [[ "$RUN_TESTS" -eq 1 ]]; then
    echo "[test] running CTest suite"
    ctest --test-dir "$BUILD_DIR" --output-on-failure
fi

# Test mode stops here
if [[ "$MODE" == "test" ]]; then
    echo "[test] done."
    exit 0
fi


# ============================================================
# Packaging Step
# ============================================================
if [[ "$MODE" == "package" ]]; then
    echo "[package] Not implemented yet."
    exit 0
fi


# ============================================================
# Script finalization
# ============================================================
echo "[build] completed successfully."
exit 0
