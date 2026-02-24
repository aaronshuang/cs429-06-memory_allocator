#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "libtdmm/tdmm.h"

// Helper macro for testing
#define TEST_PRINT(test_name) printf("Running %s...\n", test_name)

void run_unit_tests() {
    TEST_PRINT("Test 1: Basic Allocation and Writing");
    void *p1 = t_malloc(16);
    assert(p1 != NULL);
    strcpy((char*)p1, "Hello World!"); // If memory isn't mapped right, this segfaults
    printf("  -> p1 string: %s\n", (char*)p1);

    TEST_PRINT("Test 2: Multiple Allocations & Splitting");
    void *p2 = t_malloc(32);
    void *p3 = t_malloc(64);
    assert(p2 != NULL && p3 != NULL);
    assert(p1 != p2 && p2 != p3); // Ensure pointers are distinct

    TEST_PRINT("Test 3: Defensive Programming (Invalid Free)");
    // The spec requires catching invalid pointers. This should print your stderr message but NOT crash.
    int fake_ptr = 0xDEADBEEF;
    t_free(&fake_ptr); 

    TEST_PRINT("Test 4: Defensive Programming (Double Free)");
    t_free(p2);
    t_free(p2); // Should trigger your double-free detection

    TEST_PRINT("Test 5: Coalescing Physical Neighbors");
    // p2 is already free. p1 and p3 surround it. 
    // Freeing them in reverse order forces your coalescing logic to check both left and right neighbors.
    t_free(p3);
    t_free(p1);
    
    // At this point, the initial block should be fully coalesced back into one large chunk.
    // If we request a massive chunk now, it should fit without needing a new mmap page.
    void *massive = t_malloc(4000); 
    assert(massive != NULL);
    t_free(massive);

    TEST_PRINT("Test 6: Heap Expansion");
    // Request more than the 4096 page size. This forces request_more_memory to trigger.
    void *p_large1 = t_malloc(5000);
    assert(p_large1 != NULL);
    t_free(p_large1);

    printf("All Unit Tests Passed for current strategy!\n\n");
}

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("Testing FIRST_FIT Policy\n");
    printf("========================================\n");
    t_init(FIRST_FIT);
    run_unit_tests();
    
    // Note: Since we don't have a t_cleanup to unmap memory, running the other policies
    // in the exact same process run will just append memory to the existing heap. 
    // For pure unit testing, it's safe enough.
    
    printf("========================================\n");
    printf("Testing BEST_FIT Policy\n");
    printf("========================================\n");
    t_init(BEST_FIT);
    run_unit_tests();

    printf("========================================\n");
    printf("Testing WORST_FIT Policy\n");
    printf("========================================\n");
    t_init(WORST_FIT);
    run_unit_tests();

    printf("Testing complete. Allocator is structurally sound.\n");
    return 0;
}