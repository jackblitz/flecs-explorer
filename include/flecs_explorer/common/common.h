#ifndef FLECS_EXPLORER_COMMON_H
#define FLECS_EXPLORER_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define APP_VERSION_MAJOR 0
#define APP_VERSION_MINOR 1
#define APP_VERSION_PATCH 0
#define APP_VERSION_STRING "0.1.0"

typedef enum AppResult {
    APP_OK = 0,
    APP_ERROR_INVALID_ARGUMENT = 1,
    APP_ERROR_OUT_OF_MEMORY = 2,
    APP_ERROR_IO = 3,
    APP_ERROR_NETWORK = 4,
    APP_ERROR_PARSE = 5,
    APP_ERROR_SYSTEM = 6
} AppResult;

/**
 * @brief Returns a static, human-readable string description of an AppResult
 * code.
 *
 * @param result The result enum value.
 * @return A static, never-NULL string owned by the library.
 */
const char *AppResultToString(AppResult result);

#endif /* FLECS_EXPLORER_COMMON_H */
