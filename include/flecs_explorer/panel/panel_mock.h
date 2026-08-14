#pragma once

/**
 * @file panel_mock.h
 * @brief Mock ECS data generator and visual testing harness for panels.
 *
 * Typical usage:
 * @code
 *     MockWorldData mockData;
 *     PanelMockWorldDataInit(&mockData);
 *     WINDOW *win = PanelManagerGetWindow(manager, PANEL_ENTITIES);
 *     PanelMockRenderContent(win, PANEL_ENTITIES, &mockData, true);
 * @endcode
 *
 * Purpose:
 * Provides realistic fake ECS telemetry, hierarchical entity lists, and
 * syntax-colored component inspection fields for testing and validating
 * the multi-pane UI layout before live Flecs REST integration.
 */

#include <stdbool.h>
#include <stdint.h>
#include <curses.h>

#include "flecs_explorer/panel/panel_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Mock ECS component property key-value pair.
 */
typedef struct MockComponent {
    char name[64];
    char jsonValue[512];
} MockComponent;

/**
 * @brief Mock ECS entity record with associated mock components.
 */
typedef struct MockEntity {
    char name[64];
    char path[128];
    uint64_t id;
    int componentCount;
    MockComponent components[8];
} MockEntity;

/**
 * @brief Complete snapshot of mock Flecs world state for UI testing.
 */
typedef struct MockWorldData {
    char worldName[64];
    char serverUrl[128];
    double fps;
    int entityCount;
    int tableCount;
    int systemCount;
    uint64_t latencyMs;
    const char *status;
    int selectedEntityIndex;
    int totalEntities;
    MockEntity entities[32];
} MockWorldData;

/**
 * @brief Populates a MockWorldData structure with realistic Flecs ECS data.
 *
 * @param outData Pointer to MockWorldData struct to initialize. Non-NULL.
 */
void PanelMockWorldDataInit(MockWorldData *outData);

/**
 * @brief Renders mock content inside the given panel window using Darcula
 * styling.
 *
 * @param win Curses WINDOW to render content into. Non-NULL.
 * @param panel PanelId identifying which view content to draw.
 * @param data Mock world data snapshot. Non-NULL.
 * @param isFocused true if this panel currently holds keyboard/mouse focus.
 */
void PanelMockRenderContent(WINDOW *win, PanelId panel,
                            const MockWorldData *data, bool isFocused);

#ifdef __cplusplus
}
#endif
