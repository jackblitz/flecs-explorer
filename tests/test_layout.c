#include "panel/layout.h"
#include <assert.h>
#include <stddef.h>

void test_panel_layout_null_safety(void)
{
    assert(PanelLayoutCreate(NULL) == PANEL_ERROR_INVALID_ARGUMENT);
    PanelLayoutDestroy(NULL);
    PanelLayoutResize(NULL);
    PanelLayoutFocusCycle(NULL);
    assert(PanelLayoutActivePaneGet(NULL) == PANEL_PANE_ENTITY_TREE);
    PanelLayoutBordersRender(NULL);
}

void test_panel_layout_uninitialized_terminal(void)
{
    PanelLayout *layout = NULL;
    /* stdscr is NULL, so terminal size is 0x0 (< 24x80) */
    assert(PanelLayoutCreate(&layout) == PANEL_ERROR_TERMINAL_INIT_FAILED);
    assert(layout == NULL);
}

int main(void)
{
    test_panel_layout_null_safety();
    test_panel_layout_uninitialized_terminal();
    return 0;
}
