#include "panel/terminal.h"
#include <ncurses.h>
#include <locale.h>
#include <signal.h>
#include <stdlib.h>

static void HandleSignal(int sig) {
    (void)sig;
    PanelTerminalCleanup();
    exit(0);
}

const char *PanelResultToString(PanelResult result) {
    switch (result) {
        case PANEL_OK:
            return "PANEL_OK";
        case PANEL_ERROR_TERMINAL_INIT_FAILED:
            return "PANEL_ERROR_TERMINAL_INIT_FAILED";
        case PANEL_ERROR_INVALID_ARGUMENT:
            return "PANEL_ERROR_INVALID_ARGUMENT";
        default:
            return "UNKNOWN_RESULT";
    }
}

PanelResult PanelTerminalInit(void) {
    setlocale(LC_ALL, "");
    if (initscr() == NULL) {
        return PANEL_ERROR_TERMINAL_INIT_FAILED;
    }
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    signal(SIGINT, HandleSignal);
    signal(SIGTERM, HandleSignal);

    return PANEL_OK;
}

void PanelTerminalCleanup(void) {
    endwin();
}

void PanelTerminalSizeQuery(int *outRows, int *outCols) {
    int r = 0, c = 0;
    if (stdscr) {
        getmaxyx(stdscr, r, c);
    }
    if (outRows) {
        *outRows = r;
    }
    if (outCols) {
        *outCols = c;
    }
}
