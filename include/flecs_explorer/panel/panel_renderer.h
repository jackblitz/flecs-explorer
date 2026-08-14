#pragma once

/**
 * @file panel_renderer.h
 * @brief Dedicated rendering engine for panels, borders, widgets, and text.
 *
 * Typical usage:
 * @code
 *     PanelRenderer *renderer = NULL;
 *     if (PanelRendererCreate(&renderer) == APP_OK) {
 *         PanelRendererBeginFrame(renderer);
 *
 *         WINDOW *win = PanelManagerGetWindow(manager, PANEL_ENTITIES);
 *         PanelRendererDrawBorder(renderer, win, "Entities", true);
 *         PanelRendererDrawText(renderer, win, 1, 2, "Player",
 * PANEL_COLOR_DEFAULT);
 *
 *         PanelRendererEndFrame(renderer);
 *         PanelRendererDestroy(renderer);
 *     }
 * @endcode
 *
 * Architecture & Extensibility:
 * Isolates all visual drawing, styling, and terminal buffer updates from
 * layout and window management. Future UI components (buttons, tree views,
 * tables) submit render commands through this renderer.
 */

#include <stdbool.h>
#include <curses.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_theme.h"
#include "flecs_explorer/panel/panel_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle representing the dedicated UI Renderer subsystem.
 */
typedef struct PanelRenderer PanelRenderer;

/**
 * @brief Allocates and initializes a new PanelRenderer instance.
 *
 * @param[out] outRenderer Receives pointer to allocated renderer.
 * @return APP_OK on success, error code otherwise.
 */
AppResult PanelRendererCreate(PanelRenderer **outRenderer);

/**
 * @brief Prepares the screen and virtual buffers for a new rendering frame.
 *
 * @param renderer PanelRenderer instance. Non-NULL.
 * @return APP_OK on success.
 */
AppResult PanelRendererBeginFrame(PanelRenderer *renderer);

/**
 * @brief Queues a curses window's virtual buffer to be drawn on the next frame
 * (wnoutrefresh).
 *
 * @param renderer PanelRenderer instance. Non-NULL.
 * @param win Target curses WINDOW. Non-NULL.
 */
void PanelRendererFlushWindow(PanelRenderer *renderer, WINDOW *win);

/**
 * @brief Commits all queued window buffer updates atomically (doupdate).
 *
 * @param renderer PanelRenderer instance. Non-NULL.
 * @return APP_OK on success.
 */
AppResult PanelRendererEndFrame(PanelRenderer *renderer);

/**
 * @brief Draws a styled panel border with title and active/inactive focus
 * attributes.
 *
 * @param renderer PanelRenderer instance. Non-NULL.
 * @param win Target curses WINDOW. Non-NULL.
 * @param title Panel title string displayed on top border. Can be NULL.
 * @param isFocused true if border should render with active focus styling.
 */
void PanelRendererDrawBorder(PanelRenderer *renderer, WINDOW *win,
                             const char *title, bool isFocused);

/**
 * @brief Renders styled text at specified local coordinates within a window.
 *
 * @param renderer PanelRenderer instance. Non-NULL.
 * @param win Target curses WINDOW. Non-NULL.
 * @param y Local row index inside window.
 * @param x Local column index inside window.
 * @param text Formatted string to print. Non-NULL.
 * @param color Semantic color pair to apply.
 */
void PanelRendererDrawText(PanelRenderer *renderer, WINDOW *win, int y, int x,
                           const char *text, PanelColorPair color);

/**
 * @brief Renders a centered warning box when the terminal is below minimum
 * size.
 *
 * @param renderer PanelRenderer instance. Non-NULL.
 * @param screenWidth Current screen width in columns.
 * @param screenHeight Current screen height in rows.
 * @param minWidth Required minimum width.
 * @param minHeight Required minimum height.
 */
void PanelRendererDrawTooSmallWarning(PanelRenderer *renderer, int screenWidth,
                                      int screenHeight, int minWidth,
                                      int minHeight);

/**
 * @brief Clears a window canvas and resets background attributes.
 *
 * @param renderer PanelRenderer instance. Non-NULL.
 * @param win Target curses WINDOW. Non-NULL.
 */
void PanelRendererClearWindow(PanelRenderer *renderer, WINDOW *win);

/**
 * @brief Destroys the renderer and releases internal drawing resources.
 *
 * @param renderer PanelRenderer instance to free. Can be NULL.
 */
void PanelRendererDestroy(PanelRenderer *renderer);

#ifdef __cplusplus
}
#endif
