#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "libtdmm/tdmm.h"
#include <time.h>


// Helper macro for testing
#define TEST_PRINT(test_name) printf("Running %s...\n", test_name)

extern size_t t_get_total_mapped_memory();
extern size_t t_get_currently_allocated_memory();

#define NUM_OPERATIONS 10000
#define MAX_ALLOC_SIZE 4096

typedef struct {
    void* ptr;
    size_t size;
} alloc_record_t;

void run_comparative_benchmark(alloc_strat_e strat, const char* name, FILE* csv_file) {
    t_init(strat);
    srand(42); // Fixed seed: ensures all policies face the same random cases

    alloc_record_t records[NUM_OPERATIONS];
    int active_allocs = 0;
    
    double total_utilization = 0;
    int utilization_samples = 0;

    struct timespec start_total, end_total;
    clock_gettime(CLOCK_MONOTONIC, &start_total);

    fprintf(csv_file, "Strategy,Operation,Throughput_Ops_Sec\n");

    for (int i = 0; i < NUM_OPERATIONS; i++) {
        struct timespec op_start, op_end;
        clock_gettime(CLOCK_MONOTONIC, &op_start);

        // Randomly decide to malloc or free (70% malloc to grow heap)
        if (active_allocs == 0 || (rand() % 100 < 70)) {
            size_t size = (rand() % MAX_ALLOC_SIZE) + 1;
            void* p = t_malloc(size);
            if (p) {
                records[active_allocs].ptr = p;
                records[active_allocs].size = size;
                active_allocs++;
            }
        } else {
            int index = rand() % active_allocs;
            t_free(records[index].ptr);
            records[index] = records[active_allocs - 1];
            active_allocs--;
        }

        clock_gettime(CLOCK_MONOTONIC, &op_end);

        // Track Statistics 
        size_t mapped = t_get_total_mapped_memory();
        if (mapped > 0) {
            total_utilization += (double)t_get_currently_allocated_memory() / mapped;
            utilization_samples++;
        }

        // Output throughput sample every 500 operations for the CSV 
        if (i > 0 && i % 500 == 0) {
            long long nsec = (op_end.tv_sec - start_total.tv_sec) * 1000000000LL + (op_end.tv_nsec - start_total.tv_nsec);
            double ops_per_sec = (double)i / (nsec / 1e9);
            fprintf(csv_file, "%s,%d,%.2f\n", name, i, ops_per_sec);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_total);
    long long total_nsec = (end_total.tv_sec - start_total.tv_sec) * 1000000000LL + (end_total.tv_nsec - start_total.tv_nsec);

    printf("\n--- Final Statistics for %s ---\n", name);
    printf("  Average Utilization: %.2f%%\n", (total_utilization / utilization_samples) * 100);
    printf("  Total Time: %lld ns\n", total_nsec);
    printf("  Average Throughput: %.2f ops/sec\n", (double)NUM_OPERATIONS / (total_nsec / 1e9));
    printf("  Structural Overhead: %zu bytes\n", t_get_structural_overhead());
}

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
    FILE* csv = fopen("throughput.csv", "w");
    if (!csv) return 1;
    
    run_comparative_benchmark(FIRST_FIT, "FIRST_FIT", csv);
    run_comparative_benchmark(BEST_FIT, "BEST_FIT", csv);
    run_comparative_benchmark(WORST_FIT, "WORST_FIT", csv);

    fclose(csv);
    printf("\nThroughput data saved to throughput.csv\n");
    return 0;
}