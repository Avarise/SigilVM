#!/usr/bin/env bash
# dotexe.sh : Run a Windows .exe through Proton-GE or WINE directly.

set -euo pipefail

PROTON_DIR="$HOME/.steam/steam/compatibilitytools.d"
PREFIX_BASE="$HOME/.local/share/sigilvm/wlx64/prefixes/"

check_pfx_lock() {
    LOCK_FILE="$PREFIX_DIR/pfx.lock"
    if [[ -f "$LOCK_FILE" ]]; then
        echo "Warning: Lock file exists at $LOCK_FILE"
        read -rp "Delete lock file? [y/N]: " RESP
        case "$RESP" in
            [yY]|[yY][eE][sS])
                rm -f "$LOCK_FILE"
                echo "Lock file deleted."
                ;;
            *)
                echo "Keeping lock file. This may cause freezes."
                ;;
        esac
    fi
}


# --- check args ---
if [[ $# -lt 1 ]]; then
    echo "Usage: $0 /path/to/app.exe"
    exit 1
fi
APP_EXE=$(realpath "$1")

# --- prefix selection ---
mkdir -p "$PREFIX_BASE"
mapfile -t PREFIXES < <(find "$PREFIX_BASE" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort)

echo "Select prefix:"
i=1
for p in "${PREFIXES[@]}"; do
    echo "  $i) $p"
    ((i++))
done
echo "  $i) Create new prefix"

read -rp "Enter number [1-$i]: " PCHOICE
if ! [[ "$PCHOICE" =~ ^[0-9]+$ ]] || (( PCHOICE < 1 || PCHOICE > i )); then
    echo "Invalid choice"
    exit 1
fi

if (( PCHOICE == i )); then
    read -rp "Enter new prefix name: " NEWPREFIX
    PREFIX_DIR="$PREFIX_BASE/$NEWPREFIX"
    mkdir -p "$PREFIX_DIR"
else
    PREFIX_DIR="$PREFIX_BASE/${PREFIXES[$((PCHOICE-1))]}"
fi

check_pfx_lock

# --- proton/wine selection ---
mapfile -t PROTONS < <(find "$PROTON_DIR" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort)

echo "Select runtime:"
j=1
for p in "${PROTONS[@]}"; do
    echo "  $j) $p"
    ((j++))
done
echo "  $j) Wine (system)"

read -rp "Enter number [1-$j]: " RCHOICE
if ! [[ "$RCHOICE" =~ ^[0-9]+$ ]] || (( RCHOICE < 1 || RCHOICE > j )); then
    echo "Invalid choice"
    exit 1
fi

# --- run ---
if (( RCHOICE == j )); then
    echo "Running with Wine in prefix $PREFIX_DIR..."
    WINEPREFIX="$PREFIX_DIR/pfx" wine "$APP_EXE"
else
    PROTON_SELECTED="${PROTONS[$((RCHOICE-1))]}"
    PROTON_PATH="$PROTON_DIR/$PROTON_SELECTED/proton"
    if [[ ! -x "$PROTON_PATH" ]]; then
        echo "Error: '$PROTON_PATH' not found or not executable"
        exit 1
    fi
    export STEAM_COMPAT_CLIENT_INSTALL_PATH="$HOME/.steam/steam"
    export STEAM_COMPAT_DATA_PATH="$PREFIX_DIR"
    mkdir -p "$PREFIX_DIR"
    echo "Running $APP_EXE with $PROTON_SELECTED..."
    "$PROTON_PATH" run "$APP_EXE"
fi
