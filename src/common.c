#include "flecs_explorer/common.h"

const char *AppResultToString(AppResult result)
{
    switch (result) {
    case APP_OK:
        return "Success";
    case APP_ERROR_INVALID_ARGUMENT:
        return "Invalid argument";
    case APP_ERROR_OUT_OF_MEMORY:
        return "Out of memory";
    case APP_ERROR_IO:
        return "I/O error";
    case APP_ERROR_NETWORK:
        return "Network error";
    case APP_ERROR_PARSE:
        return "Parse error";
    case APP_ERROR_SYSTEM:
        return "System error";
    default:
        return "Unknown error";
    }
}
