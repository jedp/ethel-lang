#ifndef __DEF_H
#define __DEF_H

#include <stdint.h>

#define True      1
#define False     0
#define Null      0

#define ETHEL_HEAP_SIZE_BYTES 16000
#define DICT_INIT_BUCKETS 16

// FNV (Fowlwer, Noll, Vo) FNV-1a 32-bit hash constants.
#define FNV32Prime 0x01000193
#define FNV32Basis 0x811C9DC5

enum flags {
  F_NONE           = 0x00,
  F_ASSIGNABLE     = 0x01,
  F_OVERWRITE      = 0x10,
  F_VAR            = 0x80,
};

typedef uint8_t byte;
typedef uint32_t boolean;
typedef uint32_t dim_t;

typedef struct __attribute__((__packed__)) {
  dim_t size;
  byte data[];
} bytearray_t;

#endif

