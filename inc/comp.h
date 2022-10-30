#ifndef __COMP_H
#define __COMP_H

#include "vm.h"

void cg_new(cg_t *cg);

void consts_new(obj_arr_t *consts);

void cg_free(cg_t *cg);

#endif
