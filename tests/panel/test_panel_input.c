#include "test_panel_input.h"

#include <stdio.h>
#include <string.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_layout.h"
#include "panel/panel_input.h"
#include "test_cli.h"

/**
 * @brief Validates keyboard shortcut mapping and focus toggling via Tab.
 */
static int TestPanelInputKeyMapping(void)
{
    PanelLayout layout;
    PanelLayoutCompute(100, 30, 0.35, 80, 24, &layout);

    PanelId focus = PANEL_ENTITIES;
    PanelEvent event;

    // 1. Tab ('\t') cycles focus: ENTITIES -> INSPECTOR
    AppResult res = PanelInputProcess(&layout, &focus, '\t', &event);
    TEST_ASSERT_EQ(res, APP_OK, "Tab input should return APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_FOCUS_CHANGED,
                   "Tab should emit PANEL_EVENT_FOCUS_CHANGED");
    TEST_ASSERT_EQ(focus, PANEL_INSPECTOR, "Focus should switch to INSPECTOR");
    TEST_ASSERT_EQ(event.targetPanel, PANEL_INSPECTOR,
                   "Target panel should match new focus");

    // 2. Tab again cycles back: INSPECTOR -> ENTITIES
    res = PanelInputProcess(&layout, &focus, '\t', &event);
    TEST_ASSERT_EQ(res, APP_OK, "Tab input should return APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_FOCUS_CHANGED,
                   "Tab should emit PANEL_EVENT_FOCUS_CHANGED");
    TEST_ASSERT_EQ(focus, PANEL_ENTITIES,
                   "Focus should switch back to ENTITIES");

    // 3. Shift-Tab (KEY_BTAB) cycles in reverse: ENTITIES -> INSPECTOR
    res = PanelInputProcess(&layout, &focus, KEY_BTAB, &event);
    TEST_ASSERT_EQ(res, APP_OK, "Shift-Tab should return APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_FOCUS_CHANGED,
                   "Shift-Tab should emit PANEL_EVENT_FOCUS_CHANGED");
    TEST_ASSERT_EQ(focus, PANEL_INSPECTOR, "Focus should switch to INSPECTOR");

    // 4. 'q' and 'Q' emit PANEL_EVENT_QUIT
    res = PanelInputProcess(&layout, &focus, 'q', &event);
    TEST_ASSERT_EQ(res, APP_OK, "'q' should return APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_QUIT,
                   "'q' should emit PANEL_EVENT_QUIT");

    res = PanelInputProcess(&layout, &focus, 'Q', &event);
    TEST_ASSERT_EQ(res, APP_OK, "'Q' should return APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_QUIT,
                   "'Q' should emit PANEL_EVENT_QUIT");

    // 5. KEY_RESIZE emits PANEL_EVENT_RESIZE
    res = PanelInputProcess(&layout, &focus, KEY_RESIZE, &event);
    TEST_ASSERT_EQ(res, APP_OK, "KEY_RESIZE should return APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_RESIZE,
                   "KEY_RESIZE should emit PANEL_EVENT_RESIZE");

    // 6. Navigation keys target the focused panel
    focus = PANEL_ENTITIES;
    res = PanelInputProcess(&layout, &focus, 'j', &event);
    TEST_ASSERT_EQ(res, APP_OK, "'j' navigation should return APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_KEY,
                   "'j' should emit PANEL_EVENT_KEY");
    TEST_ASSERT_EQ(event.key, 'j', "Keycode should match 'j'");
    TEST_ASSERT_EQ(event.targetPanel, PANEL_ENTITIES,
                   "Event should target focused ENTITIES panel");

    res = PanelInputProcess(&layout, &focus, KEY_DOWN, &event);
    TEST_ASSERT_EQ(res, APP_OK, "KEY_DOWN should return APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_KEY,
                   "KEY_DOWN emits PANEL_EVENT_KEY");
    TEST_ASSERT_EQ(event.key, KEY_DOWN, "Keycode matches KEY_DOWN");

    return 0;
}

/**
 * @brief Validates focus capability checks and focus cycling helpers.
 */
static int TestPanelInputFocusCycling(void)
{
    TEST_ASSERT(!PanelInputIsFocusable(PANEL_HEADER),
                "Header must not be focusable");
    TEST_ASSERT(!PanelInputIsFocusable(PANEL_STATUS),
                "Status must not be focusable");
    TEST_ASSERT(PanelInputIsFocusable(PANEL_ENTITIES),
                "Entities panel must be focusable");
    TEST_ASSERT(PanelInputIsFocusable(PANEL_INSPECTOR),
                "Inspector panel must be focusable");

    PanelId focus = PANEL_ENTITIES;
    PanelInputFocusNext(&focus);
    TEST_ASSERT_EQ(focus, PANEL_INSPECTOR,
                   "FocusNext from Entities -> Inspector");

    PanelInputFocusNext(&focus);
    TEST_ASSERT_EQ(focus, PANEL_ENTITIES,
                   "FocusNext from Inspector -> Entities");

    PanelInputFocusPrev(&focus);
    TEST_ASSERT_EQ(focus, PANEL_INSPECTOR,
                   "FocusPrev from Entities -> Inspector");

    PanelInputFocusPrev(&focus);
    TEST_ASSERT_EQ(focus, PANEL_ENTITIES,
                   "FocusPrev from Inspector -> Entities");

    return 0;
}

/**
 * @brief Validates mouse click decoding and automatic focus switching.
 */
