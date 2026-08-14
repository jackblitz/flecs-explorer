#include "flecs_explorer/panel/panel_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Dedicated renderer context managing drawing pipeline and batching.
 */
struct PanelRenderer {
    int frameIndex;
};

/**
 * Allocates and initializes a new PanelRenderer instance.
 *
 * @param outRenderer Receives pointer to allocated renderer.
 * @return APP_OK on success, APP_ERROR_INVALID_ARGUMENT or OUT_OF_MEMORY on
 * failure.
 */
AppResult PanelRendererCreate(PanelRenderer **outRenderer)
{
    if (outRenderer == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    PanelRenderer *renderer = (PanelRenderer *)calloc(1, sizeof(PanelRenderer));
    if (renderer == NULL) {
        return APP_ERROR_OUT_OF_MEMORY;
    }

    *outRenderer = renderer;
    return APP_OK;
}

/**
 * Prepares the screen and virtual buffers for a new rendering frame.
 *
 * @param renderer PanelRenderer instance.
 * @return APP_OK on success.
 */
AppResult PanelRendererBeginFrame(PanelRenderer *renderer)
{
    if (renderer == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    renderer->frameIndex++;
    return APP_OK;
}

/**
 * Queues a curses window's virtual buffer to be drawn on the next frame.
 *
 * @param renderer PanelRenderer instance.
 * @param win Target curses WINDOW.
 */
void PanelRendererFlushWindow(PanelRenderer *renderer, WINDOW *win)
{
    (void)renderer;
    if (win != NULL) {
        wnoutrefresh(win);
    }
}

/**
 * Commits all queued window buffer updates atomically (doupdate).
 *
 * @param renderer PanelRenderer instance.
 * @return APP_OK on success.
 */
AppResult PanelRendererEndFrame(PanelRenderer *renderer)
{
    if (renderer == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    doupdate();
    return APP_OK;
}

/**
 * Draws a styled panel border with title and active/inactive focus attributes.
 *
 * Uses box() and mvwprintw() on the top border. Batches update with
 * wnoutrefresh().
 *
 * @param renderer PanelRenderer instance.
 * @param win Target curses WINDOW.
 * @param title Panel title string. Can be NULL.
 * @param isFocused true if border should render with active focus styling.
 */
void PanelRendererDrawBorder(PanelRenderer *renderer, WINDOW *win,
                             const char *title, bool isFocused)
{
    (void)renderer;
    if (win == NULL) {
        return;
    }

    if (isFocused) {
        wattron(win, COLOR_PAIR(PANEL_COLOR_BORDER_ACTIVE) | A_BOLD);
    } else {
        wattron(win, COLOR_PAIR(PANEL_COLOR_BORDER_INACTIVE));
    }

    box(win, 0, 0);

    if (title != NULL && strlen(title) > 0) {
        mvwprintw(win, 0, 2, " %s ", title);
    }

    if (isFocused) {
        wattroff(win, COLOR_PAIR(PANEL_COLOR_BORDER_ACTIVE) | A_BOLD);
    } else {
        wattroff(win, COLOR_PAIR(PANEL_COLOR_BORDER_INACTIVE));
    }
}

/**
 * Renders styled text at specified local coordinates within a window.
 *
 * @param renderer PanelRenderer instance.
 * @param win Target curses WINDOW.
 * @param y Local row index inside window.
 * @param x Local column index inside window.
 * @param text Formatted string to print.
 * @param color Semantic color pair to apply.
 */
void PanelRendererDrawText(PanelRenderer *renderer, WINDOW *win, int y, int x,
                           const char *text, PanelColorPair color)
{
    (void)renderer;
    if (win == NULL || text == NULL) {
        return;
    }

    wattron(win, COLOR_PAIR(color));
    mvwprintw(win, y, x, "%s", text);
    wattroff(win, COLOR_PAIR(color));
}

/**
 * Renders a centered warning box when the terminal is below minimum size.
 *
 * @param renderer PanelRenderer instance.
 * @param screenWidth Current screen width in columns.
 * @param screenHeight Current screen height in rows.
 * @param minWidth Required minimum width.
 * @param minHeight Required minimum height.
 */
void PanelRendererDrawTooSmallWarning(PanelRenderer *renderer, int screenWidth,
                                      int screenHeight, int minWidth,
                                      int minHeight)
{
    (void)renderer;
    if (stdscr == NULL) {
        return;
    }

    erase();

    char msg[128];
    snprintf(msg, sizeof(msg),
             "Terminal size too small (%dx%d). Required: %dx%d", screenWidth,
             screenHeight, minWidth, minHeight);

    const int startY = screenHeight / 2;
    int startX = (screenWidth - (int)strlen(msg)) / 2;
    if (startX < 0) {
        startX = 0;
    }

    attron(COLOR_PAIR(PANEL_COLOR_STATUS_ERROR) | A_BOLD);
    mvprintw(startY, startX, "%s", msg);
    attroff(COLOR_PAIR(PANEL_COLOR_STATUS_ERROR) | A_BOLD);

    wnoutrefresh(stdscr);
}

/**
 * Clears a window canvas and resets background attributes.
 *
 * @param renderer PanelRenderer instance.
 * @param win Target curses WINDOW.
 */
void PanelRendererClearWindow(PanelRenderer *renderer, WINDOW *win)
{
    (void)renderer;
    if (win != NULL) {
        werase(win);
    }
}

/**
 * Destroys the renderer and releases internal drawing resources.
 *
 * @param renderer PanelRenderer instance to free.
 */
void PanelRendererDestroy(PanelRenderer *renderer)
{
    if (renderer != NULL) {
        free(renderer);
    }
}
