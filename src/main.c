#include <stdio.h>
#include <string.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_manager.h"
#include "flecs_explorer/panel/panel_mock.h"
#include "flecs_explorer/panel/panel_renderer.h"
#include "panel/panel_signals.h"

/**
 * Prints CLI usage instructions.
 *
 * @param progName Executable binary name.
 */
static void PrintUsage(const char *progName)
{
    printf("Flecs Explorer v%s\n", APP_VERSION_STRING);
    printf("Real-time Terminal UI for Flecs ECS REST API\n\n");
    printf("Usage: %s [OPTIONS]\n\n", progName);
    printf("Options:\n");
    printf("  --help, -h       Display this help message and exit\n");
    printf("  --version, -v    Display version information and exit\n\n");
    printf("Keybindings:\n");
    printf("  Tab / Shift-Tab  Switch focus between Entity Browser and "
           "Inspector\n");
    printf("  j / k / Up / Dn  Navigate entity list\n");
    printf("  Mouse Click      Focus panel / select entity\n");
    printf("  Mouse Scroll     Scroll list / navigate entities\n");
    printf("  q / F10          Quit application\n");
}

/**
 * Main application entry point for the Flecs Explorer interactive UI.
 *
 * @param argc Command line argument count.
 * @param argv Command line argument array.
 * @return 0 on clean exit, non-zero on error.
 */
int main(int argc, char **argv)
{
    
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            PrintUsage(argv[0]);
            return 0;
        }
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            printf("flecs_explorer version %s\n", APP_VERSION_STRING);
            return 0;
        }
    }

    const PanelConfig config = {.minWidth = 80,
                                .minHeight = 24,
                                .entityPanelRatio = 0.35,
                                .enableMouse = true,
                                .enableColors = true,
                                .useDarculaTheme = true};

    PanelManager *manager = NULL;
    AppResult res = PanelManagerCreate(&config, &manager);
    if (res != APP_OK) {
        fprintf(stderr, "Error: Failed to initialize PanelManager (%d)\n", res);
        return 1;
    }

    res = PanelManagerInitTerminal(manager);
    if (res != APP_OK) {
        PanelManagerDestroy(manager);
        fprintf(stderr, "Error: Failed to initialize terminal (%d)\n", res);
        return 1;
    }

    PanelRenderer *renderer = NULL;
    res = PanelRendererCreate(&renderer);
    if (res != APP_OK) {
        PanelManagerDestroy(manager);
        fprintf(stderr, "Error: Failed to create PanelRenderer (%d)\n", res);
        return 1;
    }

    MockWorldData mockData;
    PanelMockWorldDataInit(&mockData);

    // Set 33ms non-blocking input timeout (~30 FPS refresh rate)
    timeout(33);

    bool running = true;
    while (running && !PanelSignalsReceivedQuit()) {
        if (PanelSignalsReceivedResize()) {
            PanelSignalsClearResize();
            PanelManagerHandleResize(manager);
        }

        if (PanelManagerIsTooSmall(manager)) {
            PanelRendererDrawTooSmallWarning(renderer, COLS, LINES,
                                             config.minWidth, config.minHeight);
            PanelRendererEndFrame(renderer);
        } else {
            PanelRendererBeginFrame(renderer);

            for (int p = 0; p < PANEL_COUNT; ++p) {
                WINDOW *win = PanelManagerGetWindow(manager, (PanelId)p);
                if (win == NULL) {
                    continue;
                }

                const bool isFocused =
                    (PanelManagerGetFocus(manager) == (PanelId)p);
                PanelRendererClearWindow(renderer, win);

                const char *title = NULL;
                if (p == PANEL_HEADER) {
                    title = " World Overview ";
                } else if (p == PANEL_ENTITIES) {
                    title = " Entities ";
                } else if (p == PANEL_INSPECTOR) {
                    title = " Component Inspector ";
                }

                if (p != PANEL_STATUS) {
                    PanelRendererDrawBorder(renderer, win, title, isFocused);
                }

                PanelMockRenderContent(renderer, win, (PanelId)p, &mockData,
                                       isFocused);
            }

            PanelRendererEndFrame(renderer);
        }

        const int ch = getch();
        if (ch != ERR) {
            PanelEvent event;
            res = PanelManagerProcessInput(manager, ch, &event);
            if (res == APP_OK) {
                if (event.type == PANEL_EVENT_QUIT) {
                    running = false;
                } else if (event.type == PANEL_EVENT_RESIZE) {
                    PanelManagerHandleResize(manager);
                } else if (event.type == PANEL_EVENT_KEY) {
                    if (event.key == 'j' || event.key == KEY_DOWN) {
                        mockData.selectedEntityIndex =
                            (mockData.selectedEntityIndex + 1) %
                            mockData.totalEntities;
                    } else if (event.key == 'k' || event.key == KEY_UP) {
                        mockData.selectedEntityIndex =
                            (mockData.selectedEntityIndex - 1 +
                             mockData.totalEntities) %
                            mockData.totalEntities;
                    }
                } else if (event.type == PANEL_EVENT_MOUSE_CLICK) {
                    if (event.targetPanel == PANEL_ENTITIES) {
                        const int clickedIdx = event.localY - 2;
                        if (clickedIdx >= 0 &&
                            clickedIdx < mockData.totalEntities) {
                            mockData.selectedEntityIndex = clickedIdx;
                        }
                    }
                } else if (event.type == PANEL_EVENT_MOUSE_SCROLL) {
                    if (event.scrollDelta > 0) {
                        mockData.selectedEntityIndex =
                            (mockData.selectedEntityIndex + 1) %
                            mockData.totalEntities;
                    } else if (event.scrollDelta < 0) {
                        mockData.selectedEntityIndex =
                            (mockData.selectedEntityIndex - 1 +
                             mockData.totalEntities) %
                            mockData.totalEntities;
                    }
                }
            }
        }
    }

    PanelRendererDestroy(renderer);
    PanelManagerDestroy(manager);

    return 0;
}
