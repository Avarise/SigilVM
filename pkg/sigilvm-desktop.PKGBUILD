# Maintainer: Operator <you@example.com>
pkgname=('sigilvm-desktop')
pkgbase='sigilvm-desktop'
pkgdesc='SigilVM desktop assets: themes and templates'
url='https://example.org/sigilvm'
license=('MIT')
arch=('x86_64')
depends=(
  'hyprland'
  'waybar'
  'mako'
  'wofi'
  'alacritty'
  'kvantum'
  'qt5-base'
  'qt6-base'
  'qt5-wayland'
  'qt6-wayland'
  'starship'
  'zsh'
)
makedepends=('cmake' 'gcc' 'make')
source=()
sha512sums=('SKIP')

BUILD_TYPE="${BUILD_TYPE:-Release}"
if [ "$BUILD_TYPE" = "Debug" ]; then
  pkgname=('sigilvm-desktop-debug')
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
  # desktop package doesn't need to compile, but ensure build exists
  REPO_ROOT="${REPO_ROOT:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
  CACHE_DIR="$REPO_ROOT/.cache"

  if [ ! -f "$CACHE_DIR/CMakeCache.txt" ]; then
    cmake -S "$REPO_ROOT" -B "$CACHE_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    cmake --build "$CACHE_DIR" -- -j"$(nproc)"
  fi
}

package() {
  REPO_ROOT="${REPO_ROOT:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"

  # Install theme assets into /usr/share/sigilvm/themes
  install -dm755 "$pkgdir/usr/share/sigilvm/themes"
  if [ -d "$REPO_ROOT/assets/themes" ]; then
    cp -a "$REPO_ROOT/assets/themes/." "$pkgdir/usr/share/sigilvm/themes/"
  fi

  # optional manifest
  install -Dm644 /dev/null "$pkgdir/usr/share/sigilvm/manifest-desktop.txt" || true
}
