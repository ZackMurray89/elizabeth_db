#ifndef DATABASE_TYPES_H
#define DATABASE_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Database Configuration
#define PAGE_SIZE 4096
#define MAX_KEY_SIZE 255
#define MAX_VALUE_SIZE 1024

// Result Codes
typedef enum
{
    DATABASE_OK = 0,
    DATABASE_ERROR = -1,
    DATABASE_NOT_FOUND = -2,
    DATABASE_FULL = -3,
    DATABASE_INVALID_KEY = -4,
} database_result_t;

// Basic Data Types
typedef uint32_t page_id_t;
typedef uint64_t row_id_t;

#endif