#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CACHE_DIR="$REPO_ROOT/.cache"
BUILD_DIR="$REPO_ROOT/build"
PKG_DIR="$REPO_ROOT/pkg"

# default Release
BUILD_TYPE="${BUILD_TYPE:-Release}"

# CLI overrides
PACKAGES=()
for arg in "$@"; do
  case "$arg" in
    --debug) BUILD_TYPE="Debug" ;;
    --release) BUILD_TYPE="Release" ;;
    --packages=*) IFS=, read -r -a PACKAGES <<< "${arg#*=}" ;;
    --help|-h)
      cat <<EOF
Usage: $(basename "$0") [--debug|--release] [--packages=core,desktop,extra]
Defaults to Release and packages all three.
EOF
      exit 0
      ;;
    *)
      ;;
  esac
done

# default set
if [ ${#PACKAGES[@]} -eq 0 ]; then
  PACKAGES=("sigilvm-core" "sigilvm-desktop" "sigilvm-extra")
fi

mkdir -p "$CACHE_DIR" "$BUILD_DIR" "$BUILD_DIR/packages"

# compute git commit to detect changes
GIT_COMMIT="$(git -C "$REPO_ROOT" rev-parse --short HEAD 2>/dev/null || echo unknown)"
LAST_COMMIT_FILE="$CACHE_DIR/last_commit"

REBUILD=0
if [ ! -f "$CACHE_DIR/CMakeCache.txt" ]; then
  REBUILD=1
else
  LAST="$(cat "$LAST_COMMIT_FILE" 2>/dev/null || echo none)"
  if [ "$LAST" != "$GIT_COMMIT" ]; then REBUILD=1; fi
fi

if [ "$REBUILD" -eq 1 ]; then
  echo "Configuring and building in $CACHE_DIR (BUILD_TYPE=$BUILD_TYPE)..."
  cmake -S "$REPO_ROOT" -B "$CACHE_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
  cmake --build "$CACHE_DIR" -- -j"$(nproc)"
  echo "$GIT_COMMIT" > "$LAST_COMMIT_FILE"
else
  echo "No source changes detected. Performing incremental build..."
  cmake --build "$CACHE_DIR" -- -j"$(nproc)"
fi

# Ensure build artifacts accessible in build/
# If your CMake already writes to build/, this is cheap.
rsync -a --delete "$CACHE_DIR/bin/" "$BUILD_DIR/bin/" || true
rsync -a --delete "$CACHE_DIR/lib/" "$BUILD_DIR/lib/" || true

# Run makepkg for selected packages
for p in "${PACKAGES[@]}"; do
  PKGFILE="$PKG_DIR/$p.PKGBUILD"
  if [ ! -f "$PKGFILE" ]; then
    echo "Package file not found: $PKGFILE"
    exit 1
  fi

  # call makepkg in the pkg directory; export REPO_ROOT and BUILD_TYPE so PKGBUILDs can use them
  echo "Packaging $p (BUILD_TYPE=$BUILD_TYPE)..."
  (
    cd "$PKG_DIR"
    REPO_ROOT="$REPO_ROOT" BUILD_TYPE="$BUILD_TYPE" \
      makepkg --skipinteg --noconfirm || { echo "makepkg failed for $p"; exit 1; }
    # move produced packages into build/packages
    mv ./*.pkg.* "$BUILD_DIR/packages/" 2>/dev/null || true
  )
done

echo "Done. Packages can be found in: $BUILD_DIR/packages/"
