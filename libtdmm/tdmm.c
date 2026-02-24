#include "tdmm.h"
#include "../include/first_fit.h"
#include "../include/best_fit.h"
#include "../include/worst_fit.h"

static alloc_strat_e current_strat;

void t_init(alloc_strat_e strat) {
	current_strat = strat;
    
    // Initializing with 1 page
    size_t initial_size = 4096; 
    
    if (strat == FIRST_FIT) first_fit_init(initial_size);
    else if (strat == BEST_FIT) best_fit_init(initial_size);
    else if (strat == WORST_FIT) worst_fit_init(initial_size);
}

void *t_malloc(size_t size) {
    if (current_strat == FIRST_FIT) return first_fit_malloc(size);
    if (current_strat == BEST_FIT) return best_fit_malloc(size);
    if (current_strat == WORST_FIT) return worst_fit_malloc(size);
    return NULL;
}

void t_free(void *ptr) {
    if (current_strat == FIRST_FIT) first_fit_free(ptr);
    else if (current_strat == BEST_FIT) best_fit_free(ptr);
    else if (current_strat == WORST_FIT) worst_fit_free(ptr);
}
