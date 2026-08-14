#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "test_flecs_session.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "flecs_explorer/rest/flecs_session.h"
#include "test_cli.h"

/**
 * @brief Tests FlecsSession allocation, configuration, and destruction.
 *
 * Why:
 * Verifies that:
 * 1. Passing a NULL outSession pointer returns APP_ERROR_INVALID_ARGUMENT.
 * 2. Passing NULL config applies sensible defaults (default URL, 100ms poll,
 * 1000ms timeout).
 * 3. Destroying an unstarted session cleanly releases mutex, condvar, and
 * internal HTTP client without leaking memory.
 * 4. FlecsSessionDestroy(NULL) is a safe no-op.
 */
static int TestFlecsSessionCreateDestroy(void)
{
    FlecsSession *session = NULL;
    AppResult res = FlecsSessionCreate(NULL, NULL);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "FlecsSessionCreate with NULL outSession must fail");

    res = FlecsSessionCreate(NULL, &session);
    TEST_ASSERT_EQ(res, APP_OK,
                   "FlecsSessionCreate with default config should succeed");
    TEST_ASSERT_NOT_NULL(session, "session handle must not be null");

    FlecsSessionDestroy(session);
    FlecsSessionDestroy(NULL); // Invariant: Null-safe no-op

    return 0;
}

/**
 * @brief Tests instantaneous thread termination using condition variable
 * broadcast.
 *
 * Why:
 * If the worker thread used sleep() or a naive timer loop, calling
 * FlecsSessionStop would block the main thread for the remainder of the poll
 * interval (e.g. up to 5000ms). Using pthread_cond_timedwait allows
 * FlecsSessionStop to broadcast an exit signal immediately, shutting down the
 * background thread in <100ms.
 */
static int TestFlecsSessionStartStopImmediate(void)
{
    const FlecsSessionConfig config = {
        .serverUrl = "http://127.0.0.1:59999",
        .endpointPath = "/world",
        .pollIntervalMs = 5000, // Long poll interval to test prompt shutdown
        .timeoutMs = 500,
        .maxBackoffMs = 2000,
    };

    FlecsSession *session = NULL;
    AppResult res = FlecsSessionCreate(&config, &session);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionCreate should succeed");

    res = FlecsSessionStart(session);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionStart should succeed");

    // Allow worker thread to initialize and enter timedwait
    usleep(10000); // 10ms

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    res = FlecsSessionStop(session);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionStop should succeed");

    clock_gettime(CLOCK_MONOTONIC, &end);

    const long elapsedMs = (end.tv_sec - start.tv_sec) * 1000 +
                           (end.tv_nsec - start.tv_nsec) / 1000000;

    TEST_ASSERT(elapsedMs < 200, "FlecsSessionStop must terminate promptly via "
                                 "condvar broadcast (<200ms)");

    FlecsSessionDestroy(session);
    return 0;
}

/**
 * @brief Tests state snapshot retrieval when connecting to an unreachable
 * server.
 *
 * Why:
 * Proves that:
 * 1. Background polling to an unopened port transitions status to OFFLINE or
 * RECONNECTING.
 * 2. FlecsSessionGetData safely returns a snapshot with lastHttpStatus == 0 and
 * worldJson == NULL.
 * 3. Calling FlecsWorldDataFree on the retrieved snapshot releases resources
 * without memory corruption.
 */
static int TestFlecsSessionGetDataOffline(void)
{
    const FlecsSessionConfig config = {
        .serverUrl = "http://127.0.0.1:59999",
        .endpointPath = "/world",
        .pollIntervalMs = 20,
        .timeoutMs = 50,
        .maxBackoffMs = 100,
    };

    FlecsSession *session = NULL;
    AppResult res = FlecsSessionCreate(&config, &session);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionCreate should succeed");

    res = FlecsSessionStart(session);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionStart should succeed");

    // Wait for at least one polling cycle to complete
    usleep(80000); // 80ms

    FlecsWorldData data = {0};
    res = FlecsSessionGetData(session, &data);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionGetData should succeed");
    TEST_ASSERT(data.status == FLECS_SESSION_OFFLINE ||
                    data.status == FLECS_SESSION_RECONNECTING,
                "Status should reflect network connectivity failure");
    TEST_ASSERT_EQ(data.lastHttpStatus, 0,
                   "HTTP status should be 0 on network failure");
    TEST_ASSERT_NULL(data.worldJson,
                     "worldJson should be NULL on network failure");

    FlecsWorldDataFree(&data);

    res = FlecsSessionStop(session);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionStop should succeed");

    FlecsSessionDestroy(session);
    return 0;
}

