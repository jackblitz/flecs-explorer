/**
 * @file layout.h
 * @brief Opaque Window Manager for panel layout.
 *
 * Example usage:
 * @code
 * PanelLayout *layout = NULL;
 * PanelResult res = PanelLayoutCreate(&layout);
 * if (res == PANEL_OK) {
 *     PanelLayoutBordersRender(layout);
 *     PanelLayoutFocusCycle(layout);
 *     PanelLayoutDestroy(layout);
 * }
 * @endcode
 */
#pragma once

#include "panel/terminal.h"

typedef enum PanelPaneId {
    PANEL_PANE_ENTITY_TREE = 0,
    PANEL_PANE_COMPONENT_INSPECTOR = 1,
    PANEL_PANE_COUNT = 2
} PanelPaneId;

typedef struct PanelLayout PanelLayout;

/**
 * @brief Creates a PanelLayout instance.
 * @param outLayout Pointer to layout handle.
 * @return PANEL_OK on success, or error code on failure.
 */
PanelResult PanelLayoutCreate(PanelLayout **outLayout);

/**
 * @brief Destroys a PanelLayout instance.
 * @param layout The layout to destroy.
 */
void PanelLayoutDestroy(PanelLayout *layout);

/**
 * @brief Resizes the layout to match the current terminal dimensions.
 * @param layout The layout.
 */
void PanelLayoutResize(PanelLayout *layout);

/**
 * @brief Cycles focus between panes.
 * @param layout The layout.
 */
void PanelLayoutFocusCycle(PanelLayout *layout);

/**
 * @brief Gets the active pane.
 * @param layout The layout.
 * @return The active pane ID.
 */
PanelPaneId PanelLayoutActivePaneGet(const PanelLayout *layout);

/**
 * @brief Renders the borders and headers for the layout.
 * @param layout The layout.
 */
void PanelLayoutBordersRender(const PanelLayout *layout);
