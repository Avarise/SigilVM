# Maintainer: Sebastian Czapla (https://github.com/Avarise)
pkgname=('sigilvm-extra')
pkgbase='sigilvm-extra'
pkgver=0.1.0
pkgrel=1
pkgdesc='SigilVM Editor, Xorit, and optional assets'
url='https://github.com/Avarise/SigilVM'
license=('MIT')
arch=('x86_64')
depends=()
makedepends=('cmake' 'gcc' 'make')
source=()
sha512sums=()

build() {
    :
}

package() {
    # === Pre-flight checks ===
    local bins=("sigilvm-xorit" "sigilvm-dotexe" "sigilvm-editor")
    for b in "${bins[@]}"; do
        if [[ ! -x "$srcdir/out/bin/$b" ]]; then
            echo "error: missing executable out/bin/$b" >&2
            exit 1
        fi
    done

    # === Install binaries ===
    install -d "$pkgdir/usr/bin"

    for b in "${bins[@]}"; do
        install -m 0755 \
            "$srcdir/out/bin/$b" \
            "$pkgdir/usr/bin/$b"
    done
}
