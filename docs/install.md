# Installation Guide

This document explains how SigilVM is installed on the system, where files are placed, how packages are structured, and how theme generation works after installation. It assumes the Arch Linux packaging workflow but remains relevant for all platforms.

---

## 1. Installation Philosophy

SigilVM does **not** install directly from the source tree.  
Installation must be done **exclusively through the package manager**.

Reasons:

- Keeps system paths clean and predictable  
- Ensures templates, themes, and binaries are always synchronized  
- Guarantees compatibility between subsystems  
- Allows `sigilvm-tools` to reliably discover templates and system assets

Development builds remain **entirely in the repository**, never touching `/usr/`.

---

## 2. Package Overview

The Arch Linux build produces three packages:

### **sigilvm-core**
Contains:
- `/usr/lib/sigilvm/libsigilvm.so`
- `/usr/bin/sigilvm-tools`  
- Core IPC, VM, util, and realm runtime
- Shared themes directory with YAML definitions  
- Theme templates (Hyprland, Waybar, Mako, Kvantum, etc.)

### **sigilvm-desktop**
Contains:
- Desktop integration files
- Predefined themes and their metadata
- Additional launchers, scripts, and environment files
- Hyprland configuration templates

### **sigilvm-extra**
Contains:
- `/usr/bin/sigilvm-editor`  
- `/usr/bin/sigilvm-xorit`  
- Assets required by those tools  
- Example projects, optional media, and additional shaders  

---

## 3. Installed File Paths

### **Binaries**

```

/usr/bin/sigilvm-tools
/usr/bin/sigilvm-editor
/usr/bin/sigilvm-player
/usr/bin/sigilvm-xorit

```

### **Libraries**

```

/usr/lib/sigilvm/libsigilvm.so

```

### **Themes (static definitions)**

```

/usr/share/sigilvm/themes/*.yaml

```

Example:

```

/usr/share/sigilvm/themes/focus.yaml
/usr/share/sigilvm/themes/matrix.yaml
/usr/share/sigilvm/themes/love.yaml

```

### **Templates (used to generate actual local themes)**

```

/usr/share/sigilvm/themes/templates/

```

Subdirectories:

```

hypr/
waybar/
mako/
kvantum/
shell/
wofi/
alacritty/
...

```

### **User-generated themes**

These are created on demand:

```

~/.local/share/sigilvm/themes/<theme-name>/

````

Each contains the final Hyprland, Waybar, and other files generated from templates.

---

## 4. Installing Packages

### Arch Linux (PKGBUILD-based)

```bash
makepkg -si
````

Or via the helper script:

```bash
scripts/package.py --install
```

Or user-managed:

```bash
sudo pacman -U build/sigilvm-core-*.pkg.tar.zst
sudo pacman -U build/sigilvm-desktop-*.pkg.tar.zst
sudo pacman -U build/sigilvm-extra-*.pkg.tar.zst
```

---

## 5. Post-Install Theme Generation

After installation, **no themes are placed into the user’s home directory** until specifically requested.

To generate a theme (e.g., `matrix`):

```bash
sigilvm-tools theme set matrix
```

This performs:

* Reads `/usr/share/sigilvm/themes/matrix.yaml`
* Loads all templates from `/usr/share/sigilvm/themes/templates/`
* Builds final files into:

```
~/.local/share/sigilvm/themes/matrix/
```

If templates are missing, corrupted, or the user installed only `sigilvm-core` without templates:

`sigilvm-tools` will error out gracefully:

```
ERROR: Theme templates not found in /usr/share/sigilvm/themes/templates
```

---

## 6. Uninstallation

As everything is package-managed:

```bash
sudo pacman -Rns sigilvm-core sigilvm-desktop sigilvm-extra
```

User-generated themes remain:

```
~/.local/share/sigilvm/themes/
```

These can be removed manually.

---

## 7. Summary

* Installation occurs **only via packages**, never via `make install`.
* System-level files go under `/usr/lib/sigilvm` and `/usr/share/sigilvm`.
* Theme definitions are stored globally; user themes are generated locally.
* `sigilvm-tools` handles theme generation and gracefully reports missing templates.

```
