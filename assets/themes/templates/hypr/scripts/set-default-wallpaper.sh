#!/bin/bash
SCREENS="eDP-1 eDP-2 HDMI-A-1"

FILE="~/.config/hypr/default.jpg"

# set wallpaper
hyprctl hyprpaper unload "$FILE"
hyprctl hyprpaper preload "$FILE"

for s in $SCREENS; do
    hyprctl hyprpaper wallpaper "$s,$FILE"
done