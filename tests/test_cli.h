#ifndef TEST_CLI_H
#define TEST_CLI_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct TestSuite {
    const char *featureName;
    const char *description;
    int (*runFunc)(void);
} TestSuite;

#define TEST_ASSERT(cond, msg)                                                 \
    do {                                                                       \
        if (!(cond)) {                                                         \
            fprintf(stderr, "  [FAIL] %s:%d: %s (condition: %s)\n", __FILE__,  \
                    __LINE__, (msg), #cond);                                   \
            return 1;                                                          \
        }                                                                      \
    } while (0)

#define TEST_ASSERT_EQ(a, b, msg)                                              \
    do {                                                                       \
        if ((a) != (b)) {                                                      \
            fprintf(stderr, "  [FAIL] %s:%d: %s (expected %ld, got %ld)\n",    \
                    __FILE__, __LINE__, (msg), (long)(b), (long)(a));          \
            return 1;                                                          \
        }                                                                      \
    } while (0)

#define TEST_ASSERT_DOUBLE_EQ(a, b, eps, msg)                                 \
    do {                                                                       \
        double diff = (double)(a) - (double)(b);                               \
        if (diff < 0.0) {                                                      \
            diff = -diff;                                                      \
        }                                                                      \
        if (diff > (double)(eps)) {                                            \
            fprintf(stderr,                                                    \
                    "  [FAIL] %s:%d: %s (expected %f, got %f, eps %f)\n",      \
                    __FILE__, __LINE__, (msg), (double)(b), (double)(a),       \
                    (double)(eps));                                            \
            return 1;                                                          \
        }                                                                      \
    } while (0)

#define TEST_ASSERT_STR_EQ(a, b, msg)                                          \
    do {                                                                       \
        const char *actualStr = (a);                                           \
        const char *expectedStr = (b);                                         \
        if (actualStr == NULL || expectedStr == NULL ||                        \
            strcmp(actualStr, expectedStr) != 0) {                             \
            fprintf(stderr,                                                    \
                    "  [FAIL] %s:%d: %s (expected \"%s\", got \"%s\")\n",      \
                    __FILE__, __LINE__, (msg),                                 \
                    expectedStr ? expectedStr : "<NULL>",                      \
                    actualStr ? actualStr : "<NULL>");                         \
            return 1;                                                          \
        }                                                                      \
    } while (0)

#define TEST_ASSERT_NOT_NULL(ptr, msg)                                         \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            fprintf(stderr, "  [FAIL] %s:%d: %s (pointer is NULL: %s)\n",      \
                    __FILE__, __LINE__, (msg), #ptr);                          \
            return 1;                                                          \
        }                                                                      \
    } while (0)

#define TEST_ASSERT_NULL(ptr, msg)                                             \
    do {                                                                       \
        if ((ptr) != NULL) {                                                   \
            fprintf(stderr, "  [FAIL] %s:%d: %s (pointer is not NULL: %s)\n",  \
                    __FILE__, __LINE__, (msg), #ptr);                          \
            return 1;                                                          \
        }                                                                      \
    } while (0)

#define TEST_RUN(testFunc)                                                     \
    do {                                                                       \
        printf("  - %s ... ", #testFunc);                                      \
        fflush(stdout);                                                        \
        int res = testFunc();                                                  \
        if (res == 0) {                                                        \
            printf("PASSED\n");                                                \
        } else {                                                               \
            printf("FAILED\n");                                                \
            return res;                                                        \
        }                                                                      \
    } while (0)

#endif /* TEST_CLI_H */
