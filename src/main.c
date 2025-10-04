#include <stdio.h>
#include <stdlib.h>

#include "common/error.h"
#include "common/types.h"

int main(int argc, char *argv[]) {
    printf("NexSQL v1.0.0\n");

    if (argc > 1) {
        printf("Database File: %s\n", argv[1]);
    } else {
        printf("Usage: %s <database_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    DEBUG_PRINT("Starting Database Engine");

    return EXIT_SUCCESS;
}