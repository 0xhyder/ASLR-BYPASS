/*
 * Address Space Layout Randomization
 * Part 1B: Prefetch
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <mmintrin.h>
#include <xmmintrin.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <x86intrin.h>
#include "lab.h"

/*
 * Part 1
 * Find and return the single mapped address within the range [low_bound, upper_bound).
 */
uint64_t get_time_start() {
    unsigned int aux;
    uint64_t start;
    _mm_mfence(); // Ensure previous ops are done
    _mm_lfence(); // Ensure subsequent ops wait
    start = __rdtscp(&aux);
    _mm_lfence(); // Ensure start is read before next ops
    return start;
}

uint64_t get_time_end() {
    unsigned int aux;
    uint64_t end;
    _mm_lfence(); // Ensure ops before end have run
    end = __rdtscp(&aux);
    _mm_mfence(); // Ensure end is read before next ops are visible
    return end;
}


uint64_t find_address(uint64_t low_bound, uint64_t high_bound) {
   

    const int NUM_MEASUREMENTS = 10;
    const int NUM_ADDRESSES = (high_bound - low_bound) / PAGE_SIZE;

    uint64_t *deltas = malloc(NUM_ADDRESSES * NUM_MEASUREMENTS * sizeof(uint64_t));
    if (!deltas) {
        printf("Memory allocation failed\n");
        return 0;
    }

    int delta_index = 0;
    for (uint64_t addr = low_bound; addr < high_bound; addr += PAGE_SIZE) {
        for (int i = 0; i < NUM_MEASUREMENTS; i++) {
            uint64_t start, end;
            
            start = get_time_start();
            
            _mm_prefetch((void*)addr, _MM_HINT_T0);
            end = get_time_end();

            deltas[delta_index++] = end - start;
        }
    }

    uint64_t overall_min_delta = UINT64_MAX;
    uint64_t address_with_min_delta = 0;
    delta_index = 0;

    for (uint64_t addr = low_bound; addr < high_bound; addr += PAGE_SIZE) {
        uint64_t min_delta_for_addr = UINT64_MAX;
        uint64_t sum_delta = 0;

        for (int i = 0; i < NUM_MEASUREMENTS; i++) {
            uint64_t current_delta = deltas[delta_index++];
            if (current_delta < min_delta_for_addr) {
                min_delta_for_addr = current_delta;
            }
            sum_delta += current_delta;
        }
        uint64_t avg_delta = sum_delta / NUM_MEASUREMENTS;

       
        // printf("Address: %p | Min Delta: %lu | Avg Delta: %lu\n", (void*)addr, min_delta_for_addr, avg_delta);

        
        if (min_delta_for_addr < overall_min_delta) {
            overall_min_delta = min_delta_for_addr;
            address_with_min_delta = addr;
        }
    }

    printf("\n--- Overall Lowest Delta Found ---\n");
    printf("Lowest Delta Value: %lu cycles\n", overall_min_delta);
    printf("Corresponding Address: %p\n", (void*)address_with_min_delta);
    
    free(deltas);
    return address_with_min_delta;
}
