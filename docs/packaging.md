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

---
## 4. Package File Placement

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
/usr/share/sigilvm/themes/colors/*.yaml
/usr/share/sigilvm/themes/templates/
/usr/share/sigilvm/themes/common/
/usr/share/sigilvm/scripts
```

### Documentation (optional)

```
/usr/share/doc/sigilvm/*
```

---
## 5. Build Artifacts

The build system outputs its results to:

```
out/bin/
out/lib/
out/package-name-x86_64
```

CMake’s intermediate files live entirely within:

```
.cache/build
```

Neither of these directories are part of the installation footprint.

---

## 6. PKGBUILD Workflow

```bash
./build.sh --package [core/desktop/extra]
```
