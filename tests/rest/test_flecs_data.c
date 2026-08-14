#include "test_flecs_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "flecs_explorer/rest/flecs_session.h"
#include "rest/flecs_data.h"
#include "test_cli.h"

/**
 * @brief Tests string mappings for all FlecsSessionStatus enum values.
 *
 * Why:
 * The TUI header and status bars render connection status text to users.
 * We must verify that:
 * 1. Every defined enum value maps to an intuitive, uppercase status string.
 * 2. Any out-of-range or unknown status value returns a non-NULL fallback
 * ("UNKNOWN").
 */
static int TestFlecsSessionStatusToString(void)
{
    TEST_ASSERT_STR_EQ(FlecsSessionStatusToString(FLECS_SESSION_OFFLINE),
                       "OFFLINE", "OFFLINE status string mapping");
    TEST_ASSERT_STR_EQ(FlecsSessionStatusToString(FLECS_SESSION_CONNECTING),
                       "CONNECTING", "CONNECTING status string mapping");
    TEST_ASSERT_STR_EQ(FlecsSessionStatusToString(FLECS_SESSION_ONLINE),
                       "ONLINE", "ONLINE status string mapping");
    TEST_ASSERT_STR_EQ(FlecsSessionStatusToString(FLECS_SESSION_RECONNECTING),
                       "RECONNECTING", "RECONNECTING status string mapping");

    const char *unknownStatus =
        FlecsSessionStatusToString((FlecsSessionStatus)999);
    TEST_ASSERT_NOT_NULL(unknownStatus,
                         "Unknown status must not return NULL pointer");
    TEST_ASSERT_STR_EQ(unknownStatus, "UNKNOWN",
                       "Unknown status must map to 'UNKNOWN'");

    return 0;
}

/**
 * @brief Tests memory cleanup and field zeroing semantics of
 * FlecsWorldDataFree.
 *
 * Why:
 * FlecsWorldData owns an internal cJSON AST tree. We must guarantee that:
 * 1. Calling FlecsWorldDataFree(NULL) is a safe no-op.
 * 2. Calling FlecsWorldDataFree on an uninitialized/zeroed struct does not
 * crash.
 * 3. Calling FlecsWorldDataFree on an active struct frees the cJSON tree and
 *    resets all fields (worldJson to NULL, status to OFFLINE, latency to 0)
 *    to prevent use-after-free or double-free bugs in the UI frame loop.
 */
static int TestFlecsWorldDataFreeNullSafety(void)
{
    FlecsWorldDataFree(NULL);

    FlecsWorldData data = {0};
    FlecsWorldDataFree(&data);
    TEST_ASSERT_NULL(data.worldJson, "worldJson should remain NULL");
    TEST_ASSERT_EQ(data.status, FLECS_SESSION_OFFLINE,
                   "status should be reset to OFFLINE");

    cJSON *const json = cJSON_CreateObject();
    TEST_ASSERT_NOT_NULL(json, "cJSON_CreateObject should succeed");
    cJSON_AddStringToObject(json, "name", "FlecsWorld");

    data.status = FLECS_SESSION_ONLINE;
    data.lastHttpStatus = 200;
    data.latencyMs = 15;
    data.lastPollTimeMs = 1000;
    data.worldJson = json;
    snprintf(data.statusMessage, sizeof(data.statusMessage), "Connected");

    FlecsWorldDataFree(&data);
    TEST_ASSERT_NULL(data.worldJson, "worldJson must be NULL after free");
    TEST_ASSERT_EQ(data.status, FLECS_SESSION_OFFLINE,
                   "status must be reset after free");
    TEST_ASSERT_EQ(data.lastHttpStatus, 0,
                   "lastHttpStatus must be reset after free");
    TEST_ASSERT_EQ(data.latencyMs, 0, "latencyMs must be reset after free");
    TEST_ASSERT_EQ(data.statusMessage[0], '\0',
                   "statusMessage must be empty after free");

    return 0;
}

/**
 * @brief Tests deep cloning of FlecsWorldData including nested cJSON ASTs.
 *
 * Why:
 * The background worker thread holds live world state while the UI thread
 * requests frame snapshots. The deep clone must create a completely independent
 * copy of the cJSON tree so that the UI can read, manipulate, or free its copy
 * without data races, dangling pointers, or memory corruption.
 */
