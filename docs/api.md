# API & Coding Guidelines

This document provides an overview of the core API architecture in SigilVM,
explains context usage, and shares key coding guidelines.
For development and contributions, this should be considered a primary reference.

---

## 1. Core Concepts

SigilVM is structured around **modular contexts**. 
No global state exists; instead, each process maintains local state derived from advertised capabilities.

### Contexts

| Context   | Role |
|-----------|------|
| `vm`      | Core runtime, required for ecosystem participation |
| `media`   | Audio/video, input, MPRIS, playlist, window management |
| `network` | Socket management, server/client handling |
| `realm`   | Simulation core, ECS-based; can spawn multiple realm instances |
| `render`  | Vulkan-based renderer; attached to a context instance for drawing |
| `util`    | Stateless helper library (math, dice, fs, timer, project-manager) |

### App Descriptor

Each `main()` or executable has an **app descriptor** that contains:

- Process metadata  
- Domain ID (immutable)  
- Capabilities for other contexts

This descriptor is passed to context constructors, ensuring deterministic initialization.

---

## 2. Context Interactions

- Each context can operate independently.  
- Cooperation between contexts is explicit via IPC.  
- The VM context is the primary enabler: without it, other contexts are still usable but cannot fully integrate.  

---

## 3. RAII & Resource Management

- SigilVM uses **RAII patterns** to manage resources.  
- Each context owns its resources exclusively and cleans up on destruction.  
- Example: `sigil::vm::context_t` manages IPC connections and runtime flags.

---

## 4. Coding Style

- **Explicit memory management**; smart pointers are avoided.
- **Deterministic design**: all resources must have a clear lifecycle.
- **Modular code**: avoid hidden dependencies; use context interfaces.
- **Subsystem separation**: VM, platform, media, network, realm, and render should remain independent but interoperable.

---

## 5. Error Handling

- Use `status_t` and bitwise flag-based error codes.
- Avoid exceptions for cross-context signaling; errors propagate via explicit return values.
- Logs via `sigil::vm::log` for runtime inspection.

---

## 6. Header Organization

- Public headers live in `include/sigil/`
- Subdirectory structure mirrors contexts:

```

include/sigil/vm/
include/sigil/media/
include/sigil/network/
include/sigil/realm/
include/sigil/render/
include/sigil/util/

```

---

## 7. API Highlights

### VM Context (`sigil::vm::context_t`)

- Primary gateway to all subsystems
- Supports IPC, logging, parsing, and runtime versioning

### Media Context (`sigil::media::context_t`)

- Playback, input, mixer, MPRIS
- Playlist management

### Render Context (`sigil::render::context_t`)

- Vulkan initialization
- Scene creation, instance attachment, drawing

### Realm Context (`sigil::realm::context_t`)

- ECS simulation
- Start/stop simulation per instance

### Utilities (`sigil::util`)

- `dice`, `math`, `fs`, `rune`, `timer`, `vkutils`
- Stateless helpers, no context required

---

## 8. Contribution & API Usage

- Always respect **context independence**.
- Avoid accessing global resources directly.
- Interactions between subsystems must use clearly defined IPC channels.
- Follow naming conventions in headers and source files.

---

For detailed contribution rules and coding examples, see:  
**[`docs/contributing.md`](contributing.md)**

For application-level usage, see:  
**[`docs/apps.md`](apps.md)**

For installation paths and template management, see:  
**[`docs/install.md`](install.md)**
