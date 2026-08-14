#include "test_panel_renderer.h"

#include <stdio.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_renderer.h"
#include "test_cli.h"

/**
 * @brief Validates renderer creation, destruction, and argument checks.
 */
static int TestPanelRendererLifecycle(void)
{
    PanelRenderer *renderer = NULL;

    AppResult res = PanelRendererCreate(NULL);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL outRenderer should return INVALID_ARGUMENT");

    res = PanelRendererCreate(&renderer);
    TEST_ASSERT_EQ(res, APP_OK, "Create renderer should return APP_OK");
    TEST_ASSERT_NOT_NULL(renderer, "Renderer handle should be non-NULL");

    res = PanelRendererBeginFrame(renderer);
    TEST_ASSERT_EQ(res, APP_OK, "BeginFrame should return APP_OK");

    res = PanelRendererEndFrame(renderer);
    TEST_ASSERT_EQ(res, APP_OK, "EndFrame should return APP_OK");

    PanelRendererDestroy(renderer);

    // Destroy(NULL) must be safe no-op
    PanelRendererDestroy(NULL);

    return 0;
}

/**
 * @brief Validates NULL safety across all drawing operations.
 */
static int TestPanelRendererDrawNullSafety(void)
{
    PanelRenderer *renderer = NULL;
    PanelRendererCreate(&renderer);

    // Draw functions with NULL windows / strings must not crash
    PanelRendererDrawBorder(renderer, NULL, "Entities", true);
    PanelRendererDrawBorder(NULL, NULL, NULL, false);
    PanelRendererDrawText(renderer, NULL, 0, 0, "Test", PANEL_COLOR_DEFAULT);
    PanelRendererDrawText(renderer, NULL, 0, 0, NULL, PANEL_COLOR_DEFAULT);
    PanelRendererDrawTooSmallWarning(renderer, 50, 20, 80, 24);
    PanelRendererClearWindow(renderer, NULL);

    PanelRendererDestroy(renderer);
    return 0;
}

int TestPanelRendererRun(void)
{
    printf("[Panel Renderer Test Suite]\n");
    TEST_RUN(TestPanelRendererLifecycle);
    TEST_RUN(TestPanelRendererDrawNullSafety);
    printf("Panel renderer test suite completed successfully.\n\n");
    return 0;
}
