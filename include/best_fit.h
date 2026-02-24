#ifndef BEST_FIT_H
#define BEST_FIT_H

#include <stddef.h>
#include <stdbool.h>

typedef struct best_block {
    size_t size;
    bool is_free;
    
    struct best_block *left;
    struct best_block *right;
    struct best_block *parent; 
} best_block_t;

#define BEST_HEADER_SIZE sizeof(best_block_t)

int best_fit_init(size_t initial_size);
void* best_fit_malloc(size_t size);
void best_fit_free(void *ptr);

#endif