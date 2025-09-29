# Maintainer: Sebastian Czapla (https://github.com/Avarise)
pkgname=('sigilvm-core')
pkgbase='sigilvm-core'
pkgver=0.1.0
pkgrel=1
pkgdesc='SigilVM Core package'
url='https://github.com/Avarise/SigilVM'
license=('MIT')
arch=('x86_64')

depends=('glibc')

makedepends=('cmake' 'gcc' 'make')

source=()
sha512sums=()

options=('!buildflags' '!strip')

build() {
    :
}

package() {
    if [[ ! -x "$srcdir/out/bin/sigilvm-tools" ]]; then
        echo "error: missing executable out/bin/sigilvm-tools" >&2
        exit 1
    fi

    if [[ ! -f "$srcdir/out/lib/libsigilvm.so" ]]; then
        echo "error: missing shared library out/lib/libsigilvm.so" >&2
        exit 1
    fi

    # Public headers
    if [[ ! -d "$srcdir/include/sigil" ]]; then
        echo "error: missing public headers at include/sigil" >&2
        exit 1
    fi

    # Install layout
    install -d "$pkgdir/usr/bin"
    install -d "$pkgdir/usr/lib"

    # Core management tool
    install -m 0755 \
        "$srcdir/out/bin/sigilvm-tools" \
        "$pkgdir/usr/bin/sigilvm-tools"

    install -m 0644 \
        "$srcdir/out/lib/libsigilvm.so" \
        "$pkgdir/usr/lib/libsigilvm.so"

    # Install public headers
    install -d "$pkgdir/usr/include/sigil"
    cp -r "$srcdir/include/sigil/"* "$pkgdir/usr/include/sigil/"
}
