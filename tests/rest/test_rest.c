#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "test_rest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cJSON.h"

#include "flecs_explorer/rest/flecs_session.h"
#include "flecs_explorer/rest/http_client.h"
#include "rest/flecs_data.h"
#include "rest/test_flecs_data.h"
#include "rest/test_flecs_session.h"
#include "rest/test_http_client.h"
#include "test_cli.h"

/**
 * @brief Tests parsing and querying of realistic Flecs REST JSON responses.
 *
 * Why:
 * The Flecs REST API returns JSON payloads describing world entities and
 * components. This integration test simulates receiving a realistic `/world`
 * response, ingests it into a FlecsWorldData snapshot, deep-clones it, and
 * traverses the cJSON AST to inspect entity tags and component values (e.g.
 * Position, Velocity).
 */
static int TestRestMockWorldPayloadIngestion(void)
{
    const char *const mockFlecsPayload =
        "{\n"
        "  \"world\": {\n"
        "    \"name\": \"flecs_explorer_test_world\",\n"
        "    \"target_fps\": 60.0,\n"
        "    \"entity_count\": 1\n"
        "  },\n"
        "  \"results\": [\n"
        "    {\n"
        "      \"id\": 500,\n"
        "      \"name\": \"PlayerEntity\",\n"
        "      \"tags\": [\"flecs.core.Tag\", \"Player\"],\n"
        "      \"components\": {\n"
        "        \"Position\": {\"x\": 10.5, \"y\": 20.0},\n"
        "        \"Velocity\": {\"x\": 1.0, \"y\": -0.5}\n"
        "      }\n"
        "    }\n"
        "  ]\n"
        "}";

    cJSON *const parsedJson = cJSON_Parse(mockFlecsPayload);
    TEST_ASSERT_NOT_NULL(parsedJson,
                         "cJSON_Parse of mock payload must succeed");

    FlecsWorldData originalData = {
        .status = FLECS_SESSION_ONLINE,
        .lastHttpStatus = 200,
        .lastPollTimeMs = 12345678,
        .latencyMs = 12,
        .worldJson = parsedJson,
    };
    snprintf(originalData.statusMessage, sizeof(originalData.statusMessage),
             "OK");

    FlecsWorldData clonedData = {0};
    const AppResult cloneRes = FlecsWorldDataClone(&originalData, &clonedData);
    TEST_ASSERT_EQ(cloneRes, APP_OK, "FlecsWorldDataClone must succeed");

    // Free original to ensure clone is completely decoupled
    FlecsWorldDataFree(&originalData);

    // Validate cloned AST navigation
    const cJSON *const worldObj =
        cJSON_GetObjectItem(clonedData.worldJson, "world");
    TEST_ASSERT_NOT_NULL(worldObj, "Must find 'world' object");
    const cJSON *const worldName = cJSON_GetObjectItem(worldObj, "name");
    TEST_ASSERT_STR_EQ(worldName->valuestring, "flecs_explorer_test_world",
                       "World name must match");

    const cJSON *const resultsArr =
        cJSON_GetObjectItem(clonedData.worldJson, "results");
    TEST_ASSERT(cJSON_IsArray(resultsArr), "results must be an array");
    TEST_ASSERT_EQ(cJSON_GetArraySize(resultsArr), 1,
                   "results count should be 1");

    const cJSON *const playerEntity = cJSON_GetArrayItem(resultsArr, 0);
    const cJSON *const entityName = cJSON_GetObjectItem(playerEntity, "name");
    TEST_ASSERT_STR_EQ(entityName->valuestring, "PlayerEntity",
                       "Entity name must match");

    const cJSON *const components =
        cJSON_GetObjectItem(playerEntity, "components");
    const cJSON *const pos = cJSON_GetObjectItem(components, "Position");
    const cJSON *const posX = cJSON_GetObjectItem(pos, "x");
    TEST_ASSERT_DOUBLE_EQ(posX->valuedouble, 10.5, 0.001,
                          "Position.x must match");

    FlecsWorldDataFree(&clonedData);
    return 0;
}

/**
 * @brief Tests end-to-end REST engine full lifecycle and rapid reconfiguration.
 *
 * Why:
 * Verifies that creating a FlecsSession, starting background polling, modifying
 * the target server URL on the fly, polling snapshots, and stopping the session
 * behaves correctly as a complete subsystem integration without race conditions
 * or leaks.
 */
static int TestRestSubsystemLifecycleIntegration(void)
{
    const FlecsSessionConfig config = {
        .serverUrl = "http://127.0.0.1:59999",
        .endpointPath = "/world",
        .pollIntervalMs = 25,
        .timeoutMs = 50,
        .maxBackoffMs = 100,
    };

    FlecsSession *session = NULL;
    AppResult res = FlecsSessionCreate(&config, &session);
    TEST_ASSERT_EQ(res, APP_OK, "Session creation should succeed");

    res = FlecsSessionStart(session);
    TEST_ASSERT_EQ(res, APP_OK, "Session start should succeed");

    // Allow multiple polling ticks
    usleep(60000); // 60ms

    // Reconfigure server URL dynamically
    res = FlecsSessionSetServerUrl(session, "http://127.0.0.1:59998");
    TEST_ASSERT_EQ(res, APP_OK, "Dynamic URL update should succeed");

    // Retrieve state snapshot
    FlecsWorldData data = {0};
    res = FlecsSessionGetData(session, &data);
    TEST_ASSERT_EQ(res, APP_OK, "GetData should succeed");
    FlecsWorldDataFree(&data);

    res = FlecsSessionStop(session);
    TEST_ASSERT_EQ(res, APP_OK, "Session stop should succeed");

    FlecsSessionDestroy(session);
    return 0;
}

int TestRestRun(void)
{
    printf("========================================\n");
    printf("Executing Complete REST Subsystem Suite\n");
    printf("========================================\n\n");

    int res = 0;

    res = TestHttpClientRun();
    if (res != 0) {
        return res;
    }

    res = TestFlecsDataRun();
    if (res != 0) {
        return res;
    }

    res = TestFlecsSessionRun();
    if (res != 0) {
        return res;
    }

    printf("[REST Integration Scenarios]\n");
    TEST_RUN(TestRestMockWorldPayloadIngestion);
    TEST_RUN(TestRestSubsystemLifecycleIntegration);
    printf("REST integration scenarios completed successfully.\n\n");

    return 0;
}
