#include "panel/panel_input.h"

#include <string.h>

#ifndef BUTTON4_PRESSED
#define BUTTON4_PRESSED 0x00010000
#endif

#ifndef BUTTON5_PRESSED
#define BUTTON5_PRESSED 0x00200000
#endif

/**
 * Checks if a panel is capable of receiving keyboard/mouse focus.
 *
 * @param panel Target PanelId.
 * @return true for Left Entity Browser and Right Component Inspector; false
 * otherwise.
 */
bool PanelInputIsFocusable(PanelId panel)
{
    return (panel == PANEL_ENTITIES || panel == PANEL_INSPECTOR);
}

/**
 * Cycles focus forward to the next focusable panel.
 *
 * @param inOutFocus Pointer to current focused PanelId.
 */
void PanelInputFocusNext(PanelId *inOutFocus)
{
    if (inOutFocus == NULL) {
        return;
    }

    if (*inOutFocus == PANEL_ENTITIES) {
        *inOutFocus = PANEL_INSPECTOR;
    } else {
        *inOutFocus = PANEL_ENTITIES;
    }
}

/**
 * Cycles focus backward to the previous focusable panel.
 *
 * @param inOutFocus Pointer to current focused PanelId.
 */
void PanelInputFocusPrev(PanelId *inOutFocus)
{
    if (inOutFocus == NULL) {
        return;
    }

    if (*inOutFocus == PANEL_INSPECTOR) {
        *inOutFocus = PANEL_ENTITIES;
    } else {
        *inOutFocus = PANEL_INSPECTOR;
    }
}

/**
 * Decodes an explicit MEVENT mouse structure into a PanelEvent.
 *
 * Maps screen coordinates to panel bounding boxes, updates focus on click,
 * and emits click or scroll event payloads.
 *
 * @param layout Current computed layout.
 * @param inOutFocus Pointer to active focused PanelId.
 * @param mevent Pointer to curses MEVENT structure.
 * @param outEvent Receives decoded PanelEvent.
 * @return APP_OK on success, APP_ERROR_INVALID_ARGUMENT on bad inputs.
 */
AppResult PanelInputProcessMouseEvent(const PanelLayout *layout,
                                      PanelId *inOutFocus, const MEVENT *mevent,
                                      PanelEvent *outEvent)
{
    if (layout == NULL || inOutFocus == NULL || mevent == NULL ||
        outEvent == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    memset(outEvent, 0, sizeof(PanelEvent));

    PanelId target = *inOutFocus;
    int localX = 0;
    int localY = 0;

    const bool hit = PanelLayoutHitTest(layout, mevent->x, mevent->y, &target,
                                        &localX, &localY);
    if (!hit) {
        target = *inOutFocus;
    }

    // 1. Mouse wheel scroll up (Button 4)
    if (mevent->bstate & BUTTON4_PRESSED) {
        outEvent->type = PANEL_EVENT_MOUSE_SCROLL;
        outEvent->targetPanel = target;
        outEvent->localX = localX;
        outEvent->localY = localY;
        outEvent->scrollDelta = -1;
        return APP_OK;
    }

    // 2. Mouse wheel scroll down (Button 5)
    if (mevent->bstate & BUTTON5_PRESSED) {
        outEvent->type = PANEL_EVENT_MOUSE_SCROLL;
        outEvent->targetPanel = target;
        outEvent->localX = localX;
        outEvent->localY = localY;
        outEvent->scrollDelta = 1;
        return APP_OK;
    }

    // 3. Mouse button click
    if (mevent->bstate & (BUTTON1_CLICKED | BUTTON1_PRESSED |
                          BUTTON1_DOUBLE_CLICKED | BUTTON1_TRIPLE_CLICKED)) {
        if (PanelInputIsFocusable(target)) {
            *inOutFocus = target;
        }

        outEvent->type = PANEL_EVENT_MOUSE_CLICK;
        outEvent->targetPanel = target;
        outEvent->localX = localX;
        outEvent->localY = localY;
        return APP_OK;
    }

    outEvent->type = PANEL_EVENT_NONE;
    return APP_OK;
}

/**
 * Decodes a raw curses character / key code or mouse event into a PanelEvent.
 *
 * @param layout Current computed layout.
 * @param inOutFocus Pointer to active focused PanelId.
 * @param ch Raw character / key code from getch().
 * @param outEvent Receives decoded PanelEvent.
 * @return APP_OK on success, APP_ERROR_INVALID_ARGUMENT on bad inputs.
 */
AppResult PanelInputProcess(const PanelLayout *layout, PanelId *inOutFocus,
                            int ch, PanelEvent *outEvent)
{
    if (layout == NULL || inOutFocus == NULL || outEvent == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    memset(outEvent, 0, sizeof(PanelEvent));

    if (ch == KEY_MOUSE) {
        MEVENT mevent;
        if (getmouse(&mevent) == OK) {
            return PanelInputProcessMouseEvent(layout, inOutFocus, &mevent,
                                               outEvent);
        }
        outEvent->type = PANEL_EVENT_NONE;
        return APP_OK;
    }

    if (ch == '\t') {
        PanelInputFocusNext(inOutFocus);
        outEvent->type = PANEL_EVENT_FOCUS_CHANGED;
        outEvent->targetPanel = *inOutFocus;
        return APP_OK;
    }

    if (ch == KEY_BTAB) {
        PanelInputFocusPrev(inOutFocus);
        outEvent->type = PANEL_EVENT_FOCUS_CHANGED;
        outEvent->targetPanel = *inOutFocus;
        return APP_OK;
    }

    if (ch == 'q' || ch == 'Q' || ch == KEY_F(10)) {
        outEvent->type = PANEL_EVENT_QUIT;
        return APP_OK;
    }

    if (ch == KEY_RESIZE) {
        outEvent->type = PANEL_EVENT_RESIZE;
        return APP_OK;
    }

    outEvent->type = PANEL_EVENT_KEY;
    outEvent->key = ch;
    outEvent->targetPanel = *inOutFocus;
    return APP_OK;
}
