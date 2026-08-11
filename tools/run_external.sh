#!/bin/bash
# run_external.sh: Launch the passed command in a new external terminal emulator.

CMD="$@"

if command -v alacritty >/dev/null 2>&1; then
    alacritty -e $CMD
elif command -v kitty >/dev/null 2>&1; then
    kitty $CMD
elif command -v konsole >/dev/null 2>&1; then
    konsole -e $CMD
elif command -v gnome-terminal >/dev/null 2>&1; then
    gnome-terminal -- $CMD
elif command -v xterm >/dev/null 2>&1; then
    xterm -e $CMD
else
    echo "No external terminal emulator found. Running inline..."
    $CMD
fi
