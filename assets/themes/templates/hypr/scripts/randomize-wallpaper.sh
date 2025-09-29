#!/bin/bash
WALLDIR="$HOME/Pictures/Wallpapers"
SCREENS="eDP-1 eDP-2 HDMI-A-1"
FILE=$(find "$WALLDIR" -type f | shuf -n1)

# set wallpaper
hyprctl hyprpaper preload "$FILE"
for s in $SCREENS; do
    hyprctl hyprpaper wallpaper "$s,$FILE"
done