static int TestPanelInputMouseClick(void)
{
    PanelLayout layout;
    PanelLayoutCompute(100, 30, 0.35, 80, 24, &layout);

    PanelId focus = PANEL_ENTITIES;
    PanelEvent event;
    MEVENT mevent;
    memset(&mevent, 0, sizeof(mevent));

    // Click inside Right Inspector panel at (60, 15)
    mevent.x = 60;
    mevent.y = 15;
    mevent.bstate = BUTTON1_CLICKED;

    AppResult res =
        PanelInputProcessMouseEvent(&layout, &focus, &mevent, &event);
    TEST_ASSERT_EQ(res, APP_OK, "Mouse click should return APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_MOUSE_CLICK,
                   "Should emit PANEL_EVENT_MOUSE_CLICK");
    TEST_ASSERT_EQ(event.targetPanel, PANEL_INSPECTOR,
                   "Target should be PANEL_INSPECTOR");
    TEST_ASSERT_EQ(focus, PANEL_INSPECTOR,
                   "Focus should update to PANEL_INSPECTOR");
    TEST_ASSERT_EQ(event.localX, 60 - layout.inspectorRect.x,
                   "localX should match offset");
    TEST_ASSERT_EQ(event.localY, 15 - layout.inspectorRect.y,
                   "localY should match offset");

    // Click inside Header at (10, 1) -> Header is not focusable, focus
    // unchanged
    mevent.x = 10;
    mevent.y = 1;
    mevent.bstate = BUTTON1_CLICKED;

    res = PanelInputProcessMouseEvent(&layout, &focus, &mevent, &event);
    TEST_ASSERT_EQ(res, APP_OK, "Header click should return APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_MOUSE_CLICK,
                   "Emits PANEL_EVENT_MOUSE_CLICK");
    TEST_ASSERT_EQ(event.targetPanel, PANEL_HEADER, "Target is PANEL_HEADER");
    TEST_ASSERT_EQ(
        focus, PANEL_INSPECTOR,
        "Focus should remain INSPECTOR when clicking non-focusable header");

    return 0;
}

/**
 * @brief Validates mouse wheel scroll event decoding.
 */
static int TestPanelInputMouseScroll(void)
{
    PanelLayout layout;
    PanelLayoutCompute(100, 30, 0.35, 80, 24, &layout);

    PanelId focus = PANEL_ENTITIES;
    PanelEvent event;
    MEVENT mevent;
    memset(&mevent, 0, sizeof(mevent));

    // 1. Mouse wheel UP over Entity panel at (15, 10)
    mevent.x = 15;
    mevent.y = 10;
#ifdef BUTTON4_PRESSED
    mevent.bstate = BUTTON4_PRESSED;
#else
    mevent.bstate = 0x00010000;
#endif

    AppResult res =
        PanelInputProcessMouseEvent(&layout, &focus, &mevent, &event);
    TEST_ASSERT_EQ(res, APP_OK, "Scroll up should return APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_MOUSE_SCROLL,
                   "Should emit PANEL_EVENT_MOUSE_SCROLL");
    TEST_ASSERT_EQ(event.targetPanel, PANEL_ENTITIES,
                   "Target should be PANEL_ENTITIES");
    TEST_ASSERT_EQ(event.scrollDelta, -1, "Wheel up produces scrollDelta = -1");

    // 2. Mouse wheel DOWN over Inspector panel at (65, 12)
    mevent.x = 65;
    mevent.y = 12;
#ifdef BUTTON5_PRESSED
    mevent.bstate = BUTTON5_PRESSED;
#else
    mevent.bstate = 0x00200000;
#endif

    res = PanelInputProcessMouseEvent(&layout, &focus, &mevent, &event);
    TEST_ASSERT_EQ(res, APP_OK, "Scroll down should return APP_OK");
    TEST_ASSERT_EQ(event.type, PANEL_EVENT_MOUSE_SCROLL,
                   "Should emit PANEL_EVENT_MOUSE_SCROLL");
    TEST_ASSERT_EQ(event.targetPanel, PANEL_INSPECTOR,
                   "Target should be PANEL_INSPECTOR");
    TEST_ASSERT_EQ(event.scrollDelta, 1,
                   "Wheel down produces scrollDelta = +1");

    return 0;
}

/**
 * @brief Validates argument validation and error code returns.
 */
static int TestPanelInputInvalidArgs(void)
{
    PanelLayout layout;
    PanelLayoutCompute(100, 30, 0.35, 80, 24, &layout);
    PanelId focus = PANEL_ENTITIES;
    PanelEvent event;

    AppResult res = PanelInputProcess(NULL, &focus, 'q', &event);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL layout must return INVALID_ARGUMENT");

    res = PanelInputProcess(&layout, NULL, 'q', &event);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL inOutFocus must return INVALID_ARGUMENT");

    res = PanelInputProcess(&layout, &focus, 'q', NULL);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL outEvent must return INVALID_ARGUMENT");

    res = PanelInputProcessMouseEvent(&layout, &focus, NULL, &event);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL mevent must return INVALID_ARGUMENT");

    return 0;
}

int TestPanelInputRun(void)
{
    printf("[Panel Input Test Suite]\n");
    TEST_RUN(TestPanelInputKeyMapping);
    TEST_RUN(TestPanelInputFocusCycling);
    TEST_RUN(TestPanelInputMouseClick);
    TEST_RUN(TestPanelInputMouseScroll);
    TEST_RUN(TestPanelInputInvalidArgs);
    printf("Panel input test suite completed successfully.\n\n");
    return 0;
}
