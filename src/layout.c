#include "panel/layout.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

struct PanelLayout {
    WINDOW *winTree;
    WINDOW *winInspector;
    WINDOW *winStatus;
    PanelPaneId activePane;
    int termRows;
    int termCols;
};

/**
 * @brief Creates a PanelLayout instance.
 */
PanelResult PanelLayoutCreate(PanelLayout **outLayout)
{
    if (outLayout == NULL) {
        return PANEL_ERROR_INVALID_ARGUMENT;
    }

    *outLayout = NULL;

    int rows = 0;
    int cols = 0;
    PanelTerminalSizeQuery(&rows, &cols);

    if (rows < 24 || cols < 80) {
        return PANEL_ERROR_TERMINAL_INIT_FAILED;
    }

    PanelLayout *layout = malloc(sizeof(*layout));
    if (layout == NULL) {
        return PANEL_ERROR_TERMINAL_INIT_FAILED;
    }

    layout->termRows = rows;
    layout->termCols = cols;
    layout->activePane = PANEL_PANE_ENTITY_TREE;

    const int treeCols = cols / 2;
    const int inspectorCols = cols - treeCols;
    const int mainRows = rows - 1;

    layout->winTree = newwin(mainRows, treeCols, 0, 0);
    layout->winInspector = newwin(mainRows, inspectorCols, 0, treeCols);
    layout->winStatus = newwin(1, cols, mainRows, 0);

    if (layout->winTree == NULL || layout->winInspector == NULL ||
        layout->winStatus == NULL) {
        PanelLayoutDestroy(layout);
        return PANEL_ERROR_TERMINAL_INIT_FAILED;
    }

    *outLayout = layout;
    return PANEL_OK;
}

/**
 * @brief Destroys a PanelLayout instance.
 */
void PanelLayoutDestroy(PanelLayout *layout)
{
    if (layout == NULL) {
        return;
    }

    if (layout->winTree != NULL) {
        delwin(layout->winTree);
    }
    if (layout->winInspector != NULL) {
        delwin(layout->winInspector);
    }
    if (layout->winStatus != NULL) {
        delwin(layout->winStatus);
    }

    free(layout);
}

/**
 * @brief Resizes the layout to match the current terminal dimensions.
 */
void PanelLayoutResize(PanelLayout *layout)
{
    if (layout == NULL) {
        return;
    }

    int rows = 0;
    int cols = 0;
    PanelTerminalSizeQuery(&rows, &cols);

    if (rows < 24 || cols < 80) {
        return;
    }

    layout->termRows = rows;
    layout->termCols = cols;

    const int treeCols = cols / 2;
    const int inspectorCols = cols - treeCols;
    const int mainRows = rows - 1;

    if (layout->winTree != NULL) {
        wresize(layout->winTree, mainRows, treeCols);
        mvwin(layout->winTree, 0, 0);
    }

    if (layout->winInspector != NULL) {
        wresize(layout->winInspector, mainRows, inspectorCols);
        mvwin(layout->winInspector, 0, treeCols);
    }

    if (layout->winStatus != NULL) {
        wresize(layout->winStatus, 1, cols);
        mvwin(layout->winStatus, mainRows, 0);
    }
}

/**
 * @brief Cycles focus between panes.
 */
void PanelLayoutFocusCycle(PanelLayout *layout)
{
    if (layout == NULL) {
        return;
    }

    layout->activePane =
        (PanelPaneId)(((int)layout->activePane + 1) % PANEL_PANE_COUNT);
}

/**
 * @brief Gets the active pane.
 */
PanelPaneId PanelLayoutActivePaneGet(const PanelLayout *layout)
{
    if (layout == NULL) {
        return PANEL_PANE_ENTITY_TREE;
    }

    return layout->activePane;
}

/**
 * @brief Renders the borders and headers for the layout.
 */
void PanelLayoutBordersRender(const PanelLayout *layout)
{
    if (layout == NULL) {
        return;
    }

    if (layout->winTree != NULL) {
        wclear(layout->winTree);
        box(layout->winTree, 0, 0);
        mvwprintw(layout->winTree, 0, 2, " Entity Tree %s ",
                  layout->activePane == PANEL_PANE_ENTITY_TREE ? "[ACTIVE]"
                                                               : "");
        wnoutrefresh(layout->winTree);
    }

    if (layout->winInspector != NULL) {
        wclear(layout->winInspector);
        box(layout->winInspector, 0, 0);
        mvwprintw(layout->winInspector, 0, 2, " Component Inspector %s ",
                  layout->activePane == PANEL_PANE_COMPONENT_INSPECTOR
                      ? "[ACTIVE]"
                      : "");
        wnoutrefresh(layout->winInspector);
    }

    if (layout->winStatus != NULL) {
        wclear(layout->winStatus);
        mvwprintw(layout->winStatus, 0, 0,
                  " Status: Ready | Tab: Cycle Focus | q: Quit");
        wnoutrefresh(layout->winStatus);
    }

    doupdate();
}
