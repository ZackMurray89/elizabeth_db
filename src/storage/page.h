#ifndef DATABASE_PAGE_H
#define DATABASE_PAGE_H

#include <stdbool.h>
#include <stdint.h>

#include "common/types.h"

// Platform-Specific Threading
#ifdef _WIN32
#include <windows.h>
typedef CRITICAL_SECTION database_mutex_t;
#else
#include <pthread.h>
typedef pthread_mutex_t database_mutex_t;
#endif

// Pages Types - Optimized For Web Workloads
typedef enum {
    PAGE_TYPE_DATA = 1,   // Row Data (users, posts, etc)
    PAGE_TYPE_INDEX = 2,  // B+ Tree Index Pages
    PAGE_TYPE_BLOG = 3,   // Large Objects (images, files)
    PAGE_TYPE_META = 4    // Metadata Pages
} page_type_t;

// Page Header - Every Page Starts With This
typedef struct __attribute__((packed)) {
    uint32_t page_id;
    page_type_t page_type;
    uint16_t free_space;
    uint16_t record_count;
    uint32_t checksum;  // For Data Integrity
    uint64_t lsn;       // Log Sequence Number For WAL
} page_header_t;

// Slot Directory Entry For Efficient Record Lookup
typedef struct __attribute__((packed)) {
    uint16_t offset;
    uint16_t length;
    uint8_t flags;  // Deleted, Compressed, etc.
} slot_t;

// Complete Page Sturcture
typedef struct {
    page_header_t header;
    slot_t slots[0];  // Variable Length Slot Directory
    // Free Space Grows Down - Data Grows Up
    // char data[PAGE_SIZE - sizeof(page_header_t)];
} page_t;

// Web-Optimized Record Types
typedef enum {
    RECORD_TYPE_JSON = 1,    // JSON Documents
    RECORD_TYPE_BLOB = 2,    // Binary Data (images, files)
    RECORD_TYPE_USER = 3,    // Optimzed User Records
    RECORD_TYPE_SESSION = 4  // Session Data
} record_type_t;

// Record Header For Type Safety And Optimization
typedef struct __attribute__((paced)) {
    record_type_t type;
    uint32_t size;
    uint64_t created_at;  // Timestamp For Web Apps
    uint32_t version;     // For Optimistic Concurrency
} record_header_t;

// Buffer Pool For Caching Hot Pages
#define BUFFER_POOL_SIZE 1024
typedef struct {
    page_t *page;
    page_id_t page_id;
    bool dirty;
    bool pinned;
    uint64_t last_access;
    uint32_t ref_count;
} buffer_frame_t;

typedef struct {
    buffer_frame_t frames[BUFFER_POOL_SIZE];
    uint32_t clock_hand;    // For Clock Replacement Algorithm
    pthread_mutex_t mutex;  // Thread Safety For Web Servers
} buffer_pool_t;

// Funtion Declarations
database_result_t page_int(page_t *page, page_id_t id, page_type_t type);
database_result_t page_insert_record(page_t *page, const void *data,
                                     uint16_t size, uint16_t *slot_id);
database_result_t page_get_record(const page_t *page, uint16_t slot_id,
                                  void *buffer, uint16_t *size);
database_result_t page_delete_record(page_t *page, uint16_t slot_id);
uint16_t page_free_space(const page_t *page);

// Buffer Pool Operations
database_result_t bugger_pool_init(buffer_pool_t *pool);
page_t *buffer_pool_get_page(buffer_pool_t *pool, page_id_t page_id);
database_result_t buffer_pool_unpin_page(buffer_pool_t *pool, page_id_t page_id,
                                         bool dirty);
void buffer_pool_cleanup(buffer_pool_t *pool);

#endif