/*
 * A 32-bit LSR PNG as described by George Marsaglia in "Xorshift RNGs",
 * https://www.jstatsoft.org/v08/i14/paper
 */

#pragma once

#include <inttypes.h>

/*
 * Restore the LSR seed to its initial default.
 *
 * Exposed for testing.
 */
void rand32_init(void);

/* Reset the LSR seed to something based on the given seed. */
void rand32_seed(uint32_t seed);

/* Return a 32-bit pseudo-random number. */
uint32_t rand32(void);
