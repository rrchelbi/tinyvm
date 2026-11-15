#ifndef TINYVM_TEST_HARNESS_H
#define TINYVM_TEST_HARNESS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Small dependency-free test harness. CHECK always evaluates its argument,
 * even in builds with NDEBUG, and reports the failing source line.
 */
#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__,   \
                    #condition);                                               \
            return EXIT_FAILURE;                                               \
        }                                                                      \
    } while (0)

#define CHECK_CONTAINS(haystack, needle)                                       \
    CHECK(strstr((haystack), (needle)) != NULL)

#define RUN_TEST(test_function)                                                \
    do {                                                                       \
        if ((test_function)() == EXIT_SUCCESS) {                               \
            printf("PASS: %s\n", #test_function);                              \
        } else {                                                               \
            fprintf(stderr, "FAIL: %s\n", #test_function);                     \
            failures++;                                                        \
        }                                                                      \
    } while (0)

#endif /* TINYVM_TEST_HARNESS_H */

