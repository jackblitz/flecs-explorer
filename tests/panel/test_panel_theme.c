#include "test_panel_theme.h"

#include <stdio.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_theme.h"
#include "test_cli.h"

/**
 * @brief Validates theme initialization with colors disabled.
 */
static int TestPanelThemeInitDisabled(void)
{
    const AppResult res = PanelThemeInit(false);
    TEST_ASSERT_EQ(res, APP_OK,
                   "Theme init with colors disabled should return APP_OK");
    return 0;
}

/**
 * @brief Validates safe no-op handling when applying color pairs to NULL
 * window.
 */
static int TestPanelThemeApplyNullSafe(void)
{
    // Must not segfault or fail on NULL window
    PanelThemeApplyPair(NULL, PANEL_COLOR_DEFAULT);
    PanelThemeApplyPair(NULL, PANEL_COLOR_BORDER_ACTIVE);
    PanelThemeApplyPair(NULL, PANEL_COLOR_SELECTION);
    return 0;
}

int TestPanelThemeRun(void)
{
    printf("[Panel Theme Test Suite]\n");
    TEST_RUN(TestPanelThemeInitDisabled);
    TEST_RUN(TestPanelThemeApplyNullSafe);
    printf("Panel theme test suite completed successfully.\n\n");
    return 0;
}
