#!/bin/bash

CONFIG_FILE="$HOME/.config/hypr/toggle-opacity.state"

# Default state if file doesn't exist
if [ ! -f "$CONFIG_FILE" ]; then
  echo "dim" > "$CONFIG_FILE"
fi

STATE=$(cat "$CONFIG_FILE")

if [[ "$STATE" == "dim" ]]; then
  # Switch to opaque
  hyprctl keyword decoration:active_opacity 1.0
  hyprctl keyword decoration:inactive_opacity 1.0
  echo "opaque" > "$CONFIG_FILE"
else
  # Switch to transparent
  hyprctl keyword decoration:active_opacity 0.9
  hyprctl keyword decoration:inactive_opacity 0.7
  echo "dim" > "$CONFIG_FILE"
fi
