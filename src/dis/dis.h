#pragma once

#include "map.h"

typedef struct {
    map_t *consts;
    uint32_t length;
    uint32_t code_start;
    uint8_t *bytecode;
} dis_data_t;

uint32_t print_dis_byte(dis_data_t *dis, uint32_t offset);

void print_dis(dis_data_t *dis);
