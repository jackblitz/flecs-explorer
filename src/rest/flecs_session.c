#define _POSIX_C_SOURCE 200809L

#include "flecs_explorer/rest/flecs_session.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON.h"

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/rest/http_client.h"
#include "rest/flecs_data.h"

#define FLECS_DEFAULT_SERVER_URL "http://localhost:2772"
#define FLECS_DEFAULT_ENDPOINT_PATH "/world"
#define FLECS_DEFAULT_POLL_INTERVAL_MS 100U
#define FLECS_DEFAULT_TIMEOUT_MS 1000U
#define FLECS_DEFAULT_MAX_BACKOFF_MS 2000U

struct FlecsSession {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t workerThread;
    bool isRunning;
    bool isThreadActive;

    FlecsSessionConfig config;
    HttpClient *httpClient;
    FlecsWorldData latestData;
};

/**
 * @brief Constructs target URL and checks if worker thread is still active.
 */
static bool SessionGetTargetUrl(FlecsSession *session, char *outUrl,
                                size_t maxLen)
{
    pthread_mutex_lock(&session->mutex);
    if (!session->isRunning) {
        pthread_mutex_unlock(&session->mutex);
        return false;
    }
    snprintf(outUrl, maxLen, "%s%s", session->config.serverUrl,
             session->config.endpointPath);
    pthread_mutex_unlock(&session->mutex);
    return true;
}

/**
 * @brief Computes next connection state and backoff based on prior status.
 */
static FlecsSessionStatus SessionComputeFailureStatus(FlecsSession *session,
                                                      uint32_t *inOutBackoffMs)
{
    pthread_mutex_lock(&session->mutex);
    const FlecsSessionStatus priorStatus = session->latestData.status;
    const uint32_t defaultInterval = session->config.pollIntervalMs;
    const uint32_t maxBackoff = session->config.maxBackoffMs;
    pthread_mutex_unlock(&session->mutex);

    if (*inOutBackoffMs == 0) {
        *inOutBackoffMs = defaultInterval;
    } else {
        *inOutBackoffMs = *inOutBackoffMs * 2;
        if (*inOutBackoffMs > maxBackoff) {
            *inOutBackoffMs = maxBackoff;
        }
    }

    return (priorStatus == FLECS_SESSION_ONLINE) ? FLECS_SESSION_RECONNECTING
                                                 : FLECS_SESSION_OFFLINE;
}

/**
 * @brief Executes a single HTTP poll, measures latency, and parses JSON
 * response.
 */
static bool SessionExecutePollCycle(FlecsSession *session,
                                    FlecsWorldData *outCycleData,
                                    uint32_t *inOutBackoffMs,
                                    uint32_t *outSleepMs)
{
    char fullUrl[512];
    if (!SessionGetTargetUrl(session, fullUrl, sizeof(fullUrl))) {
        return false;
    }

    struct timespec start;
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    HttpResponse response = {0};
    const AppResult httpRes =
        HttpClientGet(session->httpClient, fullUrl, &response);

    clock_gettime(CLOCK_MONOTONIC, &end);

    const uint64_t latencyMs =
        (uint64_t)((end.tv_sec - start.tv_sec) * 1000 +
                   (end.tv_nsec - start.tv_nsec) / 1000000);
    const uint64_t nowMs =
        (uint64_t)(end.tv_sec * 1000 + end.tv_nsec / 1000000);

    cJSON *parsedJson = NULL;
    FlecsSessionStatus newStatus = FLECS_SESSION_OFFLINE;
    char statusMsg[256] = {0};

    if (httpRes == APP_OK && response.statusCode == 200 &&
        response.body != NULL) {
        parsedJson = cJSON_Parse(response.body);
        if (parsedJson != NULL) {
            newStatus = FLECS_SESSION_ONLINE;
            snprintf(statusMsg, sizeof(statusMsg), "Online");
            *inOutBackoffMs = session->config.pollIntervalMs;
        } else {
            newStatus = FLECS_SESSION_RECONNECTING;
            snprintf(statusMsg, sizeof(statusMsg), "JSON parse error");
        }
    } else {
        if (response.errorMessage[0] != '\0') {
            snprintf(statusMsg, sizeof(statusMsg), "%s", response.errorMessage);
        } else {
            snprintf(statusMsg, sizeof(statusMsg), "HTTP %d",
                     response.statusCode);
        }
        newStatus = SessionComputeFailureStatus(session, inOutBackoffMs);
    }

    outCycleData->status = newStatus;
    outCycleData->lastHttpStatus = response.statusCode;
    outCycleData->lastPollTimeMs = nowMs;
    outCycleData->latencyMs = latencyMs;
    outCycleData->worldJson = parsedJson;
    snprintf(outCycleData->statusMessage, sizeof(outCycleData->statusMessage),
             "%s", statusMsg);

    *outSleepMs = (newStatus == FLECS_SESSION_ONLINE)
                      ? session->config.pollIntervalMs
                      : *inOutBackoffMs;

    HttpResponseFree(&response);
    return true;
}

