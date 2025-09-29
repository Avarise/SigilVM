# Maintainer: Sebastian Czapla (https://github.com/Avarise)
pkgname=('sigilvm-desktop')
pkgbase='sigilvm-desktop'
pkgver=0.1.0
pkgrel=1
pkgdesc='SigilVM Desktop theme manager for Hyprland'
url='https://github.com/Avarise/SigilVM'
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
sha512sums=()

build() {
    :
}

package() {
    # === Pre-flight checks ===
    if [[ ! -x "$srcdir/out/bin/sigilvm-player" ]]; then
        echo "error: missing executable out/bin/sigilvm-player" >&2
        exit 1
    fi

    if [[ ! -d "$srcdir/assets/themes" ]]; then
        echo "error: missing themes directory at assets/themes" >&2
        exit 1
    fi

    # === Install binary ===
    install -d "$pkgdir/usr/bin"
    install -m 0755 \
        "$srcdir/out/bin/sigilvm-player" \
        "$pkgdir/usr/bin/sigilvm-player"

    # === Install themes ===
    install -d "$pkgdir/usr/share/sigilvm/themes"
    cp -r "$srcdir/assets/themes/"* "$pkgdir/usr/share/sigilvm/themes/"

    # === Install scripts ===
    install -d "$pkgdir/usr/share/sigilvm/scripts"
    cp -r "$srcdir/assets/scripts/"* "$pkgdir/usr/share/sigilvm/scripts/"

}
