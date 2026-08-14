#include "panel/panel_window.h"

#include <stdlib.h>
#include <string.h>

/**
 * Internal container holding curses WINDOW pointers and current layout rects.
 */
struct PanelWindows {
    WINDOW *windows[PANEL_COUNT];
    PanelRect rects[PANEL_COUNT];
    bool isTooSmall;
};

/**
 * Extracts the layout rectangle corresponding to a specific PanelId.
 *
 * @param layout Calculated panel layout.
 * @param panel Target PanelId.
 * @return PanelRect bounding box.
 */
static PanelRect GetRectForPanel(const PanelLayout *layout, PanelId panel)
{
    switch (panel) {
    case PANEL_HEADER:
        return layout->headerRect;
    case PANEL_ENTITIES:
        return layout->entitiesRect;
    case PANEL_INSPECTOR:
        return layout->inspectorRect;
    case PANEL_STATUS:
        return layout->statusRect;
    case PANEL_COUNT:
    default: {
        const PanelRect empty = {0, 0, 0, 0};
        return empty;
    }
    }
}

/**
 * Allocates and creates curses subwindows for each panel in the layout.
 *
 * Checks if layout is marked tooSmall; if valid, allocates a curses WINDOW
 * for each panel and enables keypad reporting.
 *
 * @param layout Current computed layout.
 * @param outWindows Receives allocated PanelWindows pointer.
 * @return APP_OK on success, APP_ERROR_INVALID_ARGUMENT or OUT_OF_MEMORY on
 * failure.
 */
AppResult PanelWindowsCreate(const PanelLayout *layout,
                             PanelWindows **outWindows)
{
    if (layout == NULL || outWindows == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    PanelWindows *windows = (PanelWindows *)calloc(1, sizeof(PanelWindows));
    if (windows == NULL) {
        return APP_ERROR_OUT_OF_MEMORY;
    }

    windows->isTooSmall = layout->isTooSmall;

    for (int i = 0; i < PANEL_COUNT; ++i) {
        const PanelRect rect = GetRectForPanel(layout, (PanelId)i);
        windows->rects[i] = rect;

        if (!layout->isTooSmall && rect.width > 0 && rect.height > 0) {
            windows->windows[i] =
                newwin(rect.height, rect.width, rect.y, rect.x);
            if (windows->windows[i] != NULL) {
                keypad(windows->windows[i], TRUE);
            }
        }
    }

    *outWindows = windows;
    return APP_OK;
}

/**
 * Resizes and repositions panel subwindows to match a new layout.
 *
 * If layout transitioned to/from tooSmall state, dynamically creates or deletes
 * curses windows as needed. Otherwise, invokes wresize and mvwin.
 *
 * @param windows PanelWindows instance.
 * @param layout New computed layout.
 * @return APP_OK on success, APP_ERROR_INVALID_ARGUMENT on bad inputs.
 */
AppResult PanelWindowsResize(PanelWindows *windows, const PanelLayout *layout)
{
    if (windows == NULL || layout == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    windows->isTooSmall = layout->isTooSmall;

    for (int i = 0; i < PANEL_COUNT; ++i) {
        const PanelRect newRect = GetRectForPanel(layout, (PanelId)i);
        windows->rects[i] = newRect;

        if (layout->isTooSmall) {
            if (windows->windows[i] != NULL) {
                delwin(windows->windows[i]);
                windows->windows[i] = NULL;
            }
        } else {
            if (windows->windows[i] == NULL) {
                if (newRect.width > 0 && newRect.height > 0) {
                    windows->windows[i] = newwin(newRect.height, newRect.width,
                                                 newRect.y, newRect.x);
                    if (windows->windows[i] != NULL) {
                        keypad(windows->windows[i], TRUE);
                    }
                }
            } else {
                wresize(windows->windows[i], newRect.height, newRect.width);
                mvwin(windows->windows[i], newRect.y, newRect.x);
            }
        }
    }

    return APP_OK;
}

/**
 * Retrieves the curses WINDOW pointer for a specific panel.
 *
 * @param windows PanelWindows instance.
 * @param panel PanelId enum value.
 * @return Pointer to WINDOW, or NULL if unavailable.
 */
WINDOW *PanelWindowsGet(const PanelWindows *windows, PanelId panel)
{
    if (windows == NULL || panel < 0 || panel >= PANEL_COUNT) {
        return NULL;
    }
    return windows->windows[panel];
}

/**
 * Retrieves the bounding rectangle for a specific panel.
 *
 * @param windows PanelWindows instance.
 * @param panel PanelId enum value.
 * @return PanelRect bounding box.
 */
PanelRect PanelWindowsGetRect(const PanelWindows *windows, PanelId panel)
{
    if (windows == NULL || panel < 0 || panel >= PANEL_COUNT) {
        const PanelRect empty = {0, 0, 0, 0};
        return empty;
    }
    return windows->rects[panel];
}

/**
 * Frees all curses subwindows and destroys the PanelWindows instance.
 *
 * Safe no-op if windows is NULL.
 *
 * @param windows PanelWindows instance to destroy.
 */
void PanelWindowsDestroy(PanelWindows *windows)
{
    if (windows == NULL) {
        return;
    }

    for (int i = 0; i < PANEL_COUNT; ++i) {
        if (windows->windows[i] != NULL) {
            delwin(windows->windows[i]);
            windows->windows[i] = NULL;
        }
    }

    free(windows);
}