/**
 * @brief Thread-safely updates session state with new poll cycle results.
 */
static bool SessionUpdateState(FlecsSession *session, FlecsWorldData *cycleData)
{
    pthread_mutex_lock(&session->mutex);
    if (!session->isRunning) {
        if (cycleData->worldJson != NULL) {
            cJSON_Delete(cycleData->worldJson);
            cycleData->worldJson = NULL;
        }
        pthread_mutex_unlock(&session->mutex);
        return false;
    }

    FlecsWorldDataFree(&session->latestData);
    session->latestData = *cycleData;
    pthread_mutex_unlock(&session->mutex);
    return true;
}

/**
 * @brief Sleeps for the requested interval using condition timedwait for
 * instant wake.
 */
static bool SessionTimedSleep(FlecsSession *session, uint32_t sleepMs)
{
    struct timespec sleepTs;
    clock_gettime(CLOCK_REALTIME, &sleepTs);
    sleepTs.tv_sec += (time_t)(sleepMs / 1000);
    sleepTs.tv_nsec += (long)((sleepMs % 1000) * 1000000L);
    if (sleepTs.tv_nsec >= 1000000000L) {
        sleepTs.tv_sec += 1;
        sleepTs.tv_nsec -= 1000000000L;
    }

    pthread_mutex_lock(&session->mutex);
    while (session->isRunning) {
        const int waitRes =
            pthread_cond_timedwait(&session->cond, &session->mutex, &sleepTs);
        if (waitRes != 0 || !session->isRunning) {
            break;
        }
    }
    const bool isStillRunning = session->isRunning;
    pthread_mutex_unlock(&session->mutex);
    return isStillRunning;
}

/**
 * @brief Background pthread routine for periodic decoupled REST polling.
 */
static void *WorkerThreadFunc(void *arg)
{
    FlecsSession *const session = (FlecsSession *)arg;
    uint32_t currentBackoffMs = 0;

    while (1) {
        FlecsWorldData cycleData = {0};
        uint32_t sleepMs = 0;

        if (!SessionExecutePollCycle(session, &cycleData, &currentBackoffMs,
                                     &sleepMs)) {
            break;
        }

        if (!SessionUpdateState(session, &cycleData)) {
            break;
        }

        if (!SessionTimedSleep(session, sleepMs)) {
            break;
        }
    }

    return NULL;
}

