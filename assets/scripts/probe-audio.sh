#!/usr/bin/env sh

# probe-audio.sh
# Pure observation script for modular Arch audio stack
# Never exits with failure. Reports presence only.

print_section() {
    printf "\n=== %s ===\n" "$1"
}

check_bin() {
    if command -v "$1" >/dev/null 2>&1; then
        printf "[YES] binary: %s\n" "$1"
    else
        printf "[NO ] binary: %s\n" "$1"
    fi
}

check_file() {
    if [ -e "$1" ]; then
        printf "[YES] path: %s\n" "$1"
    else
        printf "[NO ] path: %s\n" "$1"
    fi
}

check_user_service() {
    if systemctl --user status "$1" >/dev/null 2>&1; then
        printf "[YES] user service active: %s\n" "$1"
    else
        printf "[NO ] user service active: %s\n" "$1"
    fi
}

check_group_membership() {
    if id -nG "$USER" | grep -qw "$1"; then
        printf "[YES] user in group: %s\n" "$1"
    else
        printf "[NO ] user in group: %s\n" "$1"
    fi
}

print_section "Core Audio Servers"

for bin in pipewire wireplumber pw-top pactl jackd; do
    check_bin "$bin"
done

for svc in pipewire.service wireplumber.service pipewire-pulse.service; do
    check_user_service "$svc"
done

print_section "Routing / Patchbay"

for bin in qpwgraph helvum pw-link; do
    check_bin "$bin"
done

print_section "Plugin Hosts / Racks"

for bin in carla carla-rack jalv; do
    check_bin "$bin"
done

print_section "DAWs"

for bin in ardour ardour6 reaper lmms; do
    check_bin "$bin"
done

print_section "Video / Multimedia Tools"

for bin in ffmpeg ffprobe kdenlive obs; do
    check_bin "$bin"
done

print_section "MIDI / Synth / Instruments"

for bin in fluidsynth zynaddsubfx surge-xt helm aconnect; do
    check_bin "$bin"
done

print_section "Plugin Directories"

for dir in \
    "$HOME/.lv2" \
    "$HOME/.vst3" \
    "$HOME/.vst" \
    "/usr/lib/lv2" \
    "/usr/lib/vst3"
do
    check_file "$dir"
done

print_section "Realtime / Scheduling"

check_group_membership "realtime"
check_group_membership "audio"

check_file "/etc/security/limits.d/99-realtime.conf"

if command -v ulimit >/dev/null 2>&1; then
    printf "[INFO] ulimit -r: %s\n" "$(ulimit -r 2>/dev/null)"
fi

print_section "Kernel / Devices"

check_file "/dev/snd"
check_bin "lsmod"

if command -v lsmod >/dev/null 2>&1; then
    if lsmod | grep -q snd; then
        printf "[YES] kernel module loaded: snd*\n"
    else
        printf "[NO ] kernel module loaded: snd*\n"
    fi
fi

print_section "PipeWire Runtime Info"

if command -v pw-cli >/dev/null 2>&1; then
    if pw-cli info 0 >/dev/null 2>&1; then
        printf "[YES] pipewire runtime reachable\n"
    else
        printf "[NO ] pipewire runtime reachable\n"
    fi
fi

print_section "JACK Compatibility"

if command -v jack_lsp >/dev/null 2>&1; then
    if jack_lsp >/dev/null 2>&1; then
        printf "[YES] JACK graph reachable\n"
    else
        printf "[NO ] JACK graph reachable\n"
    fi
else
    printf "[NO ] binary: jack_lsp\n"
fi

print_section "Environment Variables"

for var in PIPEWIRE_LATENCY JACK_PROMISCUOUS_SERVER; do
    if [ -n "$(eval echo \$$var)" ]; then
        printf "[YES] env set: %s=%s\n" "$var" "$(eval echo \$$var)"
    else
        printf "[NO ] env set: %s\n" "$var"
    fi
done

print_section "User Audio Configuration"

check_file "$HOME/.config/pipewire"
check_file "$HOME/.config/wireplumber"
check_file "$HOME/.config/ardour"

print_section "Summary"

printf "Probe completed. No validation performed.\n"
printf "This script does not interpret conflicts.\n"

exit 0
