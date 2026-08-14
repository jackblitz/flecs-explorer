#pragma once

/**
 * @file http_client.h
 * @brief Synchronous HTTP client wrapper around libcurl for Flecs REST API.
 *
 * Typical usage:
 * @code
 *     const HttpClientConfig config = {
 *         .timeoutMs = 2000,
 *         .connectTimeoutMs = 1000,
 *         .userAgent = "flecs-explorer/0.1.0"
 *     };
 *     HttpClient *client = NULL;
 *     AppResult result = HttpClientCreate(&config, &client);
 *     if (result != APP_OK) {
 *         return result;
 *     }
 *
 *     HttpResponse response = {0};
 *     result = HttpClientGet(client, "http://localhost:2772/world", &response);
 *     if (result == APP_OK && response.statusCode == 200) {
 *         // Use response.body (e.g. parse with cJSON)
 *     }
 *
 *     HttpResponseFree(&response);
 *     HttpClientDestroy(client);
 * @endcode
 *
 * Dynamic Buffer Rationale:
 * Flecs ECS world and query responses vary widely in size—from a few hundred
 * bytes to hundreds of kilobytes or megabytes depending on entity counts.
 * libcurl delivers data in chunks via CURLOPT_WRITEFUNCTION. The response
 * body dynamically reallocates buffer capacity as chunks arrive, guaranteeing
 * complete, uncorrupted JSON payloads without artificial fixed-buffer limits
 * while keeping baseline memory usage minimal.
 *
 * Thread safety:
 * An HttpClient instance wraps an internal CURL easy handle and is not
 * internally synchronized. Dedicated worker threads should own or synchronize
 * their client instances. Concurrent execution of separate HttpClient instances
 * on different threads is thread-safe.
 */

#include <stddef.h>
#include <stdint.h>

#include "flecs_explorer/common/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Encapsulates an HTTP response payload, status code, and diagnostics.
 *
 * Plain data type: The `body` pointer is heap-allocated by HttpClientGet
 * and owned by the caller. Callers MUST free it via HttpResponseFree.
 */
typedef struct HttpResponse {
    int statusCode; /**< HTTP status code (200, 404, 0 on network/connect error)
                     */
    char *body; /**< Null-terminated dynamic response body, owned by caller */
    size_t bodySize; /**< Length of body in bytes (excluding null terminator) */
    char errorMessage[256]; /**< Diagnostic error message if request failed */
} HttpResponse;

/**
 * @brief Configuration settings for initializing an HttpClient.
 */
typedef struct HttpClientConfig {
    long timeoutMs; /**< Total transfer timeout in ms (default: 2000ms) */
    long connectTimeoutMs; /**< Connect timeout in ms (default: 1000ms) */
    const char *userAgent; /**< Optional User-Agent header string */
} HttpClientConfig;

/**
 * @brief Opaque handle representing an initialized HTTP client.
 */
typedef struct HttpClient HttpClient;

/**
 * @brief Allocates and initializes an HTTP client instance wrapping libcurl.
 *
 * @param config    Pointer to configuration struct. If NULL, defaults are
 * applied.
 * @param outClient On success, receives an owning handle; release with
 * HttpClientDestroy. Untouched on failure.
 * @return APP_OK, APP_ERROR_INVALID_ARGUMENT, APP_ERROR_OUT_OF_MEMORY, or
 * APP_ERROR_SYSTEM.
 */
AppResult HttpClientCreate(const HttpClientConfig *config,
                           HttpClient **outClient);

/**
 * @brief Executes a synchronous HTTP GET request and populates outResponse.
 *
 * Dynamically allocates and grows outResponse->body as chunks arrive.
 *
 * @param client      Active HttpClient handle. Non-NULL.
 * @param url         Target HTTP URL. Non-NULL, null-terminated string.
 * @param outResponse On success or network error, receives populated response
 * data. Caller must release dynamic memory via HttpResponseFree.
 * @return APP_OK if request completed (including non-200 HTTP statuses),
 *         APP_ERROR_INVALID_ARGUMENT, APP_ERROR_OUT_OF_MEMORY, or
 * APP_ERROR_NETWORK.
 */
AppResult HttpClientGet(HttpClient *client, const char *url,
                        HttpResponse *outResponse);

/**
 * @brief Frees dynamic memory allocated within an HttpResponse struct and
 * resets fields.
 *
 * Accepts NULL and is a no-op if response is NULL or response->body is NULL.
 *
 * @param response Pointer to HttpResponse to clean up.
 */
void HttpResponseFree(HttpResponse *response);

/**
 * @brief Destroys an HttpClient instance and releases internal curl resources.
 *
 * Accepts NULL and is a no-op if client is NULL.
 *
 * @param client Handle to destroy.
 */
void HttpClientDestroy(HttpClient *client);

/* Compatibility aliases matching spec naming */
#define HttpClient_Create HttpClientCreate
#define HttpClient_Get HttpClientGet
#define HttpResponse_Free HttpResponseFree
#define HttpClient_Destroy HttpClientDestroy

#ifdef __cplusplus
}
#endif