AppResult FlecsSessionCreate(const FlecsSessionConfig *config,
                             FlecsSession **outSession)
{
    if (outSession == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    FlecsSession *const session =
        (FlecsSession *)calloc(1, sizeof(FlecsSession));
    if (session == NULL) {
        return APP_ERROR_OUT_OF_MEMORY;
    }

    if (pthread_mutex_init(&session->mutex, NULL) != 0) {
        free(session);
        return APP_ERROR_SYSTEM;
    }

    if (pthread_cond_init(&session->cond, NULL) != 0) {
        pthread_mutex_destroy(&session->mutex);
        free(session);
        return APP_ERROR_SYSTEM;
    }

    if (config != NULL) {
        session->config = *config;
        if (session->config.serverUrl[0] == '\0') {
            snprintf(session->config.serverUrl,
                     sizeof(session->config.serverUrl), "%s",
                     FLECS_DEFAULT_SERVER_URL);
        }
        if (session->config.endpointPath[0] == '\0') {
            snprintf(session->config.endpointPath,
                     sizeof(session->config.endpointPath), "%s",
                     FLECS_DEFAULT_ENDPOINT_PATH);
        }
        if (session->config.pollIntervalMs == 0) {
            session->config.pollIntervalMs = FLECS_DEFAULT_POLL_INTERVAL_MS;
        }
        if (session->config.timeoutMs == 0) {
            session->config.timeoutMs = FLECS_DEFAULT_TIMEOUT_MS;
        }
        if (session->config.maxBackoffMs == 0) {
            session->config.maxBackoffMs = FLECS_DEFAULT_MAX_BACKOFF_MS;
        }
    } else {
        snprintf(session->config.serverUrl, sizeof(session->config.serverUrl),
                 "%s", FLECS_DEFAULT_SERVER_URL);
        snprintf(session->config.endpointPath,
                 sizeof(session->config.endpointPath), "%s",
                 FLECS_DEFAULT_ENDPOINT_PATH);
        session->config.pollIntervalMs = FLECS_DEFAULT_POLL_INTERVAL_MS;
        session->config.timeoutMs = FLECS_DEFAULT_TIMEOUT_MS;
        session->config.maxBackoffMs = FLECS_DEFAULT_MAX_BACKOFF_MS;
    }

    const HttpClientConfig httpConfig = {
        .timeoutMs = (long)session->config.timeoutMs,
        .connectTimeoutMs = (long)(session->config.timeoutMs / 2 > 0
                                       ? session->config.timeoutMs / 2
                                       : 50),
        .userAgent = "flecs-explorer/" APP_VERSION_STRING,
    };

    const AppResult httpRes =
        HttpClientCreate(&httpConfig, &session->httpClient);
    if (httpRes != APP_OK) {
        pthread_cond_destroy(&session->cond);
        pthread_mutex_destroy(&session->mutex);
        free(session);
        return httpRes;
    }

    *outSession = session;
    return APP_OK;
}

AppResult FlecsSessionStart(FlecsSession *session)
{
    if (session == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    pthread_mutex_lock(&session->mutex);
    if (session->isRunning) {
        pthread_mutex_unlock(&session->mutex);
        return APP_OK;
    }

    session->isRunning = true;
    const int threadRes =
        pthread_create(&session->workerThread, NULL, WorkerThreadFunc, session);
    if (threadRes != 0) {
        session->isRunning = false;
        pthread_mutex_unlock(&session->mutex);
        return APP_ERROR_SYSTEM;
    }

    session->isThreadActive = true;
    pthread_mutex_unlock(&session->mutex);

    return APP_OK;
}

AppResult FlecsSessionStop(FlecsSession *session)
{
    if (session == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    pthread_mutex_lock(&session->mutex);
    if (!session->isRunning && !session->isThreadActive) {
        pthread_mutex_unlock(&session->mutex);
        return APP_OK;
    }

    session->isRunning = false;
    pthread_cond_broadcast(&session->cond);
    const bool shouldJoin = session->isThreadActive;
    session->isThreadActive = false;
    pthread_mutex_unlock(&session->mutex);

    if (shouldJoin) {
        pthread_join(session->workerThread, NULL);
    }

    return APP_OK;
}

AppResult FlecsSessionGetData(FlecsSession *session, FlecsWorldData *outData)
{
    if (session == NULL || outData == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    pthread_mutex_lock(&session->mutex);
    const AppResult cloneRes =
        FlecsWorldDataClone(&session->latestData, outData);
    pthread_mutex_unlock(&session->mutex);

    return cloneRes;
}

AppResult FlecsSessionSetServerUrl(FlecsSession *session, const char *url)
{
    if (session == NULL || url == NULL || url[0] == '\0') {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    pthread_mutex_lock(&session->mutex);
    snprintf(session->config.serverUrl, sizeof(session->config.serverUrl), "%s",
             url);
    pthread_mutex_unlock(&session->mutex);

    return APP_OK;
}

void FlecsSessionDestroy(FlecsSession *session)
{
    if (session == NULL) {
        return;
    }

    FlecsSessionStop(session);

    pthread_mutex_lock(&session->mutex);
    FlecsWorldDataFree(&session->latestData);
    if (session->httpClient != NULL) {
        HttpClientDestroy(session->httpClient);
        session->httpClient = NULL;
    }
    pthread_mutex_unlock(&session->mutex);

    pthread_cond_destroy(&session->cond);
    pthread_mutex_destroy(&session->mutex);
    free(session);
}
