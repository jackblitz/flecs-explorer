#pragma once

/**
 * @file panel_theme.h
 * @brief JetBrains CLion Darcula theme palette and curses color pair setup.
 *
 * Typical usage:
 * @code
 *     if (PanelThemeInit(true) == APP_OK) {
 *         PanelThemeApplyPair(win, PANEL_COLOR_BORDER_ACTIVE);
 *     }
 * @endcode
 *
 * Visual Palette:
 * Implements the iconic JetBrains Darcula dark IDE color palette using RGB
 * definitions when supported by the terminal emulator, with graceful
 * fallbacks for ANSI 256-color and 16-color environments.
 */

#include <stdbool.h>
#include <curses.h>

#include "flecs_explorer/common/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Semantic color pairs modeled after CLion Darcula.
 */
typedef enum PanelColorPair {
    PANEL_COLOR_DEFAULT = 1,       /**< Primary text (#A9B7C6 on #2B2B2B) */
    PANEL_COLOR_CHROME = 2,        /**< Panel chrome (#A9B7C6 on #313335) */
    PANEL_COLOR_BORDER_ACTIVE = 3, /**< Focused border (#FFC66D on #2B2B2B) */
    PANEL_COLOR_BORDER_INACTIVE =
        4,                         /**< Unfocused border (#808080 on #2B2B2B) */
    PANEL_COLOR_SELECTION = 5,     /**< Highlight (#FFFFFF on #214283) */
    PANEL_COLOR_STATUS_ONLINE = 6, /**< Online status (#6A8759 on #313335) */
    PANEL_COLOR_STATUS_WARN = 7,   /**< Reconnecting (#CC7832 on #313335) */
    PANEL_COLOR_STATUS_ERROR = 8,  /**< Offline error (#BC3F3C on #313335) */
    PANEL_COLOR_NUMERIC = 9,       /**< Numerical data (#6897BB on #2B2B2B) */
    PANEL_COLOR_KEYWORD = 10       /**< Shortcuts & tags (#CC7832 on #313335) */
} PanelColorPair;

/**
 * @brief Initializes curses colors and color pairs matching Darcula theme.
 *
 * If terminal supports custom colors (can_change_color()), exact Darcula RGB
 * values are mapped. Otherwise, maps to closest 256-color / 16-color ANSI
 * fallbacks.
 *
 * @param enableColors If true, initializes curses colors and pairs.
 * @return APP_OK on success, APP_ERROR_SYSTEM if colors not supported.
 */
AppResult PanelThemeInit(bool enableColors);

/**
 * @brief Applies the specified color pair attribute to a curses WINDOW.
 *
 * @param win Target curses WINDOW pointer. Non-NULL.
 * @param pair Semantic color pair identifier.
 */
void PanelThemeApplyPair(WINDOW *win, PanelColorPair pair);

#ifdef __cplusplus
}
#endif
