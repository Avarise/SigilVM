# Packaging Guide

---
## 1. Packaging Philosophy

SigilVM is designed so that:

- **All installation is done via packages**, not from the build directory.
- Templates and base assets are always provided by packages.
- Applications and contexts discover system paths at runtime.
- Tools behave gracefully when optional packages or templates are missing.

SigilVM currently targets **Arch Linux** first, with PKGBUILDs defining the official packaging workflow.


---
## 2. Package Layout

The repository’s `pkg/` directory contains:

```
pkg/
├── sigilvm-core.PKGBUILD
├── sigilvm-desktop.PKGBUILD
└── sigilvm-extra.PKGBUILD
```

---
## 3. Package Responsibilities

### sigilvm-core

Provides all essential components of the ecosystem.

Includes:
- `libsigilvm.so`
- `sigilvm-tools`
- Shared C++ headers (installed under `/usr/include/sigil/`)


### sigilvm-desktop

Contains desktop integration and theme resources.

Includes:
- YAML theme definitions  
- Theme templates (Hyprland, Waybar, Mako, Kvantum, etc.)
- Hyprland templates  
- Waybar modules, scripts, styles  
- Shell and Mako templates  
- Kvantum theme files  
- Additional integration scripts

This package allows SigilVM to act as a desktop-level theme engine and configuration manager.

### sigilvm-extra

Contains tools not strictly required by the ecosystem.

Includes:
- `sigilvm-editor` (GUI editor frontend)
- `sigilvm-xorit`
- Additional assets and shaders
- Sample or optional user content

This package is optional for users who only need the core VM and desktop components.

---
## 4. Package File Placement

Each package installs its files under predictable paths:

### Binaries

```
/usr/bin/sigilvm-tools
/usr/bin/sigilvm-editor
/usr/bin/sigilvm-xorit
/usr/bin/sigilvm-player
```

### Libraries

```
/usr/lib/sigilvm/libsigilvm.so
```

### Themes & Templates

```
/usr/share/sigilvm/themes/*.yaml
/usr/share/sigilvm/themes/templates/
```

### Documentation (optional)

```
/usr/share/doc/sigilvm/*
```

---
## 5. Build Artifacts

The build system outputs its results to:

```
build/bin/
build/lib/
build/package-name-x86_64
```

CMake’s intermediate files live entirely within:

```
.cache/
```

Neither of these directories are part of the installation footprint.

---

## 6. PKGBUILD Workflow

### Build

```bash
./scripts/build.sh --release
```

### Install

```bash
sudo pacman -U build/sigilvm-core-*.pkg.tar.zst
sudo pacman -U build/sigilvm-desktop-*.pkg.tar.zst
sudo pacman -U build/sigilvm-extra-*.pkg.tar.zst
```

---

## 7. Packaging Scripts

The repository includes simple helper scripts under `scripts/`:

```
scripts/package.py
scripts/build.sh
scripts/clean.sh
```

Package scripts handle:

* Preparing build directories
* Invoking CMake
* Installing artifacts into `pkg/` directory structures
* Creating `.pkg.tar.zst` archives

---

## 8. Validation

After installation, packaging correctness can be checked with:

```bash
sigilvm-tools probe
```

Or specific probes like:

```bash
sigilvm-tools probe gpu
```

These commands reveal:

* Whether templates were installed
* What capabilities are advertised by the VM
* Which system components are active

Missing packages or partial installs are reported clearly.

---

## 9. Summary

* SigilVM uses **three packages** to structure the ecosystem.
* `sigilvm-core` provides foundational functionality.
* `sigilvm-desktop` brings desktop integration and templates.
* `sigilvm-extra` supplies the editor and optional tools.
* Installation and removal are always performed through a package manager.
* Packaging scripts simplify the build → package → install loop.

