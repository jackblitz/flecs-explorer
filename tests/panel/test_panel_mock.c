#include "test_panel_mock.h"

#include <stdio.h>
#include <string.h>

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/panel/panel_renderer.h"
#include "panel/panel_mock.h"
#include "test_cli.h"

/**
 * @brief Validates initialization of MockWorldData telemetry and entity tree.
 */
static int TestPanelMockDataInit(void)
{
    MockWorldData data;
    memset(&data, 0, sizeof(data));

    PanelMockWorldDataInit(&data);

    TEST_ASSERT_STR_EQ(data.worldName, "flecs_game_world",
                       "World name should match flecs_game_world");
    TEST_ASSERT_STR_EQ(data.serverUrl, "http://localhost:2772",
                       "Server URL should match localhost:2772");
    TEST_ASSERT_STR_EQ(data.status, "ONLINE", "Status should be ONLINE");
    TEST_ASSERT_EQ(data.entityCount, 1420, "Entity count should be 1420");
    TEST_ASSERT_EQ(data.tableCount, 48, "Table count should be 48");
    TEST_ASSERT_EQ(data.systemCount, 12, "System count should be 12");
    TEST_ASSERT(data.fps >= 59.0, "FPS should be approximately 60.0");
    TEST_ASSERT(data.totalEntities > 0, "Total entities should be > 0");

    // First entity should be Player with components
    TEST_ASSERT_STR_EQ(data.entities[0].name, "Player",
                       "First entity should be Player");
    TEST_ASSERT(data.entities[0].componentCount >= 3,
                "Player should have at least 3 components");
    TEST_ASSERT_STR_EQ(data.entities[0].components[0].name, "Position",
                       "First component of Player should be Position");

    return 0;
}

/**
 * @brief Validates entity selection indices and component properties.
 */
static int TestPanelMockSelectionBounds(void)
{
    MockWorldData data;
    PanelMockWorldDataInit(&data);

    TEST_ASSERT_EQ(data.selectedEntityIndex, 0,
                   "Initial selected index should be 0");
    TEST_ASSERT(data.selectedEntityIndex < data.totalEntities,
                "Selected index must be within totalEntities");

    // Check last entity
    int lastIdx = data.totalEntities - 1;
    TEST_ASSERT(lastIdx > 0, "Last index should be > 0");
    TEST_ASSERT(strlen(data.entities[lastIdx].name) > 0,
                "Last entity name must be non-empty");

    return 0;
}

/**
 * @brief Validates NULL pointer safety across mock render functions.
 */
static int TestPanelMockRenderContentNullSafe(void)
{
    MockWorldData data;
    PanelMockWorldDataInit(&data);

    PanelRenderer *renderer = NULL;
    PanelRendererCreate(&renderer);

    // Call with NULL pointers
    PanelMockRenderContent(NULL, NULL, PANEL_HEADER, &data, false);
    PanelMockRenderContent(renderer, NULL, PANEL_ENTITIES, NULL, true);
    PanelMockRenderContent(renderer, NULL, PANEL_INSPECTOR, &data, true);
    PanelMockRenderContent(renderer, NULL, PANEL_STATUS, &data, false);

    // Call with NULL window for each panel type
    for (int p = 0; p < PANEL_COUNT; ++p) {
        PanelMockRenderContent(renderer, NULL, (PanelId)p, &data, false);
    }

    PanelRendererDestroy(renderer);
    return 0;
}

int TestPanelMockRun(void)
{
    printf("[Panel Mock Test Suite]\n");
    TEST_RUN(TestPanelMockDataInit);
    TEST_RUN(TestPanelMockSelectionBounds);
    TEST_RUN(TestPanelMockRenderContentNullSafe);
    printf("Panel mock test suite completed successfully.\n\n");
    return 0;
}
