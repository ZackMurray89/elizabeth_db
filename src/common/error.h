#ifndef DATABASE_ERROR_H
#define DATABASE_ERROR_H

#include <stdio.h>
#include <stdlib.h>

// Debug Macros
#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#else
#define DEBUG_PRINT(fmt, ...)
#endif

// Error Handling Macros
#define DATABASE_ASSERT(condition, message)                                      \
    do                                                                           \
    {                                                                            \
        if (!(condition))                                                        \
        {                                                                        \
            fprintf(stderr, "[FATAL] %s:%d: %s\n", __FILE__, __LINE__, message); \
            abort();                                                             \
        }                                                                        \
    } while (0)

#define DATABASE_CHECK_ALLOC(ptr) \
    DATABASE_ASSERT((ptr) != NULL, "Memory Allocation Failed")

#endif