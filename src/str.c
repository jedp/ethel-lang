#include <stdio.h>
#include "../inc/mem.h"
#include "../inc/obj.h"
#include "../inc/str.h"

dim_t c_str_len(const char* s) {
  dim_t size = 0;
  const unsigned char *p = (const unsigned char *) s;

  while (*p++ != 0) size++;
  return size;
}

boolean c_str_eq(const char* a, const char* b) {
  const unsigned char *p1 = (const unsigned char *) a;
  const unsigned char *p2 = (const unsigned char *) b;

  while(*p1) {
    if (*p1++ != *p2++) return False;
  }
  return (*p1 == *p2) ? True : False;
}

void c_str_cp(char* dst, const char* src) {
  unsigned char *p1 = (unsigned char *) dst;
  const unsigned char *p2 = (const unsigned char *) src;

  while((*p1++ = *p2++) != 0);
}

void c_str_ncp(char* dst, const char* src, dim_t n) {
  unsigned char *p1 = (unsigned char *) dst;
  const unsigned char *p2 = (const unsigned char *) src;

  while((n-- > 0) && (*p1++ = *p2++) != 0);
  // If we only copied n bytes, add the trailing null.
  if (*p1 != 0) *p1 = 0;
}

char* c_str_ncat(char *a, const char *b, dim_t n) {
  char* s = a;

  c_str_ncp(a + c_str_len(a), b, n);

  return s;
}

/* Convert the decimal value in 0..15 to a hex character. */
static byte hex_char(int v) {
  switch(v) {
    case 0:  return '0';
    case 1:  return '1';
    case 2:  return '2';
    case 3:  return '3';
    case 4:  return '4';
    case 5:  return '5';
    case 6:  return '6';
    case 7:  return '7';
    case 8:  return '8';
    case 9:  return '9';
    case 10: return 'a';
    case 11: return 'b';
    case 12: return 'c';
    case 13: return 'd';
    case 14: return 'e';
    case 15:
    default: return 'f';
  }
}

void fmt_hex(bytearray_t *a, dim_t start, dim_t end, int n) {

}

bytearray_t *bytearray_alloc(dim_t size) {
  bytearray_t *a = mem_alloc(size + 4);
  a->size = size;
  mem_set(a->data, '\0', size);
  return a;
}

char* bytearray_to_c_str(bytearray_t *a) {
  char* s = mem_alloc(a->size + 1);
  mem_cp(s, a->data, a->size);
  s[a->size] = '\0';
  return s;
}

bytearray_t *c_str_to_bytearray(const char* s) {
  dim_t size = c_str_len(s);
  bytearray_t *a = bytearray_alloc(size);
  mem_cp(a->data, (uint8_t*) s, size);
  return a;
}

obj_t *str_len(obj_t *str_obj, obj_method_args_t *args) {
  return int_obj(str_obj->bytearray->size);
}

obj_t *str_eq(obj_t *str_obj, obj_method_args_t *args) {
  return boolean_obj(False);
}

obj_t *arr_dump(obj_t *arr_obj, obj_method_args_t *args) {
  // Each row represents 16 bytes and is represented with 80 chars, e.g.,
  // 00000000  23 69 6e 63 6c 75 64 65  20 3c 73 74 64 69 6f 2e  |#include <stdio.|
  // 00000010  68 3e 0a                                          |h>.|
  // ^         ^                                                 ^                ^
  // 0        10                                                60               77
  //
  // We we need 79 bytes to store the representation for 16 bytes.
  dim_t byte_rows = (arr_obj->bytearray->size / 16) + ((arr_obj->bytearray->size % 16) ? 1 : 0);
  dim_t size = byte_rows * 79;
  bytearray_t *ba = bytearray_alloc(size);

  dim_t row = 0;
  for (dim_t i = 0; i < arr_obj->bytearray->size; i+=16) {
    // Source offset. 16 bytes per row.
    int place = 8;
    dim_t offset = i;
    while (place > 0) {
      byte hex = hex_char(offset & 0x0f);
      ba->data[row * 79 + place - 1] = hex;
      if (offset) offset >>= 4;
      place--;
    }

    // Gap after offset.
    ba->data[row * 79 + 8] = ' ';
    ba->data[row * 79 + 9] = ' ';

    // Content bytes in hex.
    int k = 0;
    for (int j = 0; j < 16 && i+j < arr_obj->bytearray->size; j++) {
      byte data_byte = arr_obj->bytearray->data[i + j];
      byte high = hex_char((data_byte & 0xf0) >> 4);
      byte low = hex_char(data_byte & 0x0f);
      ba->data[row * 79 + 10 + k + 0] = high;
      ba->data[row * 79 + 10 + k + 1] = low;
      ba->data[row * 79 + 10 + k + 2] = ' ';

      // More space after 8th byte.
      if ((j+1) % 8 == 0) {
        ba->data[row * 79 + 10 + k + 3] = ' ';
        k += 4;
      } else {
        k += 3;
      }
    }

    // Fill remaining or unused byte content area.
    while (k < 50) {
      ba->data[row * 79 + 10 + k++] = ' ';
    }

    // Printable bytes as chars.
    ba->data[row * 79 + 60] = '|';
    int j = 0;
    while(j < 16 && i+j < arr_obj->bytearray->size) {
      byte c = arr_obj->bytearray->data[i + j];
      ba->data[row * 79 + 61 + j] = ((c >= 0x21) && (c <= 0x7e) ) ? c : '.';
      j++;
    }
    ba->data[row * 79 + 61 + j] = '|';
    ba->data[row * 79 + 61 + j + 1] = '\n';
    row++;
  }

  return string_obj(ba);
}

static_method get_str_static_method(static_method_ident_t method_id) {
  switch(method_id) {
    case METHOD_LENGTH: return str_len;
    case METHOD_EQUALS: return str_eq;
    default: return NULL;
  }
}

