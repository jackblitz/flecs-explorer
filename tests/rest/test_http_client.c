#define _POSIX_C_SOURCE 200809L

#include "test_http_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "flecs_explorer/rest/http_client.h"
#include "test_cli.h"

static int TestHttpClientCreateDestroy(void)
{
    HttpClient *client = NULL;
    AppResult res = HttpClientCreate(NULL, NULL);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "HttpClientCreate with null outClient must fail");

    const HttpClientConfig config = {.timeoutMs = 1500,
                                     .connectTimeoutMs = 500,
                                     .userAgent = "test-agent/1.0"};

    res = HttpClientCreate(&config, &client);
    TEST_ASSERT_EQ(res, APP_OK,
                   "HttpClientCreate with valid config should succeed");
    TEST_ASSERT_NOT_NULL(client, "client handle must not be null");

    HttpClientDestroy(client);
    HttpClientDestroy(NULL); // Null-safe no-op

    return 0;
}

static int TestHttpResponseFreeNullSafety(void)
{
    HttpResponseFree(NULL);

    HttpResponse response = {0};
    HttpResponseFree(&response);
    TEST_ASSERT_NULL(response.body, "response body should remain NULL");
    TEST_ASSERT_EQ(response.bodySize, 0, "response size should be 0");
    TEST_ASSERT_EQ(response.statusCode, 0, "status code should be 0");

    response.body = (char *)malloc(64);
    TEST_ASSERT_NOT_NULL(response.body, "malloc should succeed");
    strcpy(response.body, "test payload");
    response.bodySize = 12;
    response.statusCode = 200;

    HttpResponseFree(&response);
    TEST_ASSERT_NULL(response.body,
                     "response body must be reset to NULL after free");
    TEST_ASSERT_EQ(response.bodySize, 0,
                   "response bodySize must be reset to 0");

    return 0;
}

static int TestHttpClientInvalidArguments(void)
{
    HttpClient *client = NULL;
    AppResult res = HttpClientCreate(NULL, &client);
    TEST_ASSERT_EQ(res, APP_OK,
                   "HttpClientCreate with default config should succeed");

    HttpResponse response = {0};
    res = HttpClientGet(NULL, "http://127.0.0.1:59999", &response);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "Null client must return invalid argument");

    res = HttpClientGet(client, NULL, &response);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "Null url must return invalid argument");

    res = HttpClientGet(client, "http://127.0.0.1:59999", NULL);
    TEST_ASSERT_EQ(res, APP_ERROR_INVALID_ARGUMENT,
                   "Null response pointer must return invalid argument");

    HttpClientDestroy(client);
    return 0;
}

static int TestHttpClientConnectionFailure(void)
{
    const HttpClientConfig config = {
        .timeoutMs = 300, .connectTimeoutMs = 150, .userAgent = "test-agent"};

    HttpClient *client = NULL;
    AppResult res = HttpClientCreate(&config, &client);
    TEST_ASSERT_EQ(res, APP_OK, "HttpClientCreate should succeed");

    HttpResponse response = {0};
    // Port 59999 is unlikely to be open on loopback
    res =
        HttpClientGet(client, "http://127.0.0.1:59999/nonexistent", &response);
    TEST_ASSERT_EQ(
        res, APP_ERROR_NETWORK,
        "Connection to unopened port should return APP_ERROR_NETWORK");
    TEST_ASSERT_EQ(response.statusCode, 0,
                   "Failed connection should have statusCode 0");
    TEST_ASSERT_NULL(response.body, "Failed connection should have NULL body");
    TEST_ASSERT(strlen(response.errorMessage) > 0,
                "Error message must be populated on failure");

    HttpResponseFree(&response);
    HttpClientDestroy(client);
    return 0;
}

static int TestHttpClientTimeoutBehavior(void)
{
    const HttpClientConfig config = {.timeoutMs = 100,
                                     .connectTimeoutMs = 50,
                                     .userAgent = "test-agent/timeout"};

    HttpClient *client = NULL;
    AppResult res = HttpClientCreate(&config, &client);
    TEST_ASSERT_EQ(res, APP_OK, "HttpClientCreate should succeed");

    HttpResponse response = {0};
    // Use unroutable non-responsive test address (RFC 5737 TEST-NET-1:
    // 192.0.2.1)
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    res = HttpClientGet(client, "http://192.0.2.1:81/timeout", &response);
    clock_gettime(CLOCK_MONOTONIC, &end);

    const long elapsedMs = (end.tv_sec - start.tv_sec) * 1000 +
                           (end.tv_nsec - start.tv_nsec) / 1000000;

    TEST_ASSERT_EQ(res, APP_ERROR_NETWORK,
                   "Timeout should return network error");
    TEST_ASSERT(elapsedMs < 1000,
                "Timeout must abort promptly without hanging");

    HttpResponseFree(&response);
    HttpClientDestroy(client);
    return 0;
}

int TestHttpClientRun(void)
{
    printf("[HTTP Client Test Suite]\n");
    TEST_RUN(TestHttpClientCreateDestroy);
    TEST_RUN(TestHttpResponseFreeNullSafety);
    TEST_RUN(TestHttpClientInvalidArguments);
    TEST_RUN(TestHttpClientConnectionFailure);
    TEST_RUN(TestHttpClientTimeoutBehavior);
    printf("HTTP client test suite completed successfully.\n\n");
    return 0;
}
