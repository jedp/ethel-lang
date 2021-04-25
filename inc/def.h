#ifndef __DEF_H
#define __DEF_H

#include <stdint.h>

#define True      1
#define False     0
#define Null      0

enum flags {
  F_NONE   = 0x00,
  F_UNDEF  = 0x01,
  F_VAR    = 0x80,
};

typedef uint8_t byte;
typedef uint32_t boolean;
typedef uint32_t dim_t;

typedef struct __attribute__((__packed__)) {
  dim_t size;
  byte data[];
} bytearray_t;

#endif

