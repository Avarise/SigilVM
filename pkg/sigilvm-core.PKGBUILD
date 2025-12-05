# Maintainer: Avarise (https://github.com/Avarise)
pkgname=sigilvm-core
pkgver=0.1.0
pkgrel=1
pkgdesc='SigilVM core utilities and library'
arch=('x86_64')
url='https://github.com/Avarise/SigilVM'
license=('custom')
depends=()               # core depends on nothing
makedepends=(cmake git zstd tar)  # required to configure/build/package
options=()

# We don't fetch sources from the web — build from the repository on disk.
source=()
sha256sums=()

# ---- Path variables (easy to override for debugging) ----
# Note: PKGBUILD runs in the pkg/ directory. We treat repo root as one level up.
REPO_ROOT="$(cd "${srcdir:-.}/.." && pwd)"            # top-level SigilVM dir
CACHE_DIR="${REPO_ROOT}/.cache"                      # .cache/
CMAKE_BUILD_DIR="${CACHE_DIR}/vm"                     # CMake intermediate build dir
BUILD_DIR="${REPO_ROOT}/build"                        # final build outputs
PKGROOT="${CACHE_DIR}/pkg/${pkgname}"                 # staging root for this package
ASSETS_DIR="${REPO_ROOT}/assets"                      # assets/
IMGUI_DIR="${REPO_ROOT}/extern/imgui"                 # extern/imgui
BUILD_TYPE="Release"                                  # packaging always uses Release

# Ensure the directories exist and are absolute
_prepare_common_dirs() {
    mkdir -p "$CACHE_DIR"
    mkdir -p "$CMAKE_BUILD_DIR"
    mkdir -p "$BUILD_DIR"
}

prepare() {
    echo "[prepare] repository root: $REPO_ROOT"
    _prepare_common_dirs

    # Verify extern/imgui exists and is on docking branch
    if [[ -d "$IMGUI_DIR" ]]; then
        echo "[prepare] updating ImGui (docking branch) at $IMGUI_DIR"
        pushd "$IMGUI_DIR" > /dev/null || return 1
        git fetch origin --quiet || { popd > /dev/null; echo "[prepare] git fetch failed"; return 1; }
        git checkout docking --quiet || { popd > /dev/null; echo "[prepare] failed to checkout docking"; return 1; }
        git pull --ff-only origin docking --quiet || { popd > /dev/null; echo "[prepare] failed to pull docking"; return 1; }
        popd > /dev/null || return 1
    else
        echo "[prepare] ERROR: extern/imgui is missing at $IMGUI_DIR"
        return 1
    fi

    return 0
}

build() {
    echo "[build] BUILD_TYPE=$BUILD_TYPE"
    _prepare_common_dirs

    # Configure
    echo "[build] running cmake configure (src=$REPO_ROOT, build=$CMAKE_BUILD_DIR)"
    cmake -S "$REPO_ROOT" -B "$CMAKE_BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" || { echo "[build] cmake configure failed"; return 1; }

    # Build (let CMake decide whether to rebuild)
    echo "[build] compiling (cmake --build $CMAKE_BUILD_DIR)"
    cmake --build "$CMAKE_BUILD_DIR" --parallel "$(nproc)" || { echo "[build] cmake build failed"; return 1; }

    # Verify expected outputs exist (quick sanity checks)
    if [[ ! -f "${BUILD_DIR}/lib/libsigilvm.so" ]]; then
        echo "[build] WARNING: expected ${BUILD_DIR}/lib/libsigilvm.so not found"
        # do not fail here; packaging may still pick up libs from CMAKE_BUILD_DIR
    fi

    return 0
}

package() {
    echo "[package] staging package into $PKGROOT"
    rm -rf "$PKGROOT"
    mkdir -p "$PKGROOT"

    # Create target filesystem layout
    install -d -m755 "$PKGROOT/usr/bin"
    install -d -m755 "$PKGROOT/usr/lib"
    install -d -m755 "$PKGROOT/usr/share"

    # Install binaries: look into build/bin first, fallback to CMake build dir's output
    BIN_SRC_DIR="${BUILD_DIR}/bin"
    CMAKE_BIN_DIR="${CMAKE_BUILD_DIR}/bin"
    LIB_SRC_DIR="${BUILD_DIR}/lib"
    CMAKE_LIB_DIR="${CMAKE_BUILD_DIR}/lib"

    # Helper to copy a binary if it exists
    _install_bin_if_exists() {
        local name="$1"
        if [[ -x "${BIN_SRC_DIR}/${name}" ]]; then
            install -Dm755 "${BIN_SRC_DIR}/${name}" "$PKGROOT/usr/bin/${name}"
            echo "[package] installed ${BIN_SRC_DIR}/${name}"
        elif [[ -x "${CMAKE_BIN_DIR}/${name}" ]]; then
            install -Dm755 "${CMAKE_bin_DIR}/${name}" "$PKGROOT/usr/bin/${name}"
            echo "[package] installed ${CMAKE_bin_DIR}/${name}"
        else
            echo "[package] WARNING: binary ${name} not found in ${BIN_SRC_DIR} or ${CMAKE_BIN_DIR}"
        fi
    }

    # Explicitly install the core engine & tools
    if [[ -x "${BUILD_DIR}/bin/sigilvm-tools" ]]; then
        install -Dm755 "${BUILD_DIR}/bin/sigilvm-tools" "$PKGROOT/usr/bin/sigilvm-tools"
    elif [[ -x "${CMAKE_BUILD_DIR}/bin/sigilvm-tools" ]]; then
        install -Dm755 "${CMAKE_BUILD_DIR}/bin/sigilvm-tools" "$PKGROOT/usr/bin/sigilvm-tools"
    else
        echo "[package] WARNING: sigilvm-tools not found"
    fi

    # Install library: prefer build/lib then cmake build/lib
    if [[ -f "${LIB_SRC_DIR}/libsigilvm.so" ]]; then
        install -Dm644 "${LIB_SRC_DIR}/libsigilvm.so" "$PKGROOT/usr/lib/libsigilvm.so"
    elif [[ -f "${CMAKE_LIB_DIR}/libsigilvm.so" ]]; then
        install -Dm644 "${CMAKE_LIB_DIR}/libsigilvm.so" "$PKGROOT/usr/lib/libsigilvm.so"
    else
        echo "[package] WARNING: libsigilvm.so not found"
    fi

    # Optional: install shared headers or docs if desired (currently none for core)

    # Create final build directory for package archive
    mkdir -p "$BUILD_DIR"

    # Create .pkg.tar.zst into build/
    PKG_FILE="${BUILD_DIR}/${pkgname}.pkg.tar.zst"
    echo "[package] creating archive $PKG_FILE (from $PKGROOT)"
    # Use tar -> zstd pipeline to ensure .pkg.tar.zst format
    (cd "$PKGROOT" && tar -cf - .) | zstd -19 -o "$PKG_FILE" || { echo "[package] archive creation failed"; return 1; }

    echo "[package] done: $PKG_FILE"
    return 0
}
