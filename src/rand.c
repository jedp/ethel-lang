#include "../inc/rand.h"

static unsigned long y = 2463534242;

void rand32_init(void) {
  y = 2463534242;
}

void rand32_seed(uint32_t seed) {
  y = seed + ((uint64_t) seed << 32);
}

/*
 * Xorshift RNGs, by George Marsaglia.
 * https://www.jstatsoft.org/v08/i14/paper
 *
 * From the paper:
 *
 *   [This] uses one of my favorite choices, [a, b, c] = [13, 17, 5], and will
 *   pass almost all tests of randomness, except the binary rank test in
 *   Diehard [2]. (A long period xorshift RNG necessarily uses a nonsingular
 *   matrix transformation, so every successive n vectors must be linearly
 *   independent, while truly random binary vectors will be linearly
 *   independent only some 30% of the time.)
 */
uint32_t rand32(void) {
  y ^= (y << 13);
  y ^= (y >> 17);
  y ^= (y << 5);

  return (uint32_t) y & 0xffffffff;
}

