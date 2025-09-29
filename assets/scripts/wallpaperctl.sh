#!/usr/bin/env bash
set -euo pipefail

# -----------------------------
# Configuration defaults
# -----------------------------
DEFAULT_WALLPAPER="$HOME/.config/hypr/default.jpg"
WALLPAPER_DIR="$HOME/Pictures/Wallpapers"
MONITORS=$(hyprctl monitors | awk '
    /^Monitor / { name=$2 }
    /disabled: true/ { name="" }
    /^$/ && name { print name; name="" }
')

usage() {
    cat <<EOF
Usage: $(basename "$0") [command] [options]

Commands:
  default                 Set the default wallpaper
  random                  Set a random wallpaper from a directory

Options:
  -d, --dir <path>         Wallpaper directory (default: $WALLPAPER_DIR)
  -s, --monitors "A B C"   Space-separated monitor list
  -f, --file <path>        Set a specific wallpaper file directly
  -n, --dry-run            Show what would be done
  -h, --help               Show this help

Examples:
  $(basename "$0") default
  $(basename "$0") random
  $(basename "$0") random --dir ~/Pictures/Alt
  $(basename "$0") --file ~/wallpapers/special.jpg
EOF
}

die() {
    echo "error: $*" >&2
    exit 1
}

run() {
    if [[ "$DRY_RUN" == true ]]; then
        echo "[dry-run] $*"
    else
        "$@"
    fi
}

# -----------------------------
# Defaults
# -----------------------------
COMMAND=""
FILE=""
DRY_RUN=false

# -----------------------------
# Parse arguments
# -----------------------------
while [[ $# -gt 0 ]]; do
    case "$1" in
        default|random)
            COMMAND="$1"
            shift
            ;;
        -f|--file)
            FILE="$2"
            shift 2
            ;;
        -d|--dir)
            WALLPAPER_DIR="$2"
            shift 2
            ;;
        -s|--monitors)
            read -r -a MONITORS <<< "$2"
            shift 2
            ;;
        -n|--dry-run)
            DRY_RUN=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            die "Unknown argument: $1"
            ;;
    esac
done

# -----------------------------
# Determine which wallpaper to set
# -----------------------------
if [[ -n "$FILE" ]]; then
    [[ -f "$FILE" ]] || die "Specified file not found: $FILE"
elif [[ "$COMMAND" == "default" ]]; then
    FILE="$DEFAULT_WALLPAPER"
    [[ -f "$FILE" ]] || die "Default wallpaper not found: $FILE"
elif [[ "$COMMAND" == "random" ]]; then
    [[ -d "$WALLPAPER_DIR" ]] || die "Wallpaper directory not found: $WALLPAPER_DIR"
    FILE=$(find "$WALLPAPER_DIR" -type f | shuf -n1)
    [[ -n "$FILE" ]] || die "No wallpapers found in $WALLPAPER_DIR"
else
    die "No wallpaper specified. Use 'default', 'random', or '--file <path>'."
fi

# -----------------------------
# Apply wallpaper to monitors
# -----------------------------
for monitor in $MONITORS; do
    run hyprctl hyprpaper wallpaper "$monitor,$FILE,fill"
done
