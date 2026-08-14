#include "flecs_explorer/http_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "flecs_explorer/common.h"

#define HTTP_DEFAULT_TIMEOUT_MS 2000L
#define HTTP_DEFAULT_CONNECT_TIMEOUT_MS 1000L
#define HTTP_DEFAULT_USER_AGENT "flecs-explorer/" APP_VERSION_STRING

struct HttpClient {
    CURL *curlHandle;
    long timeoutMs;
    long connectTimeoutMs;
    char userAgent[128];
};

/**
 * @brief Dynamic accumulation buffer passed as userdata to curl write callback.
 */
typedef struct DynamicBuffer {
    char *data;
    size_t size;
} DynamicBuffer;

/**
 * @brief libcurl write callback to dynamically grow and append incoming chunks.
 *
 * As HTTP data packets arrive, this callback dynamically expands the buffer
 * via realloc, null-terminates the string, and appends incoming bytes.
 *
 * @param contents Pointer to incoming byte chunk delivered by libcurl.
 * @param size     Element size in bytes (always 1 in libcurl).
 * @param nmemb    Number of elements in the incoming chunk.
 * @param userp    Pointer to caller's DynamicBuffer instance.
 * @return Number of bytes processed, or 0 on allocation failure to abort curl.
 */
static size_t WriteCallback(const void *contents, size_t size, size_t nmemb,
                            void *userp)
{
    const size_t totalBytes = size * nmemb;
    DynamicBuffer *const buffer = (DynamicBuffer *)userp;

    if (totalBytes == 0) {
        return 0;
    }

    char *const newPtr =
        (char *)realloc(buffer->data, buffer->size + totalBytes + 1);
    if (newPtr == NULL) {
        return 0;
    }

    buffer->data = newPtr;
    memcpy(buffer->data + buffer->size, contents, totalBytes);
    buffer->size += totalBytes;
    buffer->data[buffer->size] = '\0';

    return totalBytes;
}

AppResult HttpClientCreate(const HttpClientConfig *config,
                           HttpClient **outClient)
{
    if (outClient == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    HttpClient *client = (HttpClient *)calloc(1, sizeof(HttpClient));
    if (client == NULL) {
        return APP_ERROR_OUT_OF_MEMORY;
    }

    client->curlHandle = curl_easy_init();
    if (client->curlHandle == NULL) {
        free(client);
        return APP_ERROR_SYSTEM;
    }

    if (config != NULL) {
        client->timeoutMs =
            config->timeoutMs > 0 ? config->timeoutMs : HTTP_DEFAULT_TIMEOUT_MS;
        client->connectTimeoutMs = config->connectTimeoutMs > 0
                                       ? config->connectTimeoutMs
                                       : HTTP_DEFAULT_CONNECT_TIMEOUT_MS;
        if (config->userAgent != NULL && config->userAgent[0] != '\0') {
            snprintf(client->userAgent, sizeof(client->userAgent), "%s",
                     config->userAgent);
        } else {
            snprintf(client->userAgent, sizeof(client->userAgent), "%s",
                     HTTP_DEFAULT_USER_AGENT);
        }
    } else {
        client->timeoutMs = HTTP_DEFAULT_TIMEOUT_MS;
        client->connectTimeoutMs = HTTP_DEFAULT_CONNECT_TIMEOUT_MS;
        snprintf(client->userAgent, sizeof(client->userAgent), "%s",
                 HTTP_DEFAULT_USER_AGENT);
    }

    *outClient = client;
    return APP_OK;
}

AppResult HttpClientGet(HttpClient *client, const char *url,
                        HttpResponse *outResponse)
{
    if (client == NULL || url == NULL || outResponse == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    memset(outResponse, 0, sizeof(*outResponse));

    DynamicBuffer buffer = {
        .data = NULL,
        .size = 0,
    };

    curl_easy_reset(client->curlHandle);
    curl_easy_setopt(client->curlHandle, CURLOPT_URL, url);
    curl_easy_setopt(client->curlHandle, CURLOPT_TIMEOUT_MS, client->timeoutMs);
    curl_easy_setopt(client->curlHandle, CURLOPT_CONNECTTIMEOUT_MS,
                     client->connectTimeoutMs);
    curl_easy_setopt(client->curlHandle, CURLOPT_USERAGENT, client->userAgent);
    curl_easy_setopt(client->curlHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(client->curlHandle, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(client->curlHandle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(client->curlHandle, CURLOPT_FOLLOWLOCATION, 1L);

    const CURLcode curlRes = curl_easy_perform(client->curlHandle);
    if (curlRes != CURLE_OK) {
        if (buffer.data != NULL) {
            free(buffer.data);
            buffer.data = NULL;
        }
        snprintf(outResponse->errorMessage, sizeof(outResponse->errorMessage),
                 "%s", curl_easy_strerror(curlRes));
        return APP_ERROR_NETWORK;
    }

    long httpCode = 0;
    curl_easy_getinfo(client->curlHandle, CURLINFO_RESPONSE_CODE, &httpCode);
    outResponse->statusCode = (int)httpCode;

    if (buffer.data == NULL) {
        char *const emptyPayload = (char *)calloc(1, 1);
        if (emptyPayload == NULL) {
            return APP_ERROR_OUT_OF_MEMORY;
        }
        outResponse->body = emptyPayload;
        outResponse->bodySize = 0;
    } else {
        outResponse->body = buffer.data;
        outResponse->bodySize = buffer.size;
    }

    return APP_OK;
}

void HttpResponseFree(HttpResponse *response)
{
    if (response == NULL) {
        return;
    }

    if (response->body != NULL) {
        free(response->body);
        response->body = NULL;
    }

    memset(response, 0, sizeof(*response));
}

void HttpClientDestroy(HttpClient *client)
{
    if (client == NULL) {
        return;
    }

    if (client->curlHandle != NULL) {
        curl_easy_cleanup(client->curlHandle);
        client->curlHandle = NULL;
    }

    free(client);
}
