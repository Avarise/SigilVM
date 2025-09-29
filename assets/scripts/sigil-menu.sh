#!/usr/bin/env bash
# sigilvm-menu.sh : Two-layer FZF launcher for SigilVM utilities
# Layer 1: Choose between Launcher or Themes
# Layer 2: If Launcher, run a .exe through Proton/Wine wrapper; if Themes, manage SigilVM themes.

set -euo pipefail

# --- Config ---
PROTON_DIR="$HOME/.steam/steam/compatibilitytools.d"
PREFIX_BASE="/opt/wlx64"
SIGIL_THEMES_DIR="/usr/share/sigilvm/themes"

# --- Source optional FZF colors ---
# shellcheck disable=SC1090
#. ~/.config/waybar/scripts/fzf-colors.sh 2>/dev/null || COLORS=()

fzf_menu() {
	local label="$1"
	shift
	printf '%s\n' "$@" | fzf \
		--border=sharp \
		--border-label=" $label " \
		--height=~100% \
		--reverse \
		--no-input \
		--pointer=">" \
		--highlight-line \
		"${COLORS[@]}"
}

check_pfx_lock() {
	local PREFIX_DIR="$1"
	local LOCK_FILE="$PREFIX_DIR/pfx.lock"
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

launcher_mode() {
	read -rp "Path to .exe file: " APP
	[[ -z "$APP" ]] && exit 0
	local APP_EXE
	APP_EXE=$(realpath "$APP")

	# --- Prefix selection ---
	mkdir -p "$PREFIX_BASE"
	mapfile -t PREFIXES < <(find "$PREFIX_BASE" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort)
	PREFIXES+=("Create new prefix")

	local PCHOICE
	PCHOICE=$(fzf_menu "Select Prefix" "${PREFIXES[@]}")
	if [[ "$PCHOICE" == "Create new prefix" ]]; then
		read -rp "Enter new prefix name: " NEWPREFIX
		PREFIX_DIR="$PREFIX_BASE/$NEWPREFIX"
		mkdir -p "$PREFIX_DIR"
	else
		PREFIX_DIR="$PREFIX_BASE/$PCHOICE"
	fi

	check_pfx_lock "$PREFIX_DIR"

	# --- Proton/Wine selection ---
	mapfile -t PROTONS < <(find "$PROTON_DIR" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort)
	PROTONS+=("Wine (system)")

	local RCHOICE
	RCHOICE=$(fzf_menu "Select Runtime" "${PROTONS[@]}")

	if [[ "$RCHOICE" == "Wine (system)" ]]; then
		echo "Running with Wine in prefix $PREFIX_DIR..."
		WINEPREFIX="$PREFIX_DIR/pfx" wine "$APP_EXE"
	else
		local PROTON_PATH="$PROTON_DIR/$RCHOICE/proton"
		if [[ ! -x "$PROTON_PATH" ]]; then
			echo "Error: '$PROTON_PATH' not found or not executable"
			exit 1
		fi
		export STEAM_COMPAT_CLIENT_INSTALL_PATH="$HOME/.steam/steam"
		export STEAM_COMPAT_DATA_PATH="$PREFIX_DIR"
		mkdir -p "$PREFIX_DIR"
		echo "Running $APP_EXE with $RCHOICE..."
		"$PROTON_PATH" run "$APP_EXE"
	fi
}

themes_mode() {
	[[ ! -d "$SIGIL_THEMES_DIR" ]] && { echo "Themes directory not found."; exit 1; }
	mapfile -t THEMES < <(find "$SIGIL_THEMES_DIR" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort)
	[[ ${#THEMES[@]} -eq 0 ]] && { echo "No themes found."; exit 0; }

	local SELECTED
	SELECTED=$(fzf_menu "Select SigilVM Theme" "${THEMES[@]}")

	if [[ -n "$SELECTED" ]]; then
		echo "Applying theme:  $SELECTED"
		sigilvm-theme-selector "$SELECTED"
		# kvantummanager --set "$SELECTED" 2>/dev/null || true
		# pkill -USR2 waybar 2>/dev/null || true
		# hyprctl reload 2>/dev/null || true
	fi
}

main() {
	local MAIN_OPTS=("SigilVM Launcher" "SigilVM Themes")
	local CHOICE
	CHOICE=$(fzf_menu "SigilVM Menu" "${MAIN_OPTS[@]}")

	case "$CHOICE" in
		"SigilVM Launcher") launcher_mode ;;
		"SigilVM Themes") themes_mode ;;
	esac
}

main
