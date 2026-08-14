#include "flecs_explorer/panel/panel_manager.h"

#include <stdlib.h>
#include <string.h>

#include "panel/panel_input.h"
#include "panel/panel_signals.h"
#include "panel/panel_window.h"

/**
 * Concrete PanelManager structure maintaining active UI layout and terminal
 * state.
 */
struct PanelManager {
    PanelConfig config;
    PanelLayout layout;
    PanelWindows *windows;
    PanelId focusedPanel;
    bool terminalInitialized;
};

/**
 * Allocates and initializes a new PanelManager instance.
 *
 * Computes initial layout geometry and allocates pane subwindow tracking
 * structures.
 *
 * @param config Pointer to configuration struct.
 * @param outManager Receives allocated PanelManager pointer.
 * @return APP_OK on success, APP_ERROR_INVALID_ARGUMENT or OUT_OF_MEMORY on
 * failure.
 */
AppResult PanelManagerCreate(const PanelConfig *config,
                             PanelManager **outManager)
{
    if (config == NULL || outManager == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    PanelManager *manager = (PanelManager *)calloc(1, sizeof(PanelManager));
    if (manager == NULL) {
        return APP_ERROR_OUT_OF_MEMORY;
    }

    manager->config = *config;
    manager->focusedPanel = PANEL_ENTITIES;
    manager->terminalInitialized = false;

    // Use current screen dimensions or configured defaults
    int initialWidth = config->minWidth;
    int initialHeight = config->minHeight;

    if (COLS > 0 && LINES > 0) {
        initialWidth = COLS;
        initialHeight = LINES;
    }

    PanelLayoutCompute(initialWidth, initialHeight, config->entityPanelRatio,
                       config->minWidth, config->minHeight, &manager->layout);

    const AppResult winRes =
        PanelWindowsCreate(&manager->layout, &manager->windows);
    if (winRes != APP_OK) {
        free(manager);
        return winRes;
    }

    *outManager = manager;
    return APP_OK;
}

/**
 * Sets up terminal modes (cbreak, noecho, keypad, curs_set, mouse reporting).
 *
 * Initializes curses theme palette and installs POSIX signal handlers.
 *
 * @param manager PanelManager instance.
 * @return APP_OK on success, APP_ERROR_INVALID_ARGUMENT on NULL input.
 */
AppResult PanelManagerInitTerminal(PanelManager *manager)
{
    if (manager == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (manager->config.enableMouse) {
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    }

    PanelThemeInit(manager->config.enableColors);
    PanelSignalsInstall();

    int screenWidth = 0;
    int screenHeight = 0;
    getmaxyx(stdscr, screenHeight, screenWidth);

    PanelLayoutCompute(
        screenWidth, screenHeight, manager->config.entityPanelRatio,
        manager->config.minWidth, manager->config.minHeight, &manager->layout);

    PanelWindowsResize(manager->windows, &manager->layout);

    manager->terminalInitialized = true;
    return APP_OK;
}

/**
 * Restores terminal modes, cursor visibility, and releases curses state
 * cleanly.
 *
 * Idempotent and safe to invoke multiple times or from signal handlers.
 *
 * @param manager PanelManager instance. Can be NULL.
 */
void PanelManagerRestoreTerminal(PanelManager *manager)
{
    if (manager != NULL && manager->terminalInitialized) {
        curs_set(1);
        echo();
        nocbreak();
        mousemask(0, NULL);
        endwin();
        manager->terminalInitialized = false;
    }

    PanelSignalsRestore();
}

/**
 * Frees all windows, layout memory, and the PanelManager instance.
 *
 * @param manager PanelManager instance to destroy. Can be NULL.
 */
void PanelManagerDestroy(PanelManager *manager)
{
    if (manager == NULL) {
        return;
    }

    PanelManagerRestoreTerminal(manager);

    if (manager->windows != NULL) {
        PanelWindowsDestroy(manager->windows);
        manager->windows = NULL;
    }

    free(manager);
}

/**
 * Recalculates layout and reallocates pane WINDOWs on terminal resize.
 *
 * @param manager PanelManager instance.
 * @return APP_OK on success, APP_ERROR_INVALID_ARGUMENT on bad inputs.
 */
AppResult PanelManagerHandleResize(PanelManager *manager)
{
    if (manager == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    int screenWidth = manager->layout.screenWidth;
    int screenHeight = manager->layout.screenHeight;

    if (stdscr != NULL) {
        getmaxyx(stdscr, screenHeight, screenWidth);
    }

    PanelLayoutCompute(
        screenWidth, screenHeight, manager->config.entityPanelRatio,
        manager->config.minWidth, manager->config.minHeight, &manager->layout);

    return PanelWindowsResize(manager->windows, &manager->layout);
}

/**
 * Decodes raw keyboard / mouse events into structured PanelEvents and updates
 * focus.
 *
 * @param manager PanelManager instance.
 * @param ch Raw character / key code from getch().
 * @param outEvent Receives decoded PanelEvent.
 * @return APP_OK on success, APP_ERROR_INVALID_ARGUMENT on bad inputs.
 */
AppResult PanelManagerProcessInput(PanelManager *manager, int ch,
                                   PanelEvent *outEvent)
{
    if (manager == NULL || outEvent == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    return PanelInputProcess(&manager->layout, &manager->focusedPanel, ch,
                             outEvent);
}

/**
 * Explicitly sets the focused panel.
 *
 * @param manager PanelManager instance.
 * @param panel PanelId to focus.
 */
void PanelManagerSetFocus(PanelManager *manager, PanelId panel)
{
    if (manager == NULL || !PanelInputIsFocusable(panel)) {
        return;
    }

    manager->focusedPanel = panel;
}

/**
 * Returns the currently focused panel ID.
 *
 * @param manager PanelManager instance.
 * @return Currently focused PanelId, or PANEL_COUNT if manager is NULL.
 */
PanelId PanelManagerGetFocus(const PanelManager *manager)
{
    if (manager == NULL) {
        return PANEL_COUNT;
    }
    return manager->focusedPanel;
}

/**
 * Cycles focus forward (Entity Panel -> Inspector Panel -> Entity Panel).
 *
 * @param manager PanelManager instance.
 */
void PanelManagerFocusNext(PanelManager *manager)
{
    if (manager == NULL) {
        return;
    }
    PanelInputFocusNext(&manager->focusedPanel);
}

/**
 * Cycles focus backward (Inspector Panel -> Entity Panel -> Inspector Panel).
 *
 * @param manager PanelManager instance.
 */
void PanelManagerFocusPrev(PanelManager *manager)
{
    if (manager == NULL) {
        return;
    }
    PanelInputFocusPrev(&manager->focusedPanel);
}

/**
 * Retrieves the curses WINDOW handle for a specific panel.
 *
 * @param manager PanelManager instance.
 * @param panel PanelId enum value.
 * @return Pointer to curses WINDOW, or NULL if unavailable.
 */
WINDOW *PanelManagerGetWindow(PanelManager *manager, PanelId panel)
{
    if (manager == NULL) {
        return NULL;
    }
    return PanelWindowsGet(manager->windows, panel);
}

/**
 * Retrieves the bounding rectangle for a specific panel.
 *
 * @param manager PanelManager instance.
 * @param panel PanelId enum value.
 * @return PanelRect bounding box.
 */
PanelRect PanelManagerGetRect(const PanelManager *manager, PanelId panel)
{
    if (manager == NULL) {
        const PanelRect empty = {0, 0, 0, 0};
        return empty;
    }
    return PanelWindowsGetRect(manager->windows, panel);
}

/**
 * Checks if the terminal is below minimum width or height constraints.
 *
 * @param manager PanelManager instance.
 * @return true if terminal is too small; false otherwise.
 */
bool PanelManagerIsTooSmall(const PanelManager *manager)
{
    if (manager == NULL) {
        return true;
    }
    return manager->layout.isTooSmall;
}

/**
 * Retrieves the current layout snapshot.
 *
 * @param manager PanelManager instance.
 * @return Read-only pointer to PanelLayout snapshot, or NULL if manager is
 * NULL.
 */
const PanelLayout *PanelManagerGetLayout(const PanelManager *manager)
{
    if (manager == NULL) {
        return NULL;
    }
    return &manager->layout;
}
