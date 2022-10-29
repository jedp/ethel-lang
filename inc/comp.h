#ifndef __COMP_H
#define __COMP_H

#include "vm.h"

void cg_new(cg_t *cg);
void cg_byte(cg_t *cg, uint8_t b);
void cg_free(cg_t *cg);

#endif
