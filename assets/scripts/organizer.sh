#!/usr/bin/env sh

set -eu

usage() {
    echo "Usage:"
    echo "  organizer.sh SOURCE [TARGET] (--flatten | --sort) [--bysize KB]"
    exit 1
}

[ "$#" -ge 2 ] || usage

SRC="$1"
shift

MODE=""
BYSIZE_KB=""
TARGET=""

# Parse arguments
while [ "$#" -gt 0 ]; do
    case "$1" in
        --flatten)
            [ -z "$MODE" ] || usage
            MODE="flatten"
            ;;
        --sort)
            [ -z "$MODE" ] || usage
            MODE="sort"
            ;;
        --bysize)
            shift
            [ "$#" -gt 0 ] || usage
            BYSIZE_KB="$1"
            ;;
        *)
            if [ -z "$TARGET" ]; then
                TARGET="$1"
            else
                usage
            fi
            ;;
    esac
    shift
done

[ -n "$MODE" ] || usage

# Default target
if [ -z "$TARGET" ]; then
    TARGET="${SRC}-organized"
fi

mkdir -p "$TARGET"

# Optional size filter
SIZE_EXPR=""
if [ -n "$BYSIZE_KB" ]; then
    MIN_BYTES=$((BYSIZE_KB * 1024))
    SIZE_EXPR="-size +${MIN_BYTES}c"
fi

timestamp="$(date +%s)"

find "$SRC" -type f ${SIZE_EXPR} -print0 |
while IFS= read -r -d '' file; do
    name="$(basename "$file")"

    case "$MODE" in
        flatten)
            dest="$TARGET/$name"

            if [ -e "$dest" ]; then
                dest="$TARGET/organizer-${timestamp}-${name}"
            fi

            mv "$file" "$dest"
            ;;

        sort)
            ext="${name##*.}"

            if [ "$name" = "$ext" ]; then
                ext="noext"
            else
                ext="$(printf '%s' "$ext" | tr 'A-Z' 'a-z')"
            fi

            mkdir -p "$TARGET/$ext"
            mv -n "$file" "$TARGET/$ext/"
            ;;
    esac
done
