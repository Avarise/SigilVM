# SigilVM goals
---
## Part I - SigilVM as application development framework.

  Delivered as SigilVM Core package (sigilvm-core), which provides Linux .so,
  public C++ headers (#include <sigil/common.h>, #include <sigil/common.h>, and other),
  and Linux binary application: /usr/bin/sigilvm-tools

---
## Part II - SigilVM as nuanced UX Layer.

  Centered around Wayland/Hyprland + Waybar, core dependencies for SigilVM desktop Package (sigilvm-desktop).
  This package contains theme templates, auxiliary scripts and small programs for desktop actions,
  such as brightness controlls or GUI theme editor/selector made with sigivm-core library.
  SigilVM is designed with adaptive modularity in mind, components are designed to be standalone,
  but open to communication via known IPC patters (DBUS, xdg-friendly state files).
  From simple scripts, to heavy GUI applications or execution endpoints, together,
  these modules essentially make the "Virtual Machine". Active SigilVM can be accessed via GUIs,
  scripts, shell binaries.

---
## Part III - SigilVM as modern Media layer

  Using GLFW, Vulkan, FFMPEG, ImGui, SigilVM provides feedback with video, audio, GUI's.
  Playing music, watching videos, or managing game library.
  Even developing your own games using SigilVM Engine.
  Programs and assets related to these functions might be found both in sigilvm-desktop package
  and sigilvm-extra package.

---
# Repository Structure
---
## Tracked Files
```
  ./apps/ - .cpp files, entry point translation units aka. main()
  ./assets/ - Meshes, scripts, shaders, theme templates, files used by SigilVM programs
  ./cmake/ - CMake config files, split into few .cmake files
  ./include/ - Contains sigil/ public header, but also few of the used libraries (imgui/ and xxhash/)
  ./pkg/ - PKGBUILD config files, used by ./build.sh
  ./src/ - libsigilvm.so implementation for public headers
  ./src/tests/ - libsigilvm.so implementation of tests, not included in final package, but lives here.
  ./build.sh - Script for repository management, builds, 
  ./CMakeLists.txt - top-level CMake aggregation
  ./Readme.md - This file
```
## Building and Artifacts

  Running ./build.sh creates ./out/ and ./.cache/ directories, that will contain built binaries and libsigilvm.so.
  Running ./build.sh --test will run test suite
  Running ./build.sh --package core/desktop/extra/all will build, test, and create packages in ./out/
  ./.cache/build is used by CMake and will be remade each time the script runs.

  Install package with # pacman -U sigilvm-package-name.pkg.tar.zst


  ### Paths getters and ensures we need
  1. All the basics, config, cache, data, state. Base variants that provide top level directory, as well as additional calls, with reference to app_desctiptor_t. Underneath they use the base variants, with additional app name based path eval.
  2. Get compdata directory, for storing of wine/proton prefixes. Many apps may use them, no single app should own those -> .local/share/sigilvm/compdata
  3. Get/Ensure of the user-facing basics: Documents Downloads Pictures Videos Music Workspace, and ensure all-in-one for these, to easily deploy desktop on new user accounts.
  