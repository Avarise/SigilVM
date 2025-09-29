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
PKG_DIR="$REPO_ROOT/pkg"
ASSETS_DIR="$REPO_ROOT/assets"
PUBLIC_HEADER_DIR="$REPO_ROOT/include/sigil"


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


prepare_pkg_cache() {
    local pkg_short="$1"

    if [[ -z "$pkg_short" ]]; then
        echo "[error] no package name provided" >&2
        exit 1
    fi

    # Find matching PKGBUILD
    local pkgbuild_src
    pkgbuild_src="$(ls "$PKG_DIR"/sigilvm-"$pkg_short".PKGBUILD 2>/dev/null || true)"

    if [[ -z "$pkgbuild_src" ]]; then
        echo "[error] no PKGBUILD matching sigilvm-$pkg_short.PKGBUILD found in $PKG_DIR" >&2
        exit 1
    fi

    local pkg_workdir="$PKG_CACHE/$pkg_short"

    echo "[package] preparing package workspace: $pkg_workdir" >&2

    rm -rf "$pkg_workdir"
    mkdir -p "$pkg_workdir"

    # Copy PKGBUILD as canonical name
    cp "$pkgbuild_src" "$pkg_workdir/PKGBUILD"

    # Copy required inputs
    if [[ -d "$OUT_DIR" ]]; then
        mkdir -p "$pkg_workdir/src"
        cp -r "$OUT_DIR" "$pkg_workdir/src/out"
    else
        echo "[error] missing out/ directory at $OUT_DIR" >&2
        exit 1
    fi

    if [[ -d "$ASSETS_DIR" ]]; then
        cp -r "$ASSETS_DIR" "$pkg_workdir/src/assets"
    else
        echo "[package] no assets/ directory present, skipping" >&2
    fi

    if [[ -d "$PUBLIC_HEADER_DIR" ]]; then
        mkdir -p "$pkg_workdir/src/include"
        cp -r "$PUBLIC_HEADER_DIR" "$pkg_workdir/src/include/sigil"
    else
        echo "[package] no header directory present, skipping" >&2
    fi

    echo "[package] workspace ready" >&2

    # RETURN VALUE (stdout only)
    printf '%s\n' "$pkg_workdir"
}


# ============================================================
# Packaging Step
# ============================================================
if [[ "$MODE" == "package" ]]; then
    echo "[package] target: $PACKAGE_TARGET"

    mkdir -p "$PKG_CACHE" "$OUT_DIR"

    if [[ "$PACKAGE_TARGET" == "all" ]]; then
        TARGETS=(core desktop extra)
    else
        TARGETS=("$PACKAGE_TARGET")
    fi

    for target in "${TARGETS[@]}"; do
        echo "[package] building package: $target"

        PKG_WORKDIR="$(prepare_pkg_cache "$target")"

        pushd "$PKG_WORKDIR" >/dev/null

        cd $PKG_WORKDIR

        makepkg --noconfirm

        # Copy, do not move, artifacts
        find . -maxdepth 1 -name "*.pkg.tar.zst" -exec cp {} "$OUT_DIR/" \;

        popd >/dev/null

        echo "[package] $target done"
    done

    echo "[package] all requested packages built"
    exit 0
fi



# ============================================================
# Script finalization
# ============================================================
echo "[build] completed successfully."
exit 0
