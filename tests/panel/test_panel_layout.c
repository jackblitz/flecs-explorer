#include "test_panel_layout.h"

#include <stdio.h>
#include <string.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_layout.h"
#include "test_cli.h"

/**
 * @brief Validates standard 80x24 minimum resolution layout geometry.
 */
static int TestPanelLayoutStandard80x24(void)
{
    PanelLayout layout;
    memset(&layout, 0, sizeof(layout));

    const AppResult result = PanelLayoutCompute(80, 24, 0.35, 80, 24, &layout);

    TEST_ASSERT_EQ(result, APP_OK, "PanelLayoutCompute should return APP_OK");
    TEST_ASSERT(!layout.isTooSmall, "80x24 should not be marked isTooSmall");
    TEST_ASSERT_EQ(layout.screenWidth, 80, "screenWidth should be 80");
    TEST_ASSERT_EQ(layout.screenHeight, 24, "screenHeight should be 24");

    // Header: Top 3 rows, full width [0, 0, 80, 3]
    TEST_ASSERT_EQ(layout.headerRect.x, 0, "Header X should be 0");
    TEST_ASSERT_EQ(layout.headerRect.y, 0, "Header Y should be 0");
    TEST_ASSERT_EQ(layout.headerRect.width, 80, "Header width should be 80");
    TEST_ASSERT_EQ(layout.headerRect.height, 3, "Header height should be 3");

    // Status: Bottom 1 row, full width [0, 23, 80, 1]
    TEST_ASSERT_EQ(layout.statusRect.x, 0, "Status X should be 0");
    TEST_ASSERT_EQ(layout.statusRect.y, 23,
                   "Status Y should be 23 (height - 1)");
    TEST_ASSERT_EQ(layout.statusRect.width, 80, "Status width should be 80");
    TEST_ASSERT_EQ(layout.statusRect.height, 1, "Status height should be 1");

    // Center body height = 24 - 3 (header) - 1 (status) = 20 rows
    // Left Entity: 80 * 0.35 = 28 cols [0, 3, 28, 20]
    TEST_ASSERT_EQ(layout.entitiesRect.x, 0, "Entities X should be 0");
    TEST_ASSERT_EQ(layout.entitiesRect.y, 3, "Entities Y should be 3");
    TEST_ASSERT_EQ(layout.entitiesRect.width, 28,
                   "Entities width should be 28");
    TEST_ASSERT_EQ(layout.entitiesRect.height, 20,
                   "Entities height should be 20");

    // Right Inspector: remaining width = 80 - 28 = 52 cols [28, 3, 52, 20]
    TEST_ASSERT_EQ(layout.inspectorRect.x, 28, "Inspector X should be 28");
    TEST_ASSERT_EQ(layout.inspectorRect.y, 3, "Inspector Y should be 3");
    TEST_ASSERT_EQ(layout.inspectorRect.width, 52,
                   "Inspector width should be 52");
    TEST_ASSERT_EQ(layout.inspectorRect.height, 20,
                   "Inspector height should be 20");

    return 0;
}

/**
 * @brief Validates 120x40 standard widescreen console layout geometry.
 */
