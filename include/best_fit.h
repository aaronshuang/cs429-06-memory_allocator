#ifndef BEST_FIT_H
#define BEST_FIT_H

#include <stddef.h>
#include <stdbool.h>

typedef struct block_header {
    size_t size;
    bool is_free;

    struct block_header *next_free;
    struct block_header *prev_free;
} block_header_t;

#define HEADER_SIZE sizeof(block_header_t)

int best_fit_init(size_t initial_size);
void* best_fit_malloc(size_t size);
void best_fit_free(void *ptr);

#endif