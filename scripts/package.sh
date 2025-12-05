#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
PKG_DIR="${REPO_ROOT}/pkg"

# ---------------------------------------------
# Defaults
# ---------------------------------------------
BUILD_TYPE="Release"
PKG_SUFFIX=""    # e.g. -debug if debug mode requested

TARGET=""
DO_DEBUG=0

# ---------------------------------------------
# Parse arguments
# ---------------------------------------------
while [[ $# -gt 0 ]]; do
    case "$1" in
        core|extra|desktop)
            TARGET="$1"
            shift
            ;;
        --debug)
            DO_DEBUG=1
            shift
            ;;
        *)
            echo "[package] Unknown argument: $1"
            exit 1
            ;;
    esac
done

# ---------------------------------------------
# Apply debug mode
# ---------------------------------------------
if [[ $DO_DEBUG -eq 1 ]]; then
    BUILD_TYPE="Debug"
    PKG_SUFFIX="-debug"
    echo "[package] Debug mode enabled"
fi

export BUILD_TYPE
export PKG_SUFFIX

echo "[package] BUILD_TYPE=$BUILD_TYPE"
echo "[package] PKG_SUFFIX=$PKG_SUFFIX"

# ---------------------------------------------
# Helper: build a single package
# ---------------------------------------------
build_one_pkg() {
    local name="$1"
    local file="${PKG_DIR}/sigilvm-${name}.PKGBUILD"

    if [[ ! -f "$file" ]]; then
        echo "[package] ERROR: PKGBUILD not found: $file"
        exit 1
    fi

    echo "----------------------------------------"
    echo "[package] Building sigilvm-$name"
    echo "----------------------------------------"

    pushd "$PKG_DIR" > /dev/null

    # Clean temp from previous builds but preserve .cache/vm (CMake cache)
    rm -rf src pkg

    # Use PKGBUILD with environment overrides
    PKGEXT=".pkg.tar.zst" \
    makepkg --clean --cleanbuild --force -p "sigilvm-${name}.PKGBUILD"

    popd > /dev/null
}

# ---------------------------------------------
# Target selection
# ---------------------------------------------
if [[ -z "$TARGET" ]]; then
    echo "[package] No package specified, building all"
    build_one_pkg "core"
    build_one_pkg "extra"
    build_one_pkg "desktop"
else
    echo "[package] Building only target: $TARGET"
    build_one_pkg "$TARGET"
fi

echo "[package] Done."
