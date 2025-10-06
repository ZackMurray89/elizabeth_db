#include "storage/page.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common/error.h"

// Caclulate Checkum For Page Integrity
static uint32_t calculate_checksum(const page_t *page) {
    // Simple CRC32 - in production, use proper CRC library
    const uint8_t *data = (const uint8_t *)page;
    uint32_t crc = 0;
    size_t len = PAGE_SIZE - sizeof(uint32_t);

    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

database_result_t page_init(page_t *page, page_id_t id, page_type_t type) {
    DATABASE_ASSERT(page != NULL, "Page Cannon Be NULL");

    memset(page, 0, PAGE_SIZE);

    page->header.page_id = id;
    page->header.page_type = type;
    page->header.record_count = 0;
    page->header.lsn = 0;

    // Calcualte Available Space (page - header - some slots)
    page->header.free_space =
        PAGE_SIZE - sizeof(page_header_t) -
        (16 * sizeof(slot_t));  // Reserver space for 16 slots initially

    page->header.checksum = calculate_checksum(page);

    DEBUG_PRINT("Initialized page %u, type %d, free space: %u", id, type,
                page->header.free_space);

    return DATABASE_OK;
}

database_result_t page_insert_record(page_t *page, const void *data,
                                     uint16_t size, uint16_t *slot_id) {
    DATABASE_ASSERT(page != NULL && data != NULL && slot_id != NULL,
                    "Invalid Parameters");

    // Check If We Have Enough Space
    uint16_t needed_space = size + sizeof(slot_t);
    if (page->header.free_space < needed_space) {
        return DATABASE_FULL;
    }

    // Find Free Slot
    uint16_t slot_idx = page->header.record_count;

    // Calculate Data Offset (Grows From End Of Page Backwards)
    uint16_t data_offset = PAGE_SIZE - size;

    // Update Slot Directory
    slot_t *slots = (slot_t *)((char *)page + sizeof(page_header_t));
    slots[slot_idx].offset = data_offset;
    slots[slot_idx].length = size;
    slots[slot_idx].flags = 0;  // Active Record

    // Copy Data To Page
    char *page_data = (char *)page;
    memcpy(page_data + data_offset, data, size);

    // Update Page Header
    page->header.record_count++;
    page->header.free_space -= needed_space;
    page->header.checksum = calculate_checksum(page);

    *slot_id = slot_idx;

    DEBUG_PRINT("Inserted record at slot %u, size %u bytes", slot_idx, size);

    return DATABASE_OK;
}

database_result_t page_get_record(const page_t *page, uint16_t slot_id,
                                  void *buffer, uint16_t *size) {
    DATABASE_ASSERT(page != NULL && buffer != NULL && size != NULL,
                    "Invalid Parameters");

    if (slot_id >= page->header.record_count) {
        return DATABASE_NOT_FOUND;
    }

    const slot_t *slots =
        (const slot_t *)((const char *)page + sizeof(page_header_t));
    const slot_t *slot = &slots[slot_id];

    // Check If Record Is Deleted
    if (slot->flags & 0x01) {  // Deleted Flag
        return DATABASE_NOT_FOUND;
    }

    // Verify Buffer Size
    if (*size < slot->length) {
        *size = slot->length;
        return DATABASE_ERROR;
    }

    // Copy Data
    const char *page_data = (const char *)page;
    memcpy(buffer, page_data + slot->offset, slot->length);
    *size = slot->length;

    return DATABASE_OK;
}

uint16_t page_free_space(const page_t *page) { return page->header.free_space; }

// Buffer Pool Implementation For Web Perfomance
static buffer_pool_t *g_buffer_pool = NULL;

database_result_t buffer_pool_init(buffer_pool_t *pool) {
    DATABASE_ASSERT(pool != NULL, "Buffer Pool Cannot Be NULL");

    memset(pool, 0, sizeof(buffer_pool_t));

    // Initializae Mutex For Thread Safety (Important For Web Servers)
    if (pthread_mutex_init(&pool->mutex, NULL) != 0) {
        return DATABASE_ERROR;
    }

    // Allocate Pages
    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {
        pool->frames[i].page = aligned_alloc(PAGE_SIZE, PAGE_SIZE);
        DATABASE_CHECK_ALLOC(pool->frames[i].page);
        pool->frames[i].page_id = 0;  // Invalid Page ID
        pool->frames[i].dirty = false;
        pool->frames[i].pinned = false;
        pool->frames[i].ref_count = 0;
    }

    pool->clock_hand = 0;
    g_buffer_pool = pool;

    DEBUG_PRINT("Initialized Buffer Pool With %d Frames", BUFFER_POOL_SIZE);

    return DATABASE_OK;
}

page_t *buffer_pool_get_page(buffer_pool_t *pool, page_id_t page_id) {
    pthread_mutex_lock(&pool->mutex);

    // First, Check If Page Is Already In Buffer
    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {
        if (pool->frames[i].page_id == page_id &&
            pool->frames[i].ref_count > 0) {
            pool->frames[i].ref_count++;
            pool->frames[i].last_access = time(NULL);
            pthread_mutex_unlock(&pool->mutex);
            return pool->frames[i].page;
        }
    }

    // Find Free Frame Using Clock Algorithm
    while (true) {
        buffer_frame_t *frame = &pool->frames[pool->clock_hand];

        if (frame->ref_count == 0) {
            // Found Free Frame
            if (frame->dirty) {
                // TODO: Write Dirty Page To Disk
                DEBUG_PRINT("Writing Dirty Page %u To Disk", frame->page_id);
            }

            // TODO: Load Page From Disk
            page_init(frame->page, page_id, PAGE_TYPE_DATA);

            frame->page_id = page_id;
            frame->dirty = false;
            frame->pinned = true;
            frame->ref_count = 1;
            frame->last_access = time(NULL);

            pthread_mutex_unlock(&pool->mutex);

            return frame->page;
        }

        pool->clock_hand = (pool->clock_hand + 1) % BUFFER_POOL_SIZE;
    }

    pthread_mutex_unlock(&pool->mutex);

    return NULL;  // Should Not Reach Here In Well-Designed System
}

void buffer_pool_cleanup(buffer_pool_t *pool) {
    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {
        if (pool->frames[i].page) {
            free(pool->frames[i].page);
        }
    }
    pthread_mutex_destroy(&pool->mutex);
}
