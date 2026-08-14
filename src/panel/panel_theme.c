#include "flecs_explorer/panel/panel_theme.h"

#include <curses.h>

/**
 * Initializes curses colors and semantic color pairs.
 *
 * Configures the terminal color subsystem using standard curses color pairs
 * and default terminal background transparency (use_default_colors).
 *
 * @param enableColors If true, initializes color subsystem if supported.
 * @return APP_OK on success, APP_ERROR_SYSTEM if terminal cannot support
 * colors.
 */
AppResult PanelThemeInit(bool enableColors)
{
    if (!enableColors) {
        return APP_OK;
    }

    if (!has_colors()) {
        return APP_OK;
    }

    start_color();
    use_default_colors();

    // Standard high-compatibility curses color pairs
    init_pair(PANEL_COLOR_DEFAULT, COLOR_WHITE, -1);
    init_pair(PANEL_COLOR_CHROME, COLOR_CYAN, -1);
    init_pair(PANEL_COLOR_BORDER_ACTIVE, COLOR_YELLOW, -1);
    init_pair(PANEL_COLOR_BORDER_INACTIVE, COLOR_WHITE, -1);
    init_pair(PANEL_COLOR_SELECTION, COLOR_BLACK, COLOR_CYAN);
    init_pair(PANEL_COLOR_STATUS_ONLINE, COLOR_GREEN, -1);
    init_pair(PANEL_COLOR_STATUS_WARN, COLOR_YELLOW, -1);
    init_pair(PANEL_COLOR_STATUS_ERROR, COLOR_RED, -1);
    init_pair(PANEL_COLOR_NUMERIC, COLOR_CYAN, -1);
    init_pair(PANEL_COLOR_KEYWORD, COLOR_YELLOW, -1);

    return APP_OK;
}

/**
 * Applies the specified semantic color pair attribute to a curses WINDOW.
 *
 * Safe no-op if window pointer is NULL or colors are not active.
 *
 * @param win Target curses WINDOW pointer.
 * @param pair Semantic color pair enum value.
 */
void PanelThemeApplyPair(WINDOW *win, PanelColorPair pair)
{
    if (win == NULL) {
        return;
    }

    wattron(win, COLOR_PAIR(pair));
}
