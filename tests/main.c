#include "common/test_common.h"
#include "json/test_json.h"
#include "panel/test_panel_input.h"
#include "panel/test_panel_layout.h"
#include "panel/test_panel_manager.h"
#include "panel/test_panel_mock.h"
#include "panel/test_panel_renderer.h"
#include "panel/test_panel_theme.h"
#include "panel/test_panel_window.h"
#include "rest/test_flecs_data.h"
#include "rest/test_flecs_session.h"
#include "rest/test_http_client.h"
#include "rest/test_rest.h"
#include "test_cli.h"
#include <stdio.h>
#include <string.h>

static const TestSuite kSuites[] = {
    {.featureName = "common",
     .description = "Common foundation & error mapping tests",
     .runFunc = TestCommonRun},
    {.featureName = "json",
     .description = "cJSON parsing, serialization, and memory lifecycle tests",
     .runFunc = TestJsonRun},
    {.featureName = "panel_layout",
     .description = "Pure panel layout partitioning and hit-testing tests",
     .runFunc = TestPanelLayoutRun},
    {.featureName = "panel_theme",
     .description = "Panel theme palette and curses color pair tests",
     .runFunc = TestPanelThemeRun},
    {.featureName = "panel_window",
     .description = "Panel subwindow allocation and resize management tests",
     .runFunc = TestPanelWindowRun},
    {.featureName = "panel_renderer",
     .description = "Dedicated panel renderer and drawing pipeline tests",
     .runFunc = TestPanelRendererRun},
    {.featureName = "panel_input",
     .description =
         "Panel keyboard mapping, focus cycling, and mouse decoder tests",
     .runFunc = TestPanelInputRun},
    {.featureName = "panel_manager",
     .description =
         "Panel manager facade, lifecycle, and signal handling tests",
     .runFunc = TestPanelManagerRun},
    {.featureName = "panel_mock",
     .description =
         "Mock ECS telemetry, entity tree, and component rendering tests",
     .runFunc = TestPanelMockRun},
    {.featureName = "http_client",
     .description = "libcurl HTTP GET client and dynamic buffer tests",
     .runFunc = TestHttpClientRun},
    {.featureName = "flecs_data",
     .description = "Flecs world data models and deep cloning tests",
     .runFunc = TestFlecsDataRun},
    {.featureName = "flecs_session",
     .description = "Flecs session background thread and sync engine tests",
     .runFunc = TestFlecsSessionRun},
    {.featureName = "rest",
     .description = "Unified REST subsystem integration and scenario tests",
     .runFunc = TestRestRun}};

static const size_t kSuiteCount = sizeof(kSuites) / sizeof(kSuites[0]);

static void PrintUsage(const char *progName)
{
    printf("Flecs Explorer Unified Test Runner\n\n");
    printf("Usage: %s [OPTIONS] [FEATURE_NAME]\n\n", progName);
    printf("Options:\n");
    printf("  --all, -a        Run all registered feature test suites "
           "(default)\n");
    printf("  --list, -l       List all available feature test suites\n");
    printf("  --help, -h       Display this help message\n\n");
    printf("Available Feature Suites:\n");
    for (size_t i = 0; i < kSuiteCount; ++i) {
        printf("  - %-16s : %s\n", kSuites[i].featureName,
               kSuites[i].description);
    }
    printf("\n");
}

static void ListSuites(void)
{
    printf("Registered Feature Test Suites (%zu total):\n", kSuiteCount);
    for (size_t i = 0; i < kSuiteCount; ++i) {
        printf("  - %-16s : %s\n", kSuites[i].featureName,
               kSuites[i].description);
    }
}

static int RunSuiteByName(const char *name)
{
    for (size_t i = 0; i < kSuiteCount; ++i) {
        if (strcmp(kSuites[i].featureName, name) == 0) {
            printf("Running test suite: %s (%s)\n", kSuites[i].featureName,
                   kSuites[i].description);
            int result = kSuites[i].runFunc();
            if (result == 0) {
                printf(">>> Suite '%s' PASSED <<<\n", name);
            } else {
                fprintf(stderr, ">>> Suite '%s' FAILED with code %d <<<\n",
                        name, result);
            }
            return result;
        }
    }
    fprintf(stderr, "Error: Unknown test suite '%s'.\n\n", name);
    ListSuites();
    return 1;
}

static int RunAllSuites(void)
{
    printf("========================================\n");
    printf("Running all test suites (%zu total)...\n", kSuiteCount);
    printf("========================================\n\n");

    int failedCount = 0;
    for (size_t i = 0; i < kSuiteCount; ++i) {
        printf("--> Starting Suite [%zu/%zu]: %s\n", i + 1, kSuiteCount,
               kSuites[i].featureName);
        int result = kSuites[i].runFunc();
        if (result != 0) {
            fprintf(stderr, "Suite '%s' FAILED.\n", kSuites[i].featureName);
            failedCount++;
        }
    }

    printf("========================================\n");
    if (failedCount == 0) {
        printf("ALL TEST SUITES PASSED (%zu/%zu).\n", kSuiteCount, kSuiteCount);
        printf("========================================\n");
        return 0;
    } else {
        fprintf(stderr, "TEST FAILURES: %d of %zu suites failed.\n",
                failedCount, kSuiteCount);
        printf("========================================\n");
        return 1;
    }
}

int main(int argc, char **argv)
{
    if (argc <= 1) {
        return RunAllSuites();
    }

    const char *arg = argv[1];
    if (strcmp(arg, "--all") == 0 || strcmp(arg, "-a") == 0) {
        return RunAllSuites();
    }
    if (strcmp(arg, "--list") == 0 || strcmp(arg, "-l") == 0) {
        ListSuites();
        return 0;
    }
    if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
        PrintUsage(argv[0]);
        return 0;
    }

    return RunSuiteByName(arg);
}
