/*
 * Address Space Layout Randomization
 * Part 1A: Egghunter
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
uint64_t find_address(uint64_t low_bound, uint64_t high_bound) {
    for (uint64_t addr = low_bound; addr < high_bound; addr += PAGE_SIZE) {
        // TODO: Figure out if "addr" is the correct address or not.
        errno = 0;
        int check = access((char *)addr,F_OK);
        
        if (check == 0){
        printf("it was in first check!!!");
        return addr;
        } else {
            if (errno == EFAULT){
                continue;
            } else {
                printf("this is a last chceck\n\n");
                return addr;
            }
        }
    }
}