static int TestPanelLayoutWidescreen120x40(void)
{
    PanelLayout layout;
    memset(&layout, 0, sizeof(layout));

    const AppResult result = PanelLayoutCompute(120, 40, 0.40, 80, 24, &layout);

    TEST_ASSERT_EQ(result, APP_OK, "PanelLayoutCompute should return APP_OK");
    TEST_ASSERT(!layout.isTooSmall, "120x40 should not be marked isTooSmall");

    // Header
    TEST_ASSERT_EQ(layout.headerRect.x, 0, "Header X");
    TEST_ASSERT_EQ(layout.headerRect.y, 0, "Header Y");
    TEST_ASSERT_EQ(layout.headerRect.width, 120, "Header width");
    TEST_ASSERT_EQ(layout.headerRect.height, 3, "Header height");

    // Status
    TEST_ASSERT_EQ(layout.statusRect.x, 0, "Status X");
    TEST_ASSERT_EQ(layout.statusRect.y, 39, "Status Y (40 - 1)");
    TEST_ASSERT_EQ(layout.statusRect.width, 120, "Status width");
    TEST_ASSERT_EQ(layout.statusRect.height, 1, "Status height");

    // Center: height = 36
    // Entities: 120 * 0.40 = 48
    TEST_ASSERT_EQ(layout.entitiesRect.x, 0, "Entities X");
    TEST_ASSERT_EQ(layout.entitiesRect.y, 3, "Entities Y");
    TEST_ASSERT_EQ(layout.entitiesRect.width, 48, "Entities width");
    TEST_ASSERT_EQ(layout.entitiesRect.height, 36, "Entities height");

    // Inspector: 120 - 48 = 72
    TEST_ASSERT_EQ(layout.inspectorRect.x, 48, "Inspector X");
    TEST_ASSERT_EQ(layout.inspectorRect.y, 3, "Inspector Y");
    TEST_ASSERT_EQ(layout.inspectorRect.width, 72, "Inspector width");
    TEST_ASSERT_EQ(layout.inspectorRect.height, 36, "Inspector height");

    return 0;
}

/**
 * @brief Validates undersized terminal boundaries and isTooSmall behavior.
 */
static int TestPanelLayoutUndersizedTerminal(void)
{
    PanelLayout layout;

    // Width too small (79x24)
    AppResult res = PanelLayoutCompute(79, 24, 0.35, 80, 24, &layout);
    TEST_ASSERT_EQ(res, APP_OK, "Compute should return OK for 79x24");
    TEST_ASSERT(layout.isTooSmall, "79x24 must be flagged isTooSmall");
    TEST_ASSERT(PanelLayoutIsTooSmall(79, 24, 80, 24),
                "PanelLayoutIsTooSmall must return true for 79x24");

    // Height too small (80x23)
    res = PanelLayoutCompute(80, 23, 0.35, 80, 24, &layout);
    TEST_ASSERT_EQ(res, APP_OK, "Compute should return OK for 80x23");
    TEST_ASSERT(layout.isTooSmall, "80x23 must be flagged isTooSmall");
    TEST_ASSERT(PanelLayoutIsTooSmall(80, 23, 80, 24),
                "PanelLayoutIsTooSmall must return true for 80x23");

    // Both too small (50x15)
    res = PanelLayoutCompute(50, 15, 0.35, 80, 24, &layout);
    TEST_ASSERT_EQ(res, APP_OK, "Compute should return OK for 50x15");
    TEST_ASSERT(layout.isTooSmall, "50x15 must be flagged isTooSmall");

    return 0;
}

/**
 * @brief Validates 2D hit-testing across all distinct panels.
 */
