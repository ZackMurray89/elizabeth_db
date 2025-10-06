cmake_minimum_required(VERSION 3.15)
project(elizabeth_db VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Compiler Flags
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic -fsanitize=address,undefined")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find pthread On Unix System
if(NOT WIN32)
  find_package(Threads REQUIRED)
endif()

# Main Executable
add_executable(database
    src/main.c
    src/storage/page.c
    src/storage/btree.c
    src/common/utils.c
)

target_include_directories(database PRIVATE include src)

# Link pthread On Unix System
if(NOT WIN32)
  target_link_libraries(database Threads::Threads)
endif()

# Test Executable
enable_testing()
add_executable(test_database
    tests/test_main.c
    tests/test_storage.c
    src/storage/page.c
    src/common/utils.c
)
target_include_directories(test_database PRIVATE include src)
add_test(NAME database_tests COMMAND test_database)
