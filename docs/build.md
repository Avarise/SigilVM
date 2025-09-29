# Build Guide

This document describes the full build process for SigilVM, including build modes,
platform notes, toolchain details, directory structure, and troubleshooting.

---

## 1. Overview

SigilVM is built with CMake.  
All intermediate files are generated into `.cache/build`.  
Final binaries, libraries, and packages are placed into `out/`.

Directory roles:

```
.cache/build  → CMake artifacts, temporary state
out/bin/      → compiled executables
out/lib/      → compiled libraries (including libsigilvm.so)
out/pkg/      → generated package contents
```

---

## 2. System Requirements

### Required

- C++20-capable compiler (Clang or GCC)
- CMake ≥ 3.16
- Make
- Vulkan SDK or system Vulkan headers/libraries
- FFmpeg development libraries
- GLFW (windowing backend)

### Optional (used automatically if found)

- Valgrind  
- cppcheck  
- clang-tidy  

---

## 3. Basic Build (Debug)

Minimal debug build:

```bash
cmake -B .cache/build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build .cache/build
```
---

## 4. Release Build

```bash
cmake -B .cache/build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build .cache/build --target all -- -j$(nproc)
```

This is the mode used for packaging and installation.

---

## 5. Platform Notes

### Linux (primary target)

Officially supported.
Recommended compilers: **clang** on Arch Linux.

### Windows

Not officially supported yet. IPC and media subsystems may require additional ports.

### MacOS

Not supported. Might be supported in limited scope.

---
