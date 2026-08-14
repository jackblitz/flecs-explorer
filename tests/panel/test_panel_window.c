#include "test_panel_window.h"

#include <stdio.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_layout.h"
#include "panel/panel_window.h"
#include "test_cli.h"

/**
 * @brief Validates argument validation in window allocation.
 */
static int TestPanelWindowsInvalidArgs(void)
{
    PanelWindows *windows = NULL;
    PanelLayout layout;
    PanelLayoutCompute(100, 30, 0.35, 80, 24, &layout);

    AppResult res = PanelWindowsCreate(NULL, &windows);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL layout must return INVALID_ARGUMENT");

    res = PanelWindowsCreate(&layout, NULL);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL outWindows must return INVALID_ARGUMENT");

    return 0;
}

/**
 * @brief Validates window allocation and rect retrieval for too-small layouts.
 */
static int TestPanelWindowsTooSmallLayout(void)
{
    PanelLayout layout;
    PanelLayoutCompute(50, 20, 0.35, 80, 24, &layout);
    TEST_ASSERT(layout.isTooSmall, "Layout should be marked too small");

    PanelWindows *windows = NULL;
    AppResult res = PanelWindowsCreate(&layout, &windows);
    TEST_ASSERT_EQ(res, APP_OK,
                   "Create on tooSmall layout should return APP_OK");
    TEST_ASSERT_NOT_NULL(windows, "Windows handle should be non-NULL");

    // All windows should be NULL when tooSmall
    for (int p = 0; p < PANEL_COUNT; ++p) {
        WINDOW *win = PanelWindowsGet(windows, (PanelId)p);
        TEST_ASSERT_NULL(win, "Windows should be NULL when tooSmall");
    }

    PanelWindowsDestroy(windows);
    return 0;
}

/**
 * @brief Validates safe destruction and NULL handling.
 */
static int TestPanelWindowsNullSafety(void)
{
    // Destroy(NULL) must be safe no-op
    PanelWindowsDestroy(NULL);

    WINDOW *win = PanelWindowsGet(NULL, PANEL_ENTITIES);
    TEST_ASSERT_NULL(win, "Get on NULL windows must return NULL");

    const PanelRect rect = PanelWindowsGetRect(NULL, PANEL_HEADER);
    TEST_ASSERT_EQ(rect.width, 0, "Rect width on NULL windows should be 0");
    TEST_ASSERT_EQ(rect.height, 0, "Rect height on NULL windows should be 0");

    return 0;
}

int TestPanelWindowRun(void)
{
    printf("[Panel Window Test Suite]\n");
    TEST_RUN(TestPanelWindowsInvalidArgs);
    TEST_RUN(TestPanelWindowsTooSmallLayout);
    TEST_RUN(TestPanelWindowsNullSafety);
    printf("Panel window test suite completed successfully.\n\n");
    return 0;
}
