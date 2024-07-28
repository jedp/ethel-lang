#include <string.h>
#include <printf.h>
#include "../mem/mem.h"
#include "obj_arr.h"

obj_arr_t *obj_arr_new(const uint8_t *bytes, uint32_t length) {
    obj_arr_t *obj_arr = (obj_arr_t *) obj_new(sizeof(obj_arr_t), OBJ_TYPE_BYTEARRAY);

    uint8_t *arr_bytes = mem_realloc(NULL, 0, sizeof(uint8_t) + length);
    if (bytes == NULL) {
        memset(arr_bytes, 0, length);
    } else {
        memcpy(arr_bytes, bytes, length);
    }

    obj_arr->length = length;
    obj_arr->buflen = length;
    obj_arr->buf = arr_bytes;

    return obj_arr;
}
