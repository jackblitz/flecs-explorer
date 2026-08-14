#pragma once

/**
 * @file panel_input.h
 * @brief Input dispatcher, keycode mapping, focus cycling, and mouse event
 * decoder.
 *
 * Typical usage:
 * @code
 *     PanelEvent event;
 *     PanelId focusedPanel = PANEL_ENTITIES;
 *     int ch = getch();
 *     if (PanelInputProcess(&layout, &focusedPanel, ch, &event) == APP_OK) {
 *         // Handle event (event.type, event.targetPanel, etc.)
 *     }
 * @endcode
 *
 * Internal Component:
 * Private to the panel subsystem (under src/panel/). Translates raw curses
 * keypresses and mouse MEVENT structures into high-level PanelEvent payloads,
 * updating active panel focus when Tab, Shift-Tab, or click events occur.
 */

#include <stdbool.h>
#include <curses.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_layout.h"
#include "flecs_explorer/panel/panel_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Checks if a panel is capable of receiving keyboard/mouse focus.
 *
 * Header and Status footer are non-focusable; Left Entity Browser and
 * Right Component Inspector are focusable.
 *
 * @param panel Target PanelId.
 * @return true if panel is focusable; false otherwise.
 */
bool PanelInputIsFocusable(PanelId panel);

/**
 * @brief Cycles focus forward to the next focusable panel.
 *
 * @param inOutFocus Pointer to current focused PanelId. Updated in place.
 */
void PanelInputFocusNext(PanelId *inOutFocus);

/**
 * @brief Cycles focus backward to the previous focusable panel.
 *
 * @param inOutFocus Pointer to current focused PanelId. Updated in place.
 */
void PanelInputFocusPrev(PanelId *inOutFocus);

/**
 * @brief Decodes a raw curses character / key code or mouse event into a
 * PanelEvent.
 *
 * @param layout Current computed layout. Non-NULL.
 * @param inOutFocus Pointer to active focused PanelId. Updated on focus change.
 * @param ch Raw character / key code from getch().
 * @param[out] outEvent Receives decoded PanelEvent. Non-NULL.
 * @return APP_OK on success, error code otherwise.
 */
AppResult PanelInputProcess(const PanelLayout *layout, PanelId *inOutFocus,
                            int ch, PanelEvent *outEvent);

/**
 * @brief Decodes an explicit MEVENT mouse structure into a PanelEvent.
 *
 * Hit-tests mouse coordinates against the layout and maps clicks/scrolls.
 *
 * @param layout Current computed layout. Non-NULL.
 * @param inOutFocus Pointer to active focused PanelId. Updated on click.
 * @param mevent Pointer to curses MEVENT structure. Non-NULL.
 * @param[out] outEvent Receives decoded PanelEvent. Non-NULL.
 * @return APP_OK on success, error code otherwise.
 */
AppResult PanelInputProcessMouseEvent(const PanelLayout *layout,
                                      PanelId *inOutFocus, const MEVENT *mevent,
                                      PanelEvent *outEvent);

#ifdef __cplusplus
}
#endif
