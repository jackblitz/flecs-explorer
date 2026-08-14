#include "flecs_explorer/common.h"
#include "cJSON.h"
#include <curl/curl.h>
#include <curses.h>
#include <stdio.h>

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("==================================================\n");
    printf(" Flecs Explorer v%s\n", APP_VERSION_STRING);
    printf(" Real-time Terminal UI for Flecs ECS REST API\n");
    printf("==================================================\n\n");

    printf("System Linkage & Environment:\n");
    printf("  - cJSON Version    : %s\n", cJSON_Version());
    printf("  - libcurl Version  : %s\n", curl_version());
    printf("  - ncurses Version  : %s\n", curses_version());
    printf("\nReady for initialization.\n");

    return (int)APP_OK;
}
