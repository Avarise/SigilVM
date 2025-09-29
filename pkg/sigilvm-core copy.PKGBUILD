# SigilVM Core: Shared library and tools for SigilVM ecosystem

pkgname=sigilvm-core
pkgver=@VERSION@
pkgrel=@RELEASE@
pkgdesc="SigilVM core environment themes and theme selector applet"
arch=('x86_64')
url="https://example.org/sigilvm"
license=('MIT')

# Core desktop stack dependencies (minimal reproducible runtime)
depends=()

# Build system & utilities (optional for packager, not end user)
makedepends=('git' 'python' 'rsync')

# Optional additional packages that improve experience
optdepends=(
)

provides=('sigilvm-core')
conflicts=('sigilvm-core-git')

# Sources are staged by the Python packaging script
source=()
noextract=()
sha256sums=()
package() {
    echo "→ Installing SigilVM Core files into package directory"
    echo "Working directory: $(pwd)"
    echo "Startdir: $startdir"
    echo "Srcdir: $srcdir"

    # Ensure required directories exist inside the package root
    install -d "$pkgdir/usr/bin"
    install -d "$pkgdir/usr/lib"

    # Copy the compiled sigilvm-tools
    if [[ -f "$startdir/usr/bin/sigilvm-tools" ]]; then
        install -Dm755 "$startdir/usr/bin/sigilvm-tools" \
            "$pkgdir/usr/bin/sigilvm-tools"
        echo "→ Installed sigilvm-tools"
    else
        echo "[ERROR] sigilvm-tools binary not found at $startdir/usr/bin/"
        exit 1
    fi

    # Copy the compiled sigilvm-editor
    if [[ -f "$startdir/usr/bin/sigilvm-editor" ]]; then
        install -Dm755 "$startdir/usr/bin/sigilvm-editor" \
            "$pkgdir/usr/bin/sigilvm-editor"
        echo "→ Installed sigilvm-editor"
    else
        echo "[ERROR] sigilvm-editor binary not found at $startdir/usr/bin/"
        exit 1
    fi

    # Copy the compiled shared library
    if [[ -f "$startdir/usr/lib/libsigilvm.a" ]]; then
        install -Dm755 "$startdir/usr/lib/libsigilvm.a" \
            "$pkgdir/usr/lib/libsigilvm.a"
        echo "→ Installed libsigilvm.a"
    else
        echo "[ERROR] lib/libsigilvm.a not found at $startdir/usr/lib/"
        exit 1
    fi



    # Optional: clean up unneeded metadata files
    find "$pkgdir" -name '.DS_Store' -delete 2>/dev/null || true
    find "$pkgdir" -name 'Thumbs.db' -delete 2>/dev/null || true

    echo "✔ SigilVM core package assembled successfully."
}
