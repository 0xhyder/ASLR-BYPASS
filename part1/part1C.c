/*
 * Address Space Layout Randomization
 * Part 1C: Speculative Probing
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

#define PROBE_PAGES 8 
/*
 * Part 1
 * Find and return the single mapped address within the range [low_bound, upper_bound).
 */

  // note:
  // this part of challenge is still incomplete, will update after complition.


uint64_t speculative_probing_gadget(uint64_t condition, (void*)guess, (void*)controlled_memory):
    if condition:
        // Access 1: Derefence the "guess" address (if it is mapped).
        idx = load(guess)

        // If guess was not mapped, we will crash here.
        // Hopefully all crashes happen under speculation so the program doesn't crash!

        // Access 2: Modify some controlled memory at an index dependent on the first load.
        // This only happens if the first load didn't crash, since the index is
        // a function of the contents of the first load.
        controlled_memory[idx] += 1 

uint64_t find_address(uint64_t low_bound, uint64_t high_bound) {
    for (uint64_t addr = low_bound; addr < high_bound; addr += PAGE_SIZE) {
        // TODO: Figure out if "addr" is the correct address or not.

        probe_array = malloc(0x1000);
    }

    return 0;
}
