# SigilVM
  .cache/build      CMake Artifacts
  .cache/staging    Package composition area
  apps/             C++ sources for app entry points
  assets/           Templates, desktop scripts, meshes, textures, shaders, etc
  cmake/            Detailed CMake config directory
  core/             C++ sources for core, stateless and platform agnostic library elements, 
  docs/             Buildable documentation
  include/          Public headers
  modules/          SigilVM modules: highest level of abstraction and orchestration, but without their own entry point, servers, game engine, desktop engine
  out/              Ignored by git, compressed packages, compiled binaries
  pkg/              Instructions and templates for packaging systems and known formats, Linux and Windows
  platform/         C++ sources for platform layer, file system, OS caps, etc
  runtime/          C++ source for SigilVM runtime, execution, IPC, etc
  systems/          Reusable domain focused components, like media, networking, ecs, database, rendering, simulation
  tests/            GTest suite for SigilVM library, and platform specific build test scripts (.sh and .ps1) to be used by build tools
  thirdparty/       External dependencies
  tools/            scripts for building, packaging, testing and doc rendering
  CMakeLists.txt
  Readme.md         This file



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



  ### What I got in code and how can I separate it?

Alright, let me separate various parts of the project into public/private headers and into clear responsibility layers. The concepts covered by SigilVM in no particular order:
Typedefs,
Application identity: set of structs that hold info about argv, argc, envp, provide mechanism to get app name.
App identity also allows aggregation of configs and shared state using xdg paths, like ".config/sigilvm/sigilvm-editor/editor.ini"
Color primitives, color spaces, in-memory representation of images etc.
Math, hashing (based on thirdparty code), mathematical vector definitions used by SigilVM apps
Platform subsystem: some of the functions require allocation of sigil::platform::context_t which is later passed by reference. Unified way to deal with platform, from filesystem, to devices, like audio sources, sinks, GPU interaction, and window creation. Basically all platform specific code translated into one API for other SigilVM apps. 
Renderer: So far Vulkan based, might evolve. Some work is shifted into platform subsys, mostly things about acquiring physical device, or draw surfaces (window_t related concepts)
Media subsystem: requires allocation of sigil::media::context_t which is passed by reference to many calls of the API. Relies of platform subsystem to acquire devices, some interactions with render subsystem to reduce code duplication, and finally, FFMPEG for heavy-lifting.
Network subsystem: In short, all things network, some functions rely on unified platform API, some are purely in-memory representations, like request handlers etc.
"Virtual Machine" subsystem, namesake of the SigilVM: IPC + coordination layer, built on top of "Application Identity", but no limited to it. This is the part that makes it possible to issue simple shell command or run .appimage / .exe file, but instead of starting monolithic memory hog, set of slim services, deamons, preloaders etc runs in unified way, making load times fast. Somewhere close to the VM code, a command parsing headers exists, basically, they allow programmer to register simple commands like {"theme", "set", *handler_function_here*}.

PEU: Process Execution Unit: part of platform unification, replacement for fork/execvp and other similar calls, made for clean and safe execution of shell commands, with strict stdin/stdout fd redirection semantics, etc. Used heavily in application launcher program.

Some random, stateless utils: strings formatters, for example, a function that can take uint32_t of bytes and produce clean strings with automatic generation of kB, MB, GB etc. More utils, like in-memory XORing for simple obfuscation (not real cryptography), time related utils etc.

Simulation engine aka Game Engine. Key concepts here: ECS implementation, albeit, simple in nature at the moment. Implementation of "engines", each engine is basically piece of code that runs with set tick rate or in response to event like new_frame_event. This allows to create program that simulates physics in 1000 Hz, but rendering happens at 144 FPS, while network sync happens 10 times a second. Each engine can bind to other components, and usually has whole CPU thread/core reserved.

Desktop enhancement: theme manager for Hyprland and Waybar, media player app, compatibility wrapper for wine/proton (in short: hash exe file, and use that hash to load compatibility profile which is combination of runner wine/proton + path to wine/proton prefix)

Scripting language for SigilVM, probably with LLVM backend, at the moment, developement of scripting is halted, as it requires the codebase to be mature enough, but we at least want to include it in our concept space early and be aware of it.


in ./apps/ I got pure .cpp files at the moment:
dotexe.cpp (compiles to sigilvm-dotexe): application launcher, aware of path, .desktop entries, and custom system of "profiles" which are essentially .yaml files that hold paths for proton/wine prefix, proton runner or wine binary, file hash (used to determine if given exe already has profile or one needs to be created.)
editor.cpp - heavy lifting GUI app, meant to be game editor, theme editor, asset editor etc.
player.cpp - media player program, music and video, used by game engine to play cutscenes and/or sounds.
tools.cpp - small but really important program, it's responsible for configuration of the SigilVM, can purge/reset active VM, build and deploy themes. This application, is packaged with SigilVM  Headers and libsigilvm.so (or libsigilvm.dll) as "sigilvm-core" package for both windows and linux. Finally, this app is both sanity, compatibility and performance tester/verifier, meant to be the final arbiter and helper when things go wrong for regular or power user.
xorit.cpp - small cryptographic binary util, asset obfuscation.

./include/core
./include/platform
./include/runtime
./include/systems
The packaging system could easily copy these into one directory, so that when sigilvm is installed into system, a developer might use includes like:
#include <sigilvm/core/format.h> and use .so or .dll

then, leave ./src for more detailed implementation, with some .h thrown in there, but not exposed in final api.



// Create particle
    var c := scene.addCircle({
            pos := e.pos;
            radius := 0.03;
            vel := [0, 0];
            density := 30;
            restitution := 0.4;
            friction := 0.2;
            collisionGroup := 1;
            colorHSVA := [rand.uniform * 360, 1, 1, 1];
            timeToLive := 3;
            opaqueBorders := false;
            drawBorder := false;
            showTrail := true;
            trailLength := 600;
            trailFade := true
        })

    // Attach tracer / pen
    c.addPen({
        colorHSVA := c.colorHSVA;
        fadeTime := 1.5;
        maxPoints := 120;
        penSize := 3;
    });
