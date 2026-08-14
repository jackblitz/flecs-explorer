#include "flecs_explorer/panel/panel_layout.h"

#include <math.h>
#include <string.h>

#define PANEL_HEADER_HEIGHT 3
#define PANEL_STATUS_HEIGHT 1
#define PANEL_MIN_RATIO 0.05
#define PANEL_MAX_RATIO 0.95

/**
 * Checks if given dimensions meet minimum display constraints.
 *
 * Compares current width and height against required minimum thresholds.
 *
 * @param width Screen width in columns.
 * @param height Screen height in rows.
 * @param minWidth Minimum required width.
 * @param minHeight Minimum required height.
 * @return true if width < minWidth or height < minHeight; false otherwise.
 */
bool PanelLayoutIsTooSmall(int width, int height, int minWidth, int minHeight)
{
    return (width < minWidth || height < minHeight);
}

/**
 * Computes screen partition rectangles for all panels given dimensions.
 *
 * Pure layout math: divides total screen height into Header (3 rows),
 * Status (1 row), and central body. Central body width is split into
 * Left Entity panel (width * entityRatio) and Right Inspector panel
 * (remaining).
 *
 * @param width Total screen width in columns.
 * @param height Total screen height in rows.
 * @param entityRatio Desired width ratio for entity panel (0.05 to 0.95).
 * @param minWidth Minimum supported screen width.
 * @param minHeight Minimum supported screen height.
 * @param outLayout Output struct populated with panel rectangles and
 * isTooSmall.
 * @return APP_OK on success, APP_ERROR_INVALID_ARGUMENT on bad inputs.
 */
AppResult PanelLayoutCompute(int width, int height, double entityRatio,
                             int minWidth, int minHeight,
                             PanelLayout *outLayout)
{
    if (outLayout == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    if (width <= 0 || height <= 0) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    if (entityRatio < PANEL_MIN_RATIO || entityRatio > PANEL_MAX_RATIO) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    outLayout->screenWidth = width;
    outLayout->screenHeight = height;
    outLayout->isTooSmall =
        PanelLayoutIsTooSmall(width, height, minWidth, minHeight);

    if (outLayout->isTooSmall) {
        memset(&outLayout->headerRect, 0, sizeof(PanelRect));
        memset(&outLayout->entitiesRect, 0, sizeof(PanelRect));
        memset(&outLayout->inspectorRect, 0, sizeof(PanelRect));
        memset(&outLayout->statusRect, 0, sizeof(PanelRect));
        return APP_OK;
    }

    // 1. Header: Top 3 rows across full width
    outLayout->headerRect.x = 0;
    outLayout->headerRect.y = 0;
    outLayout->headerRect.width = width;
    outLayout->headerRect.height = PANEL_HEADER_HEIGHT;

    // 2. Status: Bottom 1 row across full width
    outLayout->statusRect.x = 0;
    outLayout->statusRect.y = height - PANEL_STATUS_HEIGHT;
    outLayout->statusRect.width = width;
    outLayout->statusRect.height = PANEL_STATUS_HEIGHT;

    // 3. Center Body: Remaining vertical space
    const int bodyY = PANEL_HEADER_HEIGHT;
    const int bodyHeight = height - PANEL_HEADER_HEIGHT - PANEL_STATUS_HEIGHT;

    // Split horizontally: Left (Entities) vs Right (Inspector)
    const int entitiesWidth = (int)floor((double)width * entityRatio);
    const int inspectorWidth = width - entitiesWidth;

    outLayout->entitiesRect.x = 0;
    outLayout->entitiesRect.y = bodyY;
    outLayout->entitiesRect.width = entitiesWidth;
    outLayout->entitiesRect.height = bodyHeight;

    outLayout->inspectorRect.x = entitiesWidth;
    outLayout->inspectorRect.y = bodyY;
    outLayout->inspectorRect.width = inspectorWidth;
    outLayout->inspectorRect.height = bodyHeight;

    return APP_OK;
}

/**
 * Checks if a point lies within a bounding box rectangle.
 *
 * @param rect Pointer to target PanelRect.
 * @param x Point X coordinate.
 * @param y Point Y coordinate.
 * @return true if (x, y) is inside rect bounds; false otherwise.
 */
static bool RectContainsPoint(const PanelRect *rect, int x, int y)
{
    return (x >= rect->x && x < rect->x + rect->width && y >= rect->y &&
            y < rect->y + rect->height);
}

/**
 * Performs 2D hit-testing to identify which panel contains point (x, y).
 *
 * Tests the point against each panel rectangle and computes panel-relative
 * local coordinates.
 *
 * @param layout Current calculated layout.
 * @param screenX Global screen column (0-indexed).
 * @param screenY Global screen row (0-indexed).
 * @param outPanel Receives detected PanelId on match.
 * @param outLocalX Receives column relative to panel origin on match.
 * @param outLocalY Receives row relative to panel origin on match.
 * @return true if point is inside a panel, false otherwise.
 */
bool PanelLayoutHitTest(const PanelLayout *layout, int screenX, int screenY,
                        PanelId *outPanel, int *outLocalX, int *outLocalY)
{
    if (layout == NULL || layout->isTooSmall) {
        return false;
    }

    if (screenX < 0 || screenX >= layout->screenWidth || screenY < 0 ||
        screenY >= layout->screenHeight) {
        return false;
    }

    const PanelRect rects[PANEL_COUNT] = {
        [PANEL_HEADER] = layout->headerRect,
        [PANEL_ENTITIES] = layout->entitiesRect,
        [PANEL_INSPECTOR] = layout->inspectorRect,
        [PANEL_STATUS] = layout->statusRect,
    };

    for (int i = 0; i < PANEL_COUNT; ++i) {
        if (RectContainsPoint(&rects[i], screenX, screenY)) {
            if (outPanel != NULL) {
                *outPanel = (PanelId)i;
            }
            if (outLocalX != NULL) {
                *outLocalX = screenX - rects[i].x;
            }
            if (outLocalY != NULL) {
                *outLocalY = screenY - rects[i].y;
            }
            return true;
        }
    }

    return false;
}
