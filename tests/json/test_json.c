#include "test_json.h"
#include "test_cli.h"
#include "cJSON.h"
#include <stdlib.h>

static int TestAssertionsPass(void)
{
    TEST_ASSERT(1 == 1, "Identity check should pass");
    TEST_ASSERT_EQ(42, 42, "Integer equality should pass");
    TEST_ASSERT_STR_EQ("flecs_explorer", "flecs_explorer",
                       "String equality should pass");

    int dummy = 10;
    TEST_ASSERT_NOT_NULL(&dummy, "Non-null pointer check should pass");
    TEST_ASSERT_NULL(NULL, "Null pointer check should pass");
    return 0;
}

static int TestCJsonParseAndSerialize(void)
{
    const char *jsonText =
        "{\"name\":\"Position\",\"x\":10.5,\"y\":20.0,\"active\":true}";

    cJSON *root = cJSON_Parse(jsonText);
    TEST_ASSERT_NOT_NULL(root, "cJSON_Parse should return valid root object");

    cJSON *nameItem = cJSON_GetObjectItemCaseSensitive(root, "name");
    TEST_ASSERT_NOT_NULL(nameItem, "Should find 'name' item");
    TEST_ASSERT(cJSON_IsString(nameItem), "'name' item should be a string");
    TEST_ASSERT_STR_EQ(nameItem->valuestring, "Position",
                       "'name' value should match");

    cJSON *xItem = cJSON_GetObjectItemCaseSensitive(root, "x");
    TEST_ASSERT_NOT_NULL(xItem, "Should find 'x' item");
    TEST_ASSERT(cJSON_IsNumber(xItem), "'x' item should be a number");
    TEST_ASSERT(xItem->valuedouble == 10.5, "'x' value should match 10.5");

    cJSON *activeItem = cJSON_GetObjectItemCaseSensitive(root, "active");
    TEST_ASSERT_NOT_NULL(activeItem, "Should find 'active' item");
    TEST_ASSERT(cJSON_IsTrue(activeItem), "'active' item should be true");

    char *printed = cJSON_PrintUnformatted(root);
    TEST_ASSERT_NOT_NULL(printed, "cJSON_PrintUnformatted should succeed");

    free(printed);
    cJSON_Delete(root);
    return 0;
}

static int TestCJsonCreateAndModify(void)
{
    cJSON *obj = cJSON_CreateObject();
    TEST_ASSERT_NOT_NULL(obj, "cJSON_CreateObject should succeed");

    cJSON_AddStringToObject(obj, "app", "flecs-explorer");
    cJSON_AddNumberToObject(obj, "version_major", 0);
    cJSON_AddBoolToObject(obj, "ready", true);

    cJSON *versionItem = cJSON_GetObjectItemCaseSensitive(obj, "version_major");
    TEST_ASSERT_NOT_NULL(versionItem, "Version item should exist");
    TEST_ASSERT_EQ(versionItem->valueint, 0, "Version major should be 0");

    cJSON_Delete(obj);
    return 0;
}

int TestJsonRun(void)
{
    printf("[JSON Test Suite]\n");
    TEST_RUN(TestAssertionsPass);
    TEST_RUN(TestCJsonParseAndSerialize);
    TEST_RUN(TestCJsonCreateAndModify);
    printf("JSON test suite completed successfully.\n\n");
    return 0;
}
