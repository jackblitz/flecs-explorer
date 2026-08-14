#pragma once

/**
 * @file flecs_session.h
 * @brief Thread-safe Flecs session data models and connection state management.
 *
 * Typical usage:
 * @code
 *     FlecsSessionConfig config = {
 *         .serverUrl = "http://localhost:2772",
 *         .endpointPath = "/world",
 *         .pollIntervalMs = 100,
 *         .timeoutMs = 1000,
 *         .maxBackoffMs = 2000
 *     };
 *     FlecsSession *session = NULL;
 *     if (FlecsSessionCreate(&config, &session) == APP_OK) {
 *         FlecsSessionStart(session);
 *
 *         FlecsWorldData data;
 *         if (FlecsSessionGetData(session, &data) == APP_OK) {
 *             if (data.status == FLECS_SESSION_ONLINE && data.worldJson) {
 *                 // Inspect parsed ECS world cJSON tree
 *             }
 *             FlecsWorldDataFree(&data); // Free frame clone
 *         }
 *
 *         FlecsSessionStop(session);
 *         FlecsSessionDestroy(session);
 *     }
 * @endcode
 *
 * Memory Ownership & Deep Cloning:
 * The background session polling loop manages internal state and AST objects.
 * When the UI thread queries for the latest world state via
 * FlecsSessionGetData, the session thread-safely duplicates the cJSON AST into
 * a caller-owned FlecsWorldData structure. The caller MUST free this struct
 * using FlecsWorldDataFree to prevent memory leaks.
 */

#include <stdbool.h>
#include <stdint.h>

#include "cJSON.h"

#include "flecs_explorer/common/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Connection health state machine states.
 */
typedef enum FlecsSessionStatus {
    FLECS_SESSION_OFFLINE = 0, /**< Server unreachable or connection refused */
    FLECS_SESSION_CONNECTING = 1, /**< Initial connection attempt in progress */
    FLECS_SESSION_ONLINE = 2, /**< Actively connected with 200 OK responses */
    FLECS_SESSION_RECONNECTING =
        3 /**< Connection dropped; retrying with backoff */
} FlecsSessionStatus;

/**
 * @brief Returns a static string representation of a FlecsSessionStatus.
 *
 * @param status The status enum value.
 * @return Static string ("OFFLINE", "CONNECTING", "ONLINE", "RECONNECTING",
 * "UNKNOWN").
 */
const char *FlecsSessionStatusToString(FlecsSessionStatus status);

/**
 * @brief Thread-safe snapshot of ECS world data and connection telemetry.
 *
 * Populated automatically by FlecsSessionGetData. The worldJson pointer is
 * heap-allocated and owned by this struct. Callers must release it via
 * FlecsWorldDataFree.
 */
typedef struct FlecsWorldData {
    FlecsSessionStatus status; /**< Current connection status */
    int lastHttpStatus;        /**< Last HTTP status code (e.g. 200, 404, 0) */
    uint64_t lastPollTimeMs;   /**< Monotonic timestamp of last poll in ms */
    uint64_t latencyMs;        /**< HTTP round-trip + parse latency in ms */
    cJSON *worldJson; /**< Cloned cJSON root object, or NULL if offline/error */
    char statusMessage[256]; /**< Diagnostic or error string */
} FlecsWorldData;

/**
 * @brief Frees dynamic memory held in a FlecsWorldData struct and resets
 * fields.
 *
 * Accepts NULL and is a safe no-op if data is NULL or worldJson is NULL.
 *
 * @param data Pointer to the FlecsWorldData struct to clean up.
 */
void FlecsWorldDataFree(FlecsWorldData *data);

/**
 * @brief Configuration parameters for initializing a FlecsSession.
 */
typedef struct FlecsSessionConfig {
    char serverUrl[256]; /**< Base server URL (e.g. "http://localhost:2772") */
    char endpointPath[256];  /**< Polling endpoint path (e.g. "/world") */
    uint32_t pollIntervalMs; /**< Poll period in ms (default: 100ms) */
    uint32_t timeoutMs;      /**< Request timeout in ms (default: 1000ms) */
    uint32_t maxBackoffMs;   /**< Max backoff on network failure in ms (default:
                                2000ms) */
} FlecsSessionConfig;

/**
 * @brief Opaque handle representing an active Flecs background polling session.
 */
typedef struct FlecsSession FlecsSession;

/**
 * @brief Allocates and initializes a new FlecsSession instance.
 */
AppResult FlecsSessionCreate(const FlecsSessionConfig *config,
                             FlecsSession **outSession);

/**
 * @brief Starts the background polling thread.
 */
AppResult FlecsSessionStart(FlecsSession *session);

/**
 * @brief Signals the background thread to stop and waits for it to join.
 */
AppResult FlecsSessionStop(FlecsSession *session);

/**
 * @brief Thread-safe: Copies the latest world data snapshot under mutex
 * protection.
 */
AppResult FlecsSessionGetData(FlecsSession *session, FlecsWorldData *outData);

/**
 * @brief Thread-safe: Dynamically updates the target server URL.
 */
AppResult FlecsSessionSetServerUrl(FlecsSession *session, const char *url);

/**
 * @brief Stops polling and releases all session and mutex resources.
 */
void FlecsSessionDestroy(FlecsSession *session);

#ifdef __cplusplus
}
#endif
