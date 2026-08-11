#include "panel/terminal.h"
#include <string.h>
#include <assert.h>

void test_panel_result_to_string(void) {
    assert(strcmp(PanelResultToString(PANEL_OK), "PANEL_OK") == 0);
    assert(strcmp(PanelResultToString(PANEL_ERROR_TERMINAL_INIT_FAILED), "PANEL_ERROR_TERMINAL_INIT_FAILED") == 0);
    assert(strcmp(PanelResultToString(PANEL_ERROR_INVALID_ARGUMENT), "PANEL_ERROR_INVALID_ARGUMENT") == 0);
    assert(strcmp(PanelResultToString((PanelResult)999), "UNKNOWN_RESULT") == 0);
}

int main(void) {
    test_panel_result_to_string();
    return 0;
}
