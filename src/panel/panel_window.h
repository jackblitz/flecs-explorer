#pragma once

/**
 * @file panel_window.h
 * @brief Internal subwindow allocation and resize management for curses panels.
 *
 * Typical usage:
 * @code
 *     PanelWindows *windows = NULL;
 *     if (PanelWindowsCreate(&layout, &windows) == APP_OK) {
 *         WINDOW *entWin = PanelWindowsGet(windows, PANEL_ENTITIES);
 *         PanelWindowsResize(windows, &newLayout);
 *         PanelWindowsDestroy(windows);
 *     }
 * @endcode
 *
 * Internal Component:
 * This header is private to the panel subsystem (under src/panel/). It manages
 * curses WINDOW* instances corresponding to each PanelId, abstracting newwin,
 * delwin, wresize, and mvwin calls.
 */

#include <curses.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_layout.h"
#include "flecs_explorer/panel/panel_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque container managing curses subwindows for all panels.
 */
typedef struct PanelWindows PanelWindows;

/**
 * @brief Allocates and creates curses subwindows for each panel in the layout.
 *
 * @param layout Current computed layout. Non-NULL.
 * @param[out] outWindows Receives allocated PanelWindows pointer. Non-NULL.
 * @return APP_OK on success, error code otherwise.
 */
AppResult PanelWindowsCreate(const PanelLayout *layout,
                             PanelWindows **outWindows);

/**
 * @brief Resizes and repositions panel subwindows to match a new layout.
 *
 * @param windows PanelWindows instance. Non-NULL.
 * @param layout New computed layout. Non-NULL.
 * @return APP_OK on success.
 */
AppResult PanelWindowsResize(PanelWindows *windows, const PanelLayout *layout);

/**
 * @brief Retrieves the curses WINDOW pointer for a specific panel.
 *
 * @param windows PanelWindows instance. Can be NULL.
 * @param panel PanelId enum value.
 * @return Pointer to WINDOW, or NULL if unavailable.
 */
WINDOW *PanelWindowsGet(const PanelWindows *windows, PanelId panel);

/**
 * @brief Retrieves the bounding rectangle for a specific panel.
 *
 * @param windows PanelWindows instance. Can be NULL.
 * @param panel PanelId enum value.
 * @return PanelRect bounding box.
 */
PanelRect PanelWindowsGetRect(const PanelWindows *windows, PanelId panel);

/**
 * @brief Frees all curses subwindows and destroys the PanelWindows instance.
 *
 * @param windows PanelWindows instance to destroy. Can be NULL.
 */
void PanelWindowsDestroy(PanelWindows *windows);

#ifdef __cplusplus
}
#endif
