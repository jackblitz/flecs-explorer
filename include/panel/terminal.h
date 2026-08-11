#pragma once
#include <stdbool.h>

/**
 * @file terminal.h
 * @brief Terminal Lifecycle & Signal Management.
 *
 * Typical usage:
 * @code
 * PanelResult res = PanelTerminalInit();
 * if (res == PANEL_OK) {
 *     // Use ncurses
 *     PanelTerminalCleanup();
 * }
 * @endcode
 */

typedef enum PanelResult {
    PANEL_OK = 0,
    PANEL_ERROR_TERMINAL_INIT_FAILED,
    PANEL_ERROR_INVALID_ARGUMENT
} PanelResult;

const char *PanelResultToString(PanelResult result);

PanelResult PanelTerminalInit(void);
void PanelTerminalCleanup(void);
void PanelTerminalSizeQuery(int *outRows, int *outCols);

void PanelTerminalDrawLayout(void);
bool PanelTerminalHandleInput(int ch);
