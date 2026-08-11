#include "panel/terminal.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void SpawnExternalTerminal(const char *executablePath) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "alacritty -e %s || kitty %s || konsole -e %s || gnome-terminal -- %s || xterm -e %s &",
        executablePath, executablePath, executablePath, executablePath, executablePath);
    
    int ret = system(cmd);
    (void)ret; // Ignore return value
}

int main(int argc, char **argv) {
    (void)argc;
    // CLion defaults to TERM="dumb" unless "Emulate terminal" is checked.
    // If we are in a dumb terminal, spawn a real terminal window!
    const char *term = getenv("TERM");
    if (!term || strcmp(term, "dumb") == 0) {
        printf("Dumb terminal detected. Automatically spawning external window...\n");
        SpawnExternalTerminal(argv[0]);
        return EXIT_SUCCESS;
    }

    if (PanelTerminalInit() != PANEL_OK) {
        fprintf(stderr, "Failed to initialize terminal.\n");
        return EXIT_FAILURE;
    }

    int ch;
    while (1) {
        PanelTerminalDrawLayout();
        ch = getch();
        if (PanelTerminalHandleInput(ch)) {
            break;
        }
    }

    PanelTerminalCleanup();
    return EXIT_SUCCESS;
}
