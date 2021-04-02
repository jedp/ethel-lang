#ifndef __DEF_H
#define __DEF_H

#include <stdint.h>

#define True      1
#define False     0
#define Null      0

#define F_NONE    0x00
#define F_INT     0x01
#define F_FLT     0x02
#define F_CHR     0x04
#define F_STR     0x40
#define F_VAR     0x80

typedef uint8_t byte;
typedef uint32_t boolean;
typedef uint32_t dim_t;

typedef struct __attribute__((__packed__)) {
  dim_t size;
  byte data[];
} bytearray_t;

#endif

