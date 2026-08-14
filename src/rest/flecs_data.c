#include "flecs_explorer/rest/flecs_session.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

#include "flecs_explorer/common/common.h"
#include "rest/flecs_data.h"

const char *FlecsSessionStatusToString(FlecsSessionStatus status)
{
    switch (status) {
    case FLECS_SESSION_OFFLINE:
        return "OFFLINE";
    case FLECS_SESSION_CONNECTING:
        return "CONNECTING";
    case FLECS_SESSION_ONLINE:
        return "ONLINE";
    case FLECS_SESSION_RECONNECTING:
        return "RECONNECTING";
    default:
        return "UNKNOWN";
    }
}

void FlecsWorldDataFree(FlecsWorldData *data)
{
    if (data == NULL) {
        return;
    }

    if (data->worldJson != NULL) {
        cJSON_Delete(data->worldJson);
        data->worldJson = NULL;
    }

    memset(data, 0, sizeof(*data));
}

AppResult FlecsWorldDataClone(const FlecsWorldData *src, FlecsWorldData *dest)
{
    if (src == NULL || dest == NULL) {
        return APP_ERROR_INVALID_ARGUMENT;
    }

    memset(dest, 0, sizeof(*dest));

    dest->status = src->status;
    dest->lastHttpStatus = src->lastHttpStatus;
    dest->lastPollTimeMs = src->lastPollTimeMs;
    dest->latencyMs = src->latencyMs;
    snprintf(dest->statusMessage, sizeof(dest->statusMessage), "%s",
             src->statusMessage);

    if (src->worldJson != NULL) {
        dest->worldJson = cJSON_Duplicate(src->worldJson, 1);
        if (dest->worldJson == NULL) {
            return APP_ERROR_OUT_OF_MEMORY;
        }
    }

    return APP_OK;
}
