#!/usr/bin/env bash
# scripts/kitty_tty_init.sh
# Spawns a dedicated Kitty window to host curses UI and routes GDB/CLion I/O to it.

TTY_FILE="/tmp/flecs_kitty_tty"
GDB_FILE="/tmp/flecs_gdb_inferior.gdb"

# Cleanup previous instances
pkill -f "flecs_explorer_debug_term" 2>/dev/null || true
rm -f "$TTY_FILE" "$GDB_FILE"

# Open Kitty in background and export its active pts path
kitty --title "flecs_explorer_debug_term" sh -c "tty > '$TTY_FILE'; clear; echo '=== Flecs Explorer Debug Terminal (Kitty) ==='; exec cat" &

# Wait briefly for tty file to be written
for _ in $(seq 1 50); do
    if [ -s "$TTY_FILE" ]; then
        break
    fi
    sleep 0.05
done

if [ -s "$TTY_FILE" ]; then
    KITTY_TTY=$(cat "$TTY_FILE")
    echo "set inferior-tty $KITTY_TTY" > "$GDB_FILE"
    echo "Successfully routed GDB inferior-tty to Kitty at: $KITTY_TTY"
else
    echo "Warning: Kitty TTY not available, using default console."
    touch "$GDB_FILE"
fi
