# Applications

---
## `sigilvm-player`

Lightweight audio controller and playback daemon interface.  
Used for previewing theme audio, controlling runtime playback, or integrating with system keybinds.

### Commands

```
sigilvm-player play <file.ogg>
sigilvm-player pause
sigilvm-player reload
sigilvm-player random on
sigilvm-player random off
sigilvm-player repeat none
sigilvm-player repeat one
sigilvm-player repeat all
```

---
## `sigilvm-tools`

Developer and system-maintenance utility suite.

### Theme Commands

```
sigilvm-tools theme show           # list all installed themes
sigilvm-tools theme show <name>    # show details for a specific theme
sigilvm-tools theme set <name>     # activate theme
```

### Theme Generation

```
sigilvm-tools theme build          # only if template directory exists
```

**Dynamic template discovery:**

- If `/usr/share/sigilvm/themes/templates/` exists → feature enabled  
- If not → command gracefully errors out with:  
  *“Theme templates not available on this system.”*

### Probing Commands

```
sigilvm-tools probe                # full system probe
sigilvm-tools probe gpu            # GPUs only
```

### Maintenance Commands

```

sigilvm-tools flush

```

Flush performs:
- Clearing temporary files in `.cache/`
- Cleaning active VM runtime cache
- Forcing a safe and deterministic “soft reset”

---

## `sigilvm-editor`

Main editing environment built on top of Dear ImGui.

Purpose:
- Debugging VM state
- Inspecting assets and runtime logs
- Editing theme data
- Heavy interaction with `sigil::ipc::ctx_t`

Persistent state is saved under:

```

~/.local/share/sigilvm/editor/

```

---

## `sigilvm-media-daemon`

Autonomous background daemon responsible for:

- Audio routing
- MPRIS integration
- Hot-reload of active theme assets
- Responding to player commands

This process typically starts automatically via systemd, but can also be run manually:

```

sigilvm-media-daemon --foreground

```

---

## `sigilvm-player-gui` *(optional)*

A minimal GUI front-end to `sigilvm-player`, provided only on builds with GUI support enabled.

Provides:
- Play/pause
- Seek controls
- Volume
- Random/repeat toggles

---

## `sigilvm-xorit`

Cryptographic / asset-obfuscation helper

```

sigilvm-xorit encode <file>
sigilvm-xorit decode <file>

```

---

## `sigilvm-theme-selector`

Ultra-minimal theme chooser for lightweight desktop environments.

```

sigilvm-theme-selector

```

Displays installed themes and writes the selection into VM config.

---

## Summary Table

| Binary                    | Purpose | Notes |
|---------------------------|---------|-------|
| `sigilvm-player`          | Audio playback controller | Provides theme playback primitives |
| `sigilvm-tools`           | Maintenance, probing, theme mgmt | Dynamic template discovery |
| `sigilvm-editor`          | Main editing/debugging app | ImGui-based |
| `sigilvm-media-daemon`    | Background audio/IPC daemon | Usually system-launched |
| `sigilvm-player-gui`      | Optional GUI frontend | Requires GUI build flags |
| `sigilvm-xorit`           | Asset encoding/decoding | Used internally |
| `sigilvm-theme-selector`  | Minimal theme switcher | Lightweight DE support |

---

For API details, see:  
**[`docs/api.md`](api.md)**

For build/configuration flags, see:  
**[`docs/build.md`](build.md)**

For installation layout and paths, see:  
**[`docs/install.md`](install.md)**

