#ifndef WORST_FIT_H
#define WORST_FIT_H

#include <stddef.h>
#include <stdbool.h>

typedef struct worst_block {
    size_t size;
    bool is_free;
    
    struct worst_block *child_left;
    struct worst_block *child_right;
} worst_block_t;

#define WORST_HEADER_SIZE sizeof(worst_block_t)

int worst_fit_init(size_t initial_size);
void* worst_fit_malloc(size_t size);
void worst_fit_free(void *ptr);

#endif