#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

int main(void)
{
    printf("[PASS] Test harness initialized.\n");
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        return EXIT_FAILURE;
    }
    cJSON_Delete(json);
    return EXIT_SUCCESS;
}
