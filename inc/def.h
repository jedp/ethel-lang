#ifndef __DEF_H
#define __DEF_H

#define True      1
#define False     0
#define Null      0

#define F_NONE    0x00
#define F_INT     0x01
#define F_FLT     0x02
#define F_CHR     0x04
#define F_STR     0x40
#define F_VAR     0x80

typedef unsigned char byte;
typedef unsigned int byte_block;
typedef unsigned int boolean;
typedef unsigned int dim_t;

typedef struct ByteArray {
  dim_t size;
  byte_block* blocks;
} byte_array_t;

#endif

