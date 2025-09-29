# Maintainer: Operator <you@example.com>
pkgname=('sigilvm-core')
pkgbase='sigilvm-core'
pkgdesc='SigilVM core utilities and library'
url='https://example.org/sigilvm'
license=('MIT')
arch=('x86_64')
depends=()
makedepends=('cmake' 'gcc' 'make' 'pkg-config')
install=
source=()
sha512sums=('SKIP')

# Allow BUILD_TYPE override and debug package name
BUILD_TYPE="${BUILD_TYPE:-Release}"
if [ "$BUILD_TYPE" = "Debug" ]; then
  pkgname=('sigilvm-core-debug')
fi

# attempt to derive version from git
pkgver() {
  REPO_ROOT="${REPO_ROOT:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
  cd "$REPO_ROOT"
  ver=$(git describe --tags --dirty --always 2>/dev/null || git rev-parse --short HEAD)
  echo "${ver#v}"
}

prepare() {
  return 0
}

build() {
  REPO_ROOT="${REPO_ROOT:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
  CACHE_DIR="$REPO_ROOT/.cache"

  # configure only if no cache present
  if [ ! -f "$CACHE_DIR/CMakeCache.txt" ]; then
    cmake -S "$REPO_ROOT" -B "$CACHE_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
  fi

  cmake --build "$CACHE_DIR" -- -j"$(nproc)"
}

package() {
  REPO_ROOT="${REPO_ROOT:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
  BUILD_DIR="$REPO_ROOT/build"

  # Binaries
  install -dm755 "$pkgdir/usr/bin"
  if [ -x "$BUILD_DIR/bin/sigilvm-tools" ]; then
    install -Dm755 "$BUILD_DIR/bin/sigilvm-tools" "$pkgdir/usr/bin/sigilvm-tools"
  fi

  # Libraries (static .a packaged now). .so packaging is commented out for future use.
  install -dm755 "$pkgdir/usr/lib"
  if [ -f "$BUILD_DIR/lib/libsigilvm.a" ]; then
    install -Dm644 "$BUILD_DIR/lib/libsigilvm.a" "$pkgdir/usr/lib/libsigilvm.a"
  fi

  # If/when libsigilvm.so is produced by CMake, uncomment the following lines:
  # if [ -f "$BUILD_DIR/lib/libsigilvm.so" ]; then
  #   install -Dm644 "$BUILD_DIR/lib/libsigilvm.so" "$pkgdir/usr/lib/libsigilvm.so"
  # fi

  # Headers
  if [ -d "$REPO_ROOT/include" ]; then
    install -dm755 "$pkgdir/usr/include"
    cp -a "$REPO_ROOT/include/"* "$pkgdir/usr/include/"
  fi

  # ELF metadata and strip (keep debug in debug package)
  if [ "$BUILD_TYPE" != "Debug" ]; then
    find "$pkgdir/usr/bin" -type f -executable -exec strip --strip-all {} + 2>/dev/null || true
    find "$pkgdir/usr/lib" -type f -name "*.a" -exec true {} \; # no strip for .a
  fi
}