static int TestFlecsWorldDataCloneDeepCopy(void)
{
    cJSON *const root = cJSON_CreateObject();
    TEST_ASSERT_NOT_NULL(root, "cJSON_CreateObject must succeed");
    cJSON_AddStringToObject(root, "world_name", "sandbox");
    cJSON_AddNumberToObject(root, "entity_count", 42);

    cJSON *const entities = cJSON_AddArrayToObject(root, "entities");
    cJSON *const e1 = cJSON_CreateObject();
    cJSON_AddStringToObject(e1, "name", "Player");
    cJSON_AddItemToArray(entities, e1);

    FlecsWorldData src = {
        .status = FLECS_SESSION_ONLINE,
        .lastHttpStatus = 200,
        .lastPollTimeMs = 5000,
        .latencyMs = 8,
        .worldJson = root,
    };
    snprintf(src.statusMessage, sizeof(src.statusMessage), "OK");

    FlecsWorldData dest = {0};
    const AppResult res = FlecsWorldDataClone(&src, &dest);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsWorldDataClone must succeed");

    TEST_ASSERT_EQ(dest.status, src.status, "status should match");
    TEST_ASSERT_EQ(dest.lastHttpStatus, src.lastHttpStatus,
                   "lastHttpStatus should match");
    TEST_ASSERT_EQ(dest.lastPollTimeMs, src.lastPollTimeMs,
                   "lastPollTimeMs should match");
    TEST_ASSERT_EQ(dest.latencyMs, src.latencyMs, "latencyMs should match");
    TEST_ASSERT_STR_EQ(dest.statusMessage, src.statusMessage,
                       "statusMessage should match");

    TEST_ASSERT_NOT_NULL(dest.worldJson, "dest worldJson must not be NULL");
    TEST_ASSERT(dest.worldJson != src.worldJson,
                "dest worldJson must be a distinct memory allocation");

    // Free the source data and verify destination remains fully intact and
    // valid
    FlecsWorldDataFree(&src);

    char *const renderedJson = cJSON_PrintUnformatted(dest.worldJson);
    TEST_ASSERT_NOT_NULL(renderedJson,
                         "dest worldJson should be valid and serializable");
    TEST_ASSERT(strstr(renderedJson, "Player") != NULL,
                "serialized JSON must contain cloned entity data");

    free(renderedJson);
    FlecsWorldDataFree(&dest);

    return 0;
}

/**
 * @brief Tests defensive argument validation for FlecsWorldDataClone.
 *
 * Why:
 * Verifies that passing NULL source or destination pointers returns
 * APP_ERROR_INVALID_ARGUMENT immediately without crashes.
 */
static int TestFlecsWorldDataCloneInvalidArguments(void)
{
    FlecsWorldData data = {0};
    AppResult res = FlecsWorldDataClone(NULL, &data);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL src must return invalid argument");

    res = FlecsWorldDataClone(&data, NULL);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL dest must return invalid argument");

    return 0;
}

/**
 * @brief Stress tests rapid repeated creation, deep cloning, and deallocation.
 *
 * Why:
 * Under continuous 100ms polling, thousands of snapshots are cloned and freed.
 * This test runs 500 consecutive deep clone cycles under AddressSanitizer to
 * prove 100% leak-free operation.
 */
static int TestFlecsWorldDataMemoryLifecycle(void)
{
    for (int i = 0; i < 500; ++i) {
        cJSON *const root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "iteration", i);
        cJSON_AddStringToObject(root, "status", "healthy");

        FlecsWorldData src = {
            .status = FLECS_SESSION_ONLINE,
            .lastHttpStatus = 200,
            .latencyMs = (uint64_t)i,
            .worldJson = root,
        };

        FlecsWorldData dest = {0};
        const AppResult res = FlecsWorldDataClone(&src, &dest);
        TEST_ASSERT_EQ(res, APP_OK, "Clone cycle must succeed");

        FlecsWorldDataFree(&src);
        FlecsWorldDataFree(&dest);
    }

    return 0;
}

int TestFlecsDataRun(void)
{
    printf("[Flecs World Data Model Test Suite]\n");
    TEST_RUN(TestFlecsSessionStatusToString);
    TEST_RUN(TestFlecsWorldDataFreeNullSafety);
    TEST_RUN(TestFlecsWorldDataCloneDeepCopy);
    TEST_RUN(TestFlecsWorldDataCloneInvalidArguments);
    TEST_RUN(TestFlecsWorldDataMemoryLifecycle);
    printf("Flecs world data test suite completed successfully.\n\n");
    return 0;
}
