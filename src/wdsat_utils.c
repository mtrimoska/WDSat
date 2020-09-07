//
//  wdsat_utils.c
//  WDSat
//
//  Created by Gilles Dequen on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#include "wdsat_utils.h"

#ifndef __TH_DIGITS__
#define __TH_DIGITS__
/**
 * @var int ThreadDigits = 0;
 * @brief This variable stores the number of symbols needed to print
 * a thread number
 */
byte_t thread_digits = 0;
#endif

/**
* @fn int_t fast_int_log2(int_t v)
* @brief It return the number of digit needed to represent its parameter using binary representation
* @param v is the value to be processed
*/
int_t fast_int_log2(int_t v) {
    int_t c = 0;
    while(v) {
        ++c;
        v >>= 1;
    }
    return(c);
}

/**
 * @fn int_t fast_int_log10(int_t v)
 * @brief It return the number of digit needed to represent its parameter using decimal representation
 * @param v is the value to be processed
 */
int_t fast_int_log10(int_t v) {
    int_t c = 0;
    while(v) {
        ++c;
        v /= 10;
    }
    return(c);
}

inline void print_bin(const uint_t v) {
    uint_t const sz_v = ((uint_t) sizeof(v) << 3ULL);
    uint_t mask = 1ULL << (sz_v - 1);
    while(mask) {
        printf("%llu", (v & mask) ? 1ULL : 0ULL);
        mask >>= 1;
    }
}
