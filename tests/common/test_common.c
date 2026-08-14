#include "test_common.h"
#include "test_cli.h"
#include "flecs_explorer/common.h"

static int TestVersionConstants(void)
{
    TEST_ASSERT_EQ(APP_VERSION_MAJOR, 0, "APP_VERSION_MAJOR should be 0");
    TEST_ASSERT_EQ(APP_VERSION_MINOR, 1, "APP_VERSION_MINOR should be 1");
    TEST_ASSERT_EQ(APP_VERSION_PATCH, 0, "APP_VERSION_PATCH should be 0");
    TEST_ASSERT_STR_EQ(APP_VERSION_STRING, "0.1.0",
                       "APP_VERSION_STRING should match '0.1.0'");
    return 0;
}

static int TestAppResultToStringMappings(void)
{
    TEST_ASSERT_STR_EQ(AppResultToString(APP_OK), "Success",
                       "APP_OK string mapping");
    TEST_ASSERT_STR_EQ(AppResultToString(APP_ERROR_INVALID_ARGUMENT),
                       "Invalid argument", "INVALID_ARGUMENT string mapping");
    TEST_ASSERT_STR_EQ(AppResultToString(APP_ERROR_OUT_OF_MEMORY),
                       "Out of memory", "OUT_OF_MEMORY string mapping");
    TEST_ASSERT_STR_EQ(AppResultToString(APP_ERROR_IO), "I/O error",
                       "IO string mapping");
    TEST_ASSERT_STR_EQ(AppResultToString(APP_ERROR_NETWORK), "Network error",
                       "NETWORK string mapping");
    TEST_ASSERT_STR_EQ(AppResultToString(APP_ERROR_PARSE), "Parse error",
                       "PARSE string mapping");
    TEST_ASSERT_STR_EQ(AppResultToString(APP_ERROR_SYSTEM), "System error",
                       "SYSTEM string mapping");
    return 0;
}

static int TestAppResultToStringUnknown(void)
{
    const char *unknownPositive = AppResultToString((AppResult)999);
    TEST_ASSERT_NOT_NULL(unknownPositive,
                         "Unknown positive error should not return NULL");
    TEST_ASSERT_STR_EQ(unknownPositive, "Unknown error",
                       "Unknown positive error string");

    const char *unknownNegative = AppResultToString((AppResult)-1);
    TEST_ASSERT_NOT_NULL(unknownNegative,
                         "Unknown negative error should not return NULL");
    TEST_ASSERT_STR_EQ(unknownNegative, "Unknown error",
                       "Unknown negative error string");
    return 0;
}

int TestCommonRun(void)
{
    printf("[Common Module Test Suite]\n");
    TEST_RUN(TestVersionConstants);
    TEST_RUN(TestAppResultToStringMappings);
    TEST_RUN(TestAppResultToStringUnknown);
    printf("Common module test suite completed successfully.\n\n");
    return 0;
}
