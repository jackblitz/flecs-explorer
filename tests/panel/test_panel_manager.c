#include "test_panel_manager.h"

#include <stdio.h>
#include <string.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_manager.h"
#include "panel/panel_signals.h"
#include "test_cli.h"

/**
 * @brief Validates creation, layout queries, and destruction of PanelManager.
 */
static int TestPanelManagerLifecycle(void)
{
    PanelConfig config = {.minWidth = 80,
                          .minHeight = 24,
                          .entityPanelRatio = 0.35,
                          .enableMouse = false,
                          .enableColors = false,
                          .useDarculaTheme = true};

    PanelManager *manager = NULL;
    AppResult res = PanelManagerCreate(&config, &manager);
    TEST_ASSERT_EQ(res, APP_OK, "Create manager should return APP_OK");
    TEST_ASSERT_NOT_NULL(manager, "Manager handle must be non-NULL");

    // Default focus must be PANEL_ENTITIES
    TEST_ASSERT_EQ(PanelManagerGetFocus(manager), PANEL_ENTITIES,
                   "Default focus must be PANEL_ENTITIES");

    const PanelLayout *layout = PanelManagerGetLayout(manager);
    TEST_ASSERT_NOT_NULL(layout, "Layout pointer must be non-NULL");
    TEST_ASSERT_EQ(layout->headerRect.height, 3, "Header height is 3");
    TEST_ASSERT_EQ(layout->statusRect.height, 1, "Status height is 1");

    const PanelRect entRect = PanelManagerGetRect(manager, PANEL_ENTITIES);
    TEST_ASSERT(entRect.width > 0, "Entities rect width should be > 0");

    PanelManagerDestroy(manager);

    // Destroy(NULL) must be safe
    PanelManagerDestroy(NULL);

    return 0;
}

/**
 * @brief Validates focus accessors and focusability rules.
 */
static int TestPanelManagerFocusManagement(void)
{
    PanelConfig config = {.minWidth = 80,
                          .minHeight = 24,
                          .entityPanelRatio = 0.35,
                          .enableMouse = false,
                          .enableColors = false,
                          .useDarculaTheme = true};

    PanelManager *manager = NULL;
    PanelManagerCreate(&config, &manager);

    // Switch to Inspector
    PanelManagerSetFocus(manager, PANEL_INSPECTOR);
    TEST_ASSERT_EQ(PanelManagerGetFocus(manager), PANEL_INSPECTOR,
                   "Focused panel should be INSPECTOR");

    // Header and Status are not focusable (SetFocus should be a safe no-op)
    PanelManagerSetFocus(manager, PANEL_HEADER);
    TEST_ASSERT_EQ(
        PanelManagerGetFocus(manager), PANEL_INSPECTOR,
        "Focus should remain INSPECTOR after attempting header focus");

    PanelManagerSetFocus(manager, PANEL_STATUS);
    TEST_ASSERT_EQ(
        PanelManagerGetFocus(manager), PANEL_INSPECTOR,
        "Focus should remain INSPECTOR after attempting status focus");

    // FocusNext and FocusPrev
    PanelManagerFocusNext(manager);
    TEST_ASSERT_EQ(PanelManagerGetFocus(manager), PANEL_ENTITIES,
                   "FocusNext cycles Inspector -> Entities");

    PanelManagerFocusPrev(manager);
    TEST_ASSERT_EQ(PanelManagerGetFocus(manager), PANEL_INSPECTOR,
                   "FocusPrev cycles Entities -> Inspector");

    PanelManagerDestroy(manager);
    return 0;
}

/**
 * @brief Validates input routing through PanelManager.
 */
