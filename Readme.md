# **SigilVM Overview**

## **What Is SigilVM?**

Software stack delivered in three packages:
```
SigilVM Core (sigilvm-core.pkg.tar.zst):
  - Shared Library libsigilvm.so
  - Public Headers for C++ app development
  - SigilVM Tools (/usr/bin/sigilvm-tools): primary management binary of the SigilVM ecosystem

SigilVM Desktop (sigilvm-desktop.pkg.tar.zst) - depends on core package
  - Theme templates for building /usr/share/sigilvm/templates
  - Utility scripts: wallpaperctl, volume manager, etc
  - SigilVM Media Player (/usr/bin/sigilvm-player)

SigilVM Extra (sigilvm-extra.pkg.tar.zst) - depends on core package
  - SigilVM Editor (/usr/bin/sigilvm-editor): GUI media application
  - Legacy Themes: pre-built themes

```

---
## File Structure

### Repository

Can be deployed anywhere, self contained, with .cache/ and output/ subdirectories

```
.cache/        CMake, Language Server, and other local temp files
apps/          Src for top level binaries (editor, tools, player, etc.)
assets/        Theme templates, meshes, shaders, textures
cmake/         Components of whole build process.
include/       Public headers as well as external dependencies (ImGui + Thaumaturgy)
out/           Output binaries, libraries, and packages
pkg/           Packaging templates for the ./build.sh --package
src/           Source code of all subsystems
tests/         GTest based testing suite
CMakeLists.txt Top level wrapper for cmake/ partials
Readme.md      Basic documentation, placed here with GitHub in mind
```

### System-wide Installation

Deployed via package installation, contains binaries and assets,
like templates to build themes.

```
/usr/share/sigilvm
```

### Per-user Installation and artifacts

Deployed by sigilvm-tools and other scripts, contains built themes,
temp data, as well as compatibility prefixes.

```
~/.local/share/sigilvm
~/.cache/sigilvm
```

---
## **Basic Usage**

```
1. Initialize and update git modules
2. Build with ./build.sh
3. Install packages with pacman
```

All installed binaries and their responsibilities are documented here:

📄 [`apps/Readme.md`](./apps/Read,e.md)