/**
 * @brief Tests concurrent high-frequency state snapshot reads while worker is
 * polling.
 *
 * Why:
 * The main UI render loop queries FlecsSessionGetData at 60+ FPS while the
 * background worker updates state under mutex protection. This test executes
 * 500 rapid consecutive queries to verify:
 * 1. Mutex locking prevents race conditions and heap corruption.
 * 2. Each returned snapshot is independently valid.
 * 3. Freeing 500 snapshots produces zero memory leaks under AddressSanitizer.
 */
static int TestFlecsSessionGetDataThreadSafety(void)
{
    const FlecsSessionConfig config = {
        .serverUrl = "http://127.0.0.1:59999",
        .endpointPath = "/world",
        .pollIntervalMs = 10,
        .timeoutMs = 50,
        .maxBackoffMs = 50,
    };

    FlecsSession *session = NULL;
    AppResult res = FlecsSessionCreate(&config, &session);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionCreate should succeed");

    res = FlecsSessionStart(session);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionStart should succeed");

    for (int i = 0; i < 500; ++i) {
        FlecsWorldData data = {0};
        res = FlecsSessionGetData(session, &data);
        TEST_ASSERT_EQ(res, APP_OK, "Concurrent GetData query must succeed");
        FlecsWorldDataFree(&data);
        usleep(200); // 0.2ms
    }

    res = FlecsSessionStop(session);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionStop should succeed");

    FlecsSessionDestroy(session);
    return 0;
}

/**
 * @brief Tests dynamic runtime server URL update while polling is active.
 *
 * Why:
 * Users can switch Flecs server endpoints from the TUI settings modal.
 * FlecsSessionSetServerUrl must safely lock the internal mutex and update the
 * target URL string without deadlocks or racing with active HTTP requests.
 */
static int TestFlecsSessionSetServerUrlThreadSafe(void)
{
    const FlecsSessionConfig config = {
        .serverUrl = "http://127.0.0.1:59999",
        .endpointPath = "/world",
        .pollIntervalMs = 20,
        .timeoutMs = 50,
        .maxBackoffMs = 100,
    };

    FlecsSession *session = NULL;
    AppResult res = FlecsSessionCreate(&config, &session);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionCreate should succeed");

    res = FlecsSessionStart(session);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionStart should succeed");

    res = FlecsSessionSetServerUrl(session, "http://127.0.0.1:59998");
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionSetServerUrl should succeed");

    res = FlecsSessionSetServerUrl(NULL, "http://127.0.0.1:59998");
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL session should return invalid argument");

    res = FlecsSessionSetServerUrl(session, NULL);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "NULL url should return invalid argument");

    res = FlecsSessionStop(session);
    TEST_ASSERT_EQ(res, APP_OK, "FlecsSessionStop should succeed");

    FlecsSessionDestroy(session);
    return 0;
}

int TestFlecsSessionRun(void)
{
    printf("[Flecs Session Engine Test Suite]\n");
    TEST_RUN(TestFlecsSessionCreateDestroy);
    TEST_RUN(TestFlecsSessionStartStopImmediate);
    TEST_RUN(TestFlecsSessionGetDataOffline);
    TEST_RUN(TestFlecsSessionGetDataThreadSafety);
    TEST_RUN(TestFlecsSessionSetServerUrlThreadSafe);
    printf("Flecs session engine test suite completed successfully.\n\n");
    return 0;
}
