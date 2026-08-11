#include "panel/terminal.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

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
