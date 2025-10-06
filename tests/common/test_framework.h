#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test Statistics
extern int tests_run;
extern int tests_passed;
extern int tests_failed;

// Color Output For Better Readability
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RESET "\033[0m"

// Test Macros
#define TEST_ASSERT(condition, message)                                  \
    do {                                                                 \
        tests_run++;                                                     \
        if (condition) {                                                 \
            tests_passed++;                                              \
            printf(COLOR_GREEN "✅ PASS" COLOR_RESET ": %s\n", message); \
        } else {                                                         \
            tests_failed++;                                              \
        printf(COLOR_RED "❌ FAIL" COLOR_RESET ": %s\n", message; \
               printf("  at %s:%d\n", __FILE__, __LINE__);               \
        }                                                                \
    } while (0)

#define TEST_ASSERT_EQ(expected, actual, message) \
    TEST_ASSERT((expected) == (actual), message)

#define TEST_ASSERT_NEQ(expected, actual, message) \
    TEST_ASSERT((expected) != (actual), message)

#define TEST_ASSERT_NULL(ptr, message) TEST_ASSERT((ptr) == NULL, message)

#define TEST_ASSERT_NOT_NULL(ptr, message) TEST_ASSERT((ptr) != NULL, message)

// Test Suite Macros

#define TEST_SUITE_START(name) \
    printf(COLOR_YELLOW "\n=== Running %s ===" COLOR_RESET "\n", name)

#define TEST_SUITE_END(name) \
    printf(COLOR_YELLOW "\n=== Finished %s ===" COLOR_RESET, "\n", name)

// Function Declaration Macro
#define DECLARE_TEST(name) void test_##name(void)

#define RUN_TEST(name)                           \
    do {                                         \
        printf("\nRunning Test_%s...\n", #name); \
        test_##name();                           \
    } while (0)

#endif
