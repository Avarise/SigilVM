# SigilVM Themes

SigilVM themes define the visual appearance and layout behavior of the desktop environment.  
They are **not single static files**, but *compositions* built from multiple template layers and assets.

Theme construction, deployment, and installation are handled by the **`sigilvm-tools`** binary.

---

## Theme Architecture

A SigilVM theme is composed of **two primary dimensions**:

1. **Color palette**  
2. **Structural template (“shape”)**

These are combined during the build process into a finalized, installable theme.

---

## Theme Template Layout

Theme templates live inside the SigilVM assets directory and follow this structure:

```

assets/themes
├── colors
│   ├── focus.yaml
│   ├── love.yaml
│   ├── mania.yaml
│   ├── matrix.yaml
│   ├── night.yaml
│   └── phoenix.yaml
├── common
└── templates
    ├── sharp
    └── soft

```

> The tree above is **pruned** for clarity.

### `colors/`

Contains **color palette definitions**, stored as `.yaml` files.

Each file represents a full color scheme, defining:
- Accent colors
- Backgrounds
- Foregrounds
- Focus/active states
- Semantic colors used across the system

Color palettes are **purely declarative** and contain no layout or behavioral logic.

---

### `templates/`

Defines the **structural shape** of a theme.

Each directory (e.g. `sharp`, `soft`) represents a different visual philosophy and layout style, such as:
- Bar placement
- Window rounding
- Padding and spacing
- Animation preferences
- UI density and proportions

Structural templates decide *how* the UI is shaped, not *what colors* it uses.

---

### `common/`

Currently serves as a **shared configuration pool**.

This directory contains:
- Universal configuration fragments
- Files reused across multiple templates
- Cross-theme defaults

It is intentionally flexible and may be reorganized as the theming system evolves.

---

## Scripts Integration

In addition to templates, **scripts** are part of the final theme build.

All SigilVM-related scripts are stored under:

```

assets/scripts

```

During the build process:
- Relevant scripts are selected
- Integrated into the theme output
- Deployed alongside configuration files

This allows themes to influence not only appearance, but also runtime behavior.

---

## Build & Deployment

The `sigilvm-tools` binary is responsible for:

- Combining **color palettes** and **structural templates**
- Merging common configuration and scripts
- Producing a finalized theme directory

### Output Location

Built themes are deployed to:

```

~/.local/share/sigilvm/themes/[shape]-[color]

```

Example:

```

~/.local/share/sigilvm/themes/soft-night
~/.local/share/sigilvm/themes/sharp-matrix

```

Each output directory is a **fully resolved, self-contained theme**.

---

## System-Wide Theme Templates

Theme templates are installed system-wide as part of the desktop package:

```

/usr/share/sigilvm/themes

```

These templates are provided by the **`sigilvm-desktop`** package and serve as the canonical base for theme builds.

User-level builds never modify these files directly.

---

## Distribution

Built themes are also published in the project’s:

- **Downloads**
- **Release artifacts**
