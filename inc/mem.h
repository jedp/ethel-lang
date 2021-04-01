#ifndef __MEM_H
#define __MEM_H

#include "def.h"

void* mem_alloc(dim_t size);
void mem_set(void *b, int val, unsigned int len);

#endif

