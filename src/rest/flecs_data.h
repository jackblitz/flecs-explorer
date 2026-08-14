#pragma once

#include "flecs_explorer/common/common.h"
#include "flecs_explorer/rest/flecs_session.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Internal helper to perform an authentic deep clone of a FlecsWorldData
 * struct.
 *
 * Duplicates the cJSON AST using cJSON_Duplicate and copies all scalar
 * telemetry.
 *
 * @param src  Source data struct to clone from. Non-NULL.
 * @param dest Destination data struct to receive cloned data. Non-NULL.
 * @return APP_OK on success, APP_ERROR_INVALID_ARGUMENT on NULL arguments,
 *         or APP_ERROR_OUT_OF_MEMORY if cJSON duplication fails.
 */
AppResult FlecsWorldDataClone(const FlecsWorldData *src, FlecsWorldData *dest);

#ifdef __cplusplus
}
#endif
