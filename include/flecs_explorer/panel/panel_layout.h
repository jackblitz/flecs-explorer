#pragma once

/**
 * @file panel_layout.h
 * @brief Pure geometric calculations for multi-panel screen partitioning.
 *
 * Typical usage:
 * @code
 *     PanelLayout layout;
 *     AppResult result = PanelLayoutCompute(120, 40, 0.35, 80, 24, &layout);
 *     if (result == APP_OK && !layout.isTooSmall) {
 *         // Use layout.headerRect, layout.entitiesRect, etc.
 *     }
 * @endcode
 *
 * Design & Determinism:
 * All functions in this header are pure mathematical routines that operate
 * without any ncurses or terminal dependencies. This guarantees 100%
 * headless, deterministic testability across all resolutions and edge cases.
 */

#include <stdbool.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Computes screen partition rectangles for all panels given dimensions.
 *
 * Calculates the bounding boxes for Header (top 3 rows), Status footer
 * (bottom 1 row), and splits the remaining vertical space between Left Entity
 * Panel and Right Inspector Panel according to entityRatio. If screen
 * dimensions are below minWidth x minHeight, the isTooSmall flag is set to
 * true.
 *
 * @param width Total screen width in columns (must be positive).
 * @param height Total screen height in rows (must be positive).
 * @param entityRatio Desired width ratio for entity panel (0.1 to 0.9).
 * @param minWidth Minimum supported screen width (e.g. 80).
 * @param minHeight Minimum supported screen height (e.g. 24).
 * @param[out] outLayout Output struct receiving calculated panel rectangles.
 *                       Untouched on failure.
 * @return APP_OK on success, APP_ERROR_INVALID_ARGUMENT if outLayout is NULL,
 *         dimensions are non-positive, or entityRatio is out of range.
 */
AppResult PanelLayoutCompute(int width, int height, double entityRatio,
                             int minWidth, int minHeight,
                             PanelLayout *outLayout);

/**
 * @brief Checks if given dimensions meet minimum display constraints.
 *
 * @param width Screen width in columns.
 * @param height Screen height in rows.
 * @param minWidth Minimum required width.
 * @param minHeight Minimum required height.
 * @return true if width < minWidth or height < minHeight; false otherwise.
 */
bool PanelLayoutIsTooSmall(int width, int height, int minWidth, int minHeight);

/**
 * @brief Performs 2D hit-testing to identify which panel contains point (x, y).
 *
 * Maps global screen column and row coordinates into a target PanelId and
 * calculates the local (x, y) offset relative to that panel's top-left origin.
 *
 * @param layout Current calculated layout. Must not be NULL.
 * @param screenX Global screen column (0-indexed).
 * @param screenY Global screen row (0-indexed).
 * @param[out] outPanel Receives detected PanelId on match.
 * @param[out] outLocalX Receives column relative to panel origin on match.
 * @param[out] outLocalY Receives row relative to panel origin on match.
 * @return true if point is inside a valid panel bounding box; false if outside
 *         screen or if layout is marked as too small.
 */
bool PanelLayoutHitTest(const PanelLayout *layout, int screenX, int screenY,
                        PanelId *outPanel, int *outLocalX, int *outLocalY);

#ifdef __cplusplus
}
#endif