static int TestPanelLayoutHitTesting(void)
{
    PanelLayout layout;
    const AppResult res = PanelLayoutCompute(100, 30, 0.40, 80, 24, &layout);
    TEST_ASSERT_EQ(res, APP_OK, "Compute layout");

    PanelId panel = PANEL_COUNT;
    int localX = -1;
    int localY = -1;

    // 1. Point in Header: (10, 1) -> PANEL_HEADER, local (10, 1)
    bool hit = PanelLayoutHitTest(&layout, 10, 1, &panel, &localX, &localY);
    TEST_ASSERT(hit, "Point (10, 1) should hit");
    TEST_ASSERT_EQ(panel, PANEL_HEADER, "Target panel should be HEADER");
    TEST_ASSERT_EQ(localX, 10, "localX should be 10");
    TEST_ASSERT_EQ(localY, 1, "localY should be 1");

    // 2. Point in Left Entities Panel: (15, 10) -> PANEL_ENTITIES (x:0-39,
    // y:3-28)
    hit = PanelLayoutHitTest(&layout, 15, 10, &panel, &localX, &localY);
    TEST_ASSERT(hit, "Point (15, 10) should hit");
    TEST_ASSERT_EQ(panel, PANEL_ENTITIES, "Target panel should be ENTITIES");
    TEST_ASSERT_EQ(localX, 15, "localX should be 15");
    TEST_ASSERT_EQ(localY, 7, "localY should be 7 (10 - 3)");

    // 3. Point in Right Inspector Panel: (60, 12) -> PANEL_INSPECTOR (x:40-99,
    // y:3-28)
    hit = PanelLayoutHitTest(&layout, 60, 12, &panel, &localX, &localY);
    TEST_ASSERT(hit, "Point (60, 12) should hit");
    TEST_ASSERT_EQ(panel, PANEL_INSPECTOR, "Target panel should be INSPECTOR");
    TEST_ASSERT_EQ(localX, 20, "localX should be 20 (60 - 40)");
    TEST_ASSERT_EQ(localY, 9, "localY should be 9 (12 - 3)");

    // 4. Point in Status Bar: (45, 29) -> PANEL_STATUS (x:0-99, y:29)
    hit = PanelLayoutHitTest(&layout, 45, 29, &panel, &localX, &localY);
    TEST_ASSERT(hit, "Point (45, 29) should hit");
    TEST_ASSERT_EQ(panel, PANEL_STATUS, "Target panel should be STATUS");
    TEST_ASSERT_EQ(localX, 45, "localX should be 45");
    TEST_ASSERT_EQ(localY, 0, "localY should be 0 (29 - 29)");

    // 5. Out of bounds points
    hit = PanelLayoutHitTest(&layout, -1, 5, &panel, &localX, &localY);
    TEST_ASSERT(!hit, "Negative X should not hit");

    hit = PanelLayoutHitTest(&layout, 100, 5, &panel, &localX, &localY);
    TEST_ASSERT(!hit, "X >= width should not hit");

    hit = PanelLayoutHitTest(&layout, 50, 30, &panel, &localX, &localY);
    TEST_ASSERT(!hit, "Y >= height should not hit");

    // 6. Hit testing on too-small layout returns false
    PanelLayout smallLayout;
    PanelLayoutCompute(50, 20, 0.35, 80, 24, &smallLayout);
    hit = PanelLayoutHitTest(&smallLayout, 10, 10, &panel, &localX, &localY);
    TEST_ASSERT(!hit, "Hit testing on tooSmall layout must return false");

    return 0;
}

/**
 * @brief Validates error handling for invalid input arguments.
 */
static int TestPanelLayoutInvalidArguments(void)
{
    PanelLayout layout;

    // NULL outLayout pointer
    AppResult res = PanelLayoutCompute(80, 24, 0.35, 80, 24, NULL);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL outLayout must return INVALID_ARGUMENT");

    // Non-positive width or height
    res = PanelLayoutCompute(0, 24, 0.35, 80, 24, &layout);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "Zero width must return INVALID_ARGUMENT");

    res = PanelLayoutCompute(80, -5, 0.35, 80, 24, &layout);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "Negative height must return INVALID_ARGUMENT");

    // Invalid ratio (< 0.05 or > 0.95)
    res = PanelLayoutCompute(80, 24, 0.0, 80, 24, &layout);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "Zero ratio must return INVALID_ARGUMENT");

    res = PanelLayoutCompute(80, 24, 1.2, 80, 24, &layout);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "Ratio > 1.0 must return INVALID_ARGUMENT");

    // Hit test NULL checks
    PanelId panel;
    int lx, ly;
    bool hit = PanelLayoutHitTest(NULL, 10, 10, &panel, &lx, &ly);
    TEST_ASSERT(!hit, "NULL layout in hit test must return false");

    return 0;
}

int TestPanelLayoutRun(void)
{
    printf("[Panel Layout Test Suite]\n");
    TEST_RUN(TestPanelLayoutStandard80x24);
    TEST_RUN(TestPanelLayoutWidescreen120x40);
    TEST_RUN(TestPanelLayoutUndersizedTerminal);
    TEST_RUN(TestPanelLayoutHitTesting);
    TEST_RUN(TestPanelLayoutInvalidArguments);
    printf("Panel layout test suite completed successfully.\n\n");
    return 0;
}
