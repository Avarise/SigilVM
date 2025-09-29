# **SigilVM Overview**

## **What Is SigilVM?**

Blend of three roles into one cohesive platform:

* **Application Framework** — C++ foundations for building tools and interactive software.
* **Media Engine** — Vulkan-based rendering, ECS simulation, media and networking systems.
* **Desktop Layer** — theme engine, player utilities, Hyprland/Waybar integrations, and quality-of-life desktop tooling.

Every component is optional. Applications can run standalone or participate in the larger SigilVM ecosystem through capability-based IPC.

SigilVM is developed as a single repository, typically cloned into:

```
~/Projects/SigilVM/
```


The project uses **CMake** for builds, **bash** wrappers for convenience, and **Arch Linux packages** for distribution.

---
## **Key Features**

* Modular contexts: **vm**, **platform**, **media**, **network**, **realm**, **render**, plus a stateless **util** layer.
* IPC system based on capability advertisement (no global VM state).
* Template-driven desktop theme engine for Hyprland
* Editor for engine development, content creation, and general-purpose text editing.
* Tools for probing hardware, flushing VM state, and managing themes.
* Limited C++ dialect, "Unholy C", focused on achieving C-like simplicity, with convenience of STL and namespaces.

---
## **Repository Overview**

The repository is organized around:

```
.cache/        CMake, Language Server, and other local temp files
apps/          Binaries (editor, tools, player, etc.)
assets/        Theme templates, meshes, shaders, textures
build/         Output binaries, libraries, and packages
cmake/         Components of whole build process.
docs/          In-depth project documentation
extern/        External dependencies outside of those provided by platform
include/       Public headers, delivered in sigilvm-core package
pkg/           Packaging templates for the ./scripts/package.sh
scripts/       Simple bash scripts (build.sh, package.sh)
src/           Source code of all subsystems
tests/         GTest based testing suite
CMakeLists.txt Top level wrapper for cmake/ partials
Readme.md      Basic documentation, placed here with GitHub in mind
```

---
## **Installing SigilVM**

SigilVM currently targets **Arch Linux** as its primary distribution platform.
Three packages are produced:

* **sigilvm-core** – Public headers and libsigilvm.so + sigilvm-tools binary
* **sigilvm-desktop** – Desktop themes, SigilVM Media Player + stack of dependencies to deploy Hyprland based DE
* **sigilvm-extra** – sigilvm-editor, sigilvm-xorit, optional extra assets

Installation methods and paths for each component are explained in detail here:

📄 [`docs/install.md`](./docs/install.md)  
📄 [`docs/packaging.md`](./docs/packaging.md)

---
## **Basic Usage**

### **Theme & System Tools**

```
sigilvm-tools theme show
sigilvm-tools theme show matrix
sigilvm-tools theme set matrix
sigilvm-tools probe
sigilvm-tools probe gpu
sigilvm-tools flush
```

### **Media Player**

```
sigilvm-player play theme.ogg
sigilvm-player pause
sigilvm-player reload
sigilvm-player random on
sigilvm-player random off
sigilvm-player repeat none   # or: one / all
```

All installed binaries and their responsibilities are documented here:

📄 [`docs/apps.md`](./docs/apps.md)

---
## **Building SigilVM**

SigilVM is built with CMake.
You can use the convenience scripts:

```
scripts/build.sh
scripts/package.sh
```

Or configure manually.
All build modes, flags, and platform considerations are documented here:

📄 [`docs/build.md`](./docs/build.md)


---
## **Documentation Index**

| Document                 | Purpose                                                                  |
| ------------------------ | ------------------------------------------------------------------------ |
| **docs/packaging.md**    | Arch packages, contents, responsibilities, and packaging flow.           |
| **docs/install.md**      | Install paths, package behavior, how themes are generated after install. |
| **docs/themes.md**       | Theme structure, theme components and usage instructions.                |
| **docs/build.md**        | Building for all platforms, flags, modes, toolchains.                    |
| **docs/apps.md**         | Full usage of all binaries (player, tools, editor, xorit…).              |
