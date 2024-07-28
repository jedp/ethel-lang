#include <string.h>
#include "../mem/mem.h"
#include "obj_str.h"

obj_str_t *obj_str_new(const char *chars, uint32_t length) {
    obj_str_t *obj_str = (obj_str_t *) obj_new(sizeof(obj_str_t), OBJ_TYPE_STRING);

    char *str_chars = mem_realloc(NULL, 0, sizeof(char) + length);
    memcpy(str_chars, chars, length);

    obj_str->length = length;
    obj_str->chars = (char *) chars;

    return obj_str;
}

char *obj_str_to_c(obj_str_t *obj_str) {
    char *cstr = mem_realloc(NULL, 0, sizeof(char) + obj_str->length + 1);
    memcpy(cstr, obj_str->chars, obj_str->length);
    cstr[obj_str->length + 1] = '\0';
    return cstr;
}
