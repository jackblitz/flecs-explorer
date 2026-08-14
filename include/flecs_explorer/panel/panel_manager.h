#pragma once

/**
 * @file panel_manager.h
 * @brief Primary facade for panel lifecycle, input dispatching, focus, and
 * rendering.
 *
 * Typical usage:
 * @code
 *     PanelConfig config = {
 *         .minWidth = 80,
 *         .minHeight = 24,
 *         .entityPanelRatio = 0.35,
 *         .enableMouse = true,
 *         .enableColors = true,
 *         .useDarculaTheme = true
 *     };
 *
 *     PanelManager *manager = NULL;
 *     if (PanelManagerCreate(&config, &manager) == APP_OK) {
 *         PanelManagerInitTerminal(manager);
 *
 *         // Main event loop
 *         PanelManagerRender(manager);
 *
 *         PanelManagerRestoreTerminal(manager);
 *         PanelManagerDestroy(manager);
 *     }
 * @endcode
 *
 * Architecture & Ownership:
 * PanelManager acts as the root coordinator for the multi-pane layout,
 * managing window allocations, focus cycling, resize events, and
 * double-buffered screen updates.
 */

#include <curses.h>
#include <stdbool.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_layout.h"
#include "flecs_explorer/panel/panel_renderer.h"
#include "flecs_explorer/panel/panel_theme.h"
#include "flecs_explorer/panel/panel_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocates and initializes a new PanelManager instance.
 *
 * @param config Pointer to configuration struct. Non-NULL.
 * @param[out] outManager Receives pointer to allocated PanelManager.
 * @return APP_OK on success, error code otherwise.
 */
AppResult PanelManagerCreate(const PanelConfig *config,
                             PanelManager **outManager);

/**
 * @brief Sets up terminal (ncurses, cbreak, noecho, keypad, mouse reporting).
 *
 * @param manager PanelManager instance. Non-NULL.
 * @return APP_OK on success, APP_ERROR_SYSTEM on curses failure.
 */
AppResult PanelManagerInitTerminal(PanelManager *manager);

/**
 * @brief Restores terminal modes, cursor visibility, and releases curses state
 * cleanly.
 *
 * Safe to call multiple times and safe to invoke from signal handlers.
 *
 * @param manager PanelManager instance. Can be NULL.
 */
void PanelManagerRestoreTerminal(PanelManager *manager);

/**
 * @brief Frees all windows, layout memory, and the PanelManager instance.
 *
 * @param manager PanelManager instance to destroy. Can be NULL.
 */
void PanelManagerDestroy(PanelManager *manager);

/**
 * @brief Recalculates layout and reallocates pane WINDOWs on terminal resize.
 *
 * @param manager PanelManager instance. Non-NULL.
 * @return APP_OK on success.
 */
AppResult PanelManagerHandleResize(PanelManager *manager);

/**
 * @brief Decodes raw keyboard / mouse events into structured PanelEvents and
 * handles focus.
 *
 * @param manager PanelManager instance. Non-NULL.
 * @param ch Raw character / key code from getch().
 * @param[out] outEvent Receives decoded PanelEvent. Non-NULL.
 * @return APP_OK on success.
 */
AppResult PanelManagerProcessInput(PanelManager *manager, int ch,
                                   PanelEvent *outEvent);

/**
 * @brief Explicitly sets the focused panel.
 *
 * @param manager PanelManager instance. Non-NULL.
 * @param panel PanelId to focus (PANEL_ENTITIES or PANEL_INSPECTOR).
 */
void PanelManagerSetFocus(PanelManager *manager, PanelId panel);

/**
 * @brief Returns the currently focused panel ID.
 *
 * @param manager PanelManager instance. Non-NULL.
 * @return Currently focused PanelId.
 */
PanelId PanelManagerGetFocus(const PanelManager *manager);

/**
 * @brief Cycles focus forward (Entity Panel -> Inspector Panel -> Entity
 * Panel).
 *
 * @param manager PanelManager instance. Non-NULL.
 */
void PanelManagerFocusNext(PanelManager *manager);

/**
 * @brief Cycles focus backward (Inspector Panel -> Entity Panel -> Inspector
 * Panel).
 *
 * @param manager PanelManager instance. Non-NULL.
 */
void PanelManagerFocusPrev(PanelManager *manager);

/**
 * @brief Retrieves the underlying ncurses WINDOW pointer for a specific panel.
 *
 * @param manager PanelManager instance. Non-NULL.
 * @param panel Target PanelId.
 * @return Pointer to WINDOW, or NULL if unavailable or terminal is too small.
 */
WINDOW *PanelManagerGetWindow(PanelManager *manager, PanelId panel);

/**
 * @brief Retrieves the bounding box rectangle for a specific panel.
 *
 * @param manager PanelManager instance. Non-NULL.
 * @param panel Target PanelId.
 * @return PanelRect bounding box.
 */
PanelRect PanelManagerGetRect(const PanelManager *manager, PanelId panel);

/**
 * @brief Returns the current active layout snapshot.
 *
 * @param manager PanelManager instance. Non-NULL.
 * @return Pointer to layout struct.
 */
const PanelLayout *PanelManagerGetLayout(const PanelManager *manager);

/**
 * @brief Returns true if current terminal size is below minimum required
 * resolution.
 *
 * @param manager PanelManager instance. Non-NULL.
 * @return true if width < minWidth or height < minHeight; false otherwise.
 */
bool PanelManagerIsTooSmall(const PanelManager *manager);

/**
 * @brief Checks if the panel manager has pending UI state changes requiring a
 * redraw.
 *
 * @param manager PanelManager instance. Non-NULL.
 * @return true if dirty; false otherwise.
 */
bool PanelManagerIsDirty(const PanelManager *manager);

/**
 * @brief Marks the panel manager as dirty, requesting a redraw on the next
 * frame.
 *
 * @param manager PanelManager instance. Non-NULL.
 */
void PanelManagerMarkDirty(PanelManager *manager);

/**
 * @brief Clears the dirty flag on the panel manager after rendering completes.
 *
 * @param manager PanelManager instance. Non-NULL.
 */
void PanelManagerClearDirty(PanelManager *manager);

#ifdef __cplusplus
}
#endif
