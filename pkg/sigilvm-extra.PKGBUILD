# Maintainer: Avarise (https://github.com/Avarise)
pkgname=('sigilvm-extra')
pkgbase='sigilvm-extra'
pkgdesc='SigilVM extra utilities and assets (editor, xorit) and '
url='https://github.com/Avarise/SigilVM'
license=('MIT')
arch=('x86_64')
depends=()
makedepends=('cmake' 'gcc' 'make')
source=()
sha512sums=('SKIP')

BUILD_TYPE="${BUILD_TYPE:-Release}"
if [ "$BUILD_TYPE" = "Debug" ]; then
  pkgname=('sigilvm-extra-debug')
fi

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

  if [ ! -f "$CACHE_DIR/CMakeCache.txt" ]; then
    cmake -S "$REPO_ROOT" -B "$CACHE_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
  fi
  cmake --build "$CACHE_DIR" -- -j"$(nproc)"
}

package() {
  REPO_ROOT="${REPO_ROOT:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
  BUILD_DIR="$REPO_ROOT/build"

  install -dm755 "$pkgdir/usr/bin"
  if [ -x "$BUILD_DIR/bin/sigilvm-editor" ]; then
    install -Dm755 "$BUILD_DIR/bin/sigilvm-editor" "$pkgdir/usr/bin/sigilvm-editor"
  fi
  if [ -x "$BUILD_DIR/bin/sigilvm-xorit" ]; then
    install -Dm755 "$BUILD_DIR/bin/sigilvm-xorit" "$pkgdir/usr/bin/sigilvm-xorit"
  fi

  if [ "$BUILD_TYPE" != "Debug" ]; then
    find "$pkgdir/usr/bin" -type f -executable -exec strip --strip-all {} + 2>/dev/null || true
  fi
}