static int TestPanelManagerProcessInput(void)
{
    PanelConfig config = {.minWidth = 80,
                          .minHeight = 24,
                          .entityPanelRatio = 0.35,
                          .enableMouse = false,
                          .enableColors = false,
                          .useDarculaTheme = true};

    PanelManager *manager = NULL;
    PanelManagerCreate(&config, &manager);

    PanelEvent event;

    // Tab key switches focus: ENTITIES -> INSPECTOR
    AppResult res = PanelManagerProcessInput(manager, '\t', &event);
    TEST_ASSERT_EQ(res, APP_OK, "ProcessInput Tab returns APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_FOCUS_CHANGED,
                   "Emits PANEL_EVENT_FOCUS_CHANGED");
    TEST_ASSERT_EQ(PanelManagerGetFocus(manager), PANEL_INSPECTOR,
                   "Focus switched to INSPECTOR");

    // 'q' key emits QUIT
    res = PanelManagerProcessInput(manager, 'q', &event);
    TEST_ASSERT_EQ(res, APP_OK, "ProcessInput 'q' returns APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_QUIT, "Emits PANEL_EVENT_QUIT");

    // 'k' key targets active panel (INSPECTOR)
    res = PanelManagerProcessInput(manager, 'k', &event);
    TEST_ASSERT_EQ(res, APP_OK, "ProcessInput 'k' returns APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_KEY, "Emits PANEL_EVENT_KEY");
    TEST_ASSERT_EQ(event.targetPanel, PANEL_INSPECTOR,
                   "Targets active INSPECTOR panel");

    PanelManagerDestroy(manager);
    return 0;
}

/**
 * @brief Validates resize handling and terminal restoration safety.
 */
static int TestPanelManagerResizeAndSignals(void)
{
    PanelConfig config = {.minWidth = 80,
                          .minHeight = 24,
                          .entityPanelRatio = 0.35,
                          .enableMouse = false,
                          .enableColors = false,
                          .useDarculaTheme = true};

    PanelManager *manager = NULL;
    PanelManagerCreate(&config, &manager);

    AppResult res = PanelManagerHandleResize(manager);
    TEST_ASSERT_EQ(res, APP_OK, "HandleResize should return APP_OK");

    // Signal subsystem lifecycle
    res = PanelSignalsInstall();
    TEST_ASSERT_EQ(res, APP_OK, "Signals install should return APP_OK");

    TEST_ASSERT(!PanelSignalsReceivedResize(),
                "Resize flag should be false initially");
    PanelSignalsClearResize();
    TEST_ASSERT(!PanelSignalsReceivedQuit(),
                "Quit flag should be false initially");

    PanelSignalsRestore();

    // Idempotent terminal restoration
    PanelManagerRestoreTerminal(manager);
    PanelManagerRestoreTerminal(manager);
    PanelManagerRestoreTerminal(NULL);

    PanelManagerDestroy(manager);
    return 0;
}

/**
 * @brief Validates argument validation across all facade functions.
 */
static int TestPanelManagerInvalidArgs(void)
{
    PanelConfig config = {.minWidth = 80,
                          .minHeight = 24,
                          .entityPanelRatio = 0.35,
                          .enableMouse = false,
                          .enableColors = false,
                          .useDarculaTheme = true};

    PanelManager *manager = NULL;
    PanelEvent event;

    AppResult res = PanelManagerCreate(NULL, &manager);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL config returns INVALID_ARGUMENT");

    res = PanelManagerCreate(&config, NULL);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL outManager returns INVALID_ARGUMENT");

    res = PanelManagerProcessInput(NULL, 'q', &event);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL manager in ProcessInput returns INVALID_ARGUMENT");

    res = PanelManagerHandleResize(NULL);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL manager in HandleResize returns INVALID_ARGUMENT");

    // SetFocus, FocusNext, FocusPrev NULL safety
    PanelManagerSetFocus(NULL, PANEL_ENTITIES);
    PanelManagerFocusNext(NULL);
    PanelManagerFocusPrev(NULL);

    TEST_ASSERT_NULL(PanelManagerGetLayout(NULL),
                     "GetLayout(NULL) returns NULL");
    TEST_ASSERT_NULL(PanelManagerGetWindow(NULL, PANEL_ENTITIES),
                     "GetWindow(NULL) returns NULL");
    TEST_ASSERT(PanelManagerIsTooSmall(NULL), "IsTooSmall(NULL) returns true");

    return 0;
}

int TestPanelManagerRun(void)
{
    printf("[Panel Manager Test Suite]\n");
    TEST_RUN(TestPanelManagerLifecycle);
    TEST_RUN(TestPanelManagerFocusManagement);
    TEST_RUN(TestPanelManagerProcessInput);
    TEST_RUN(TestPanelManagerResizeAndSignals);
    TEST_RUN(TestPanelManagerInvalidArgs);
    printf("Panel manager test suite completed successfully.\n\n");
    return 0;
}
