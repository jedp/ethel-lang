#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "../inc/ptr.h"
#include "../inc/mem.h"
#include "../inc/obj.h"
#include "../inc/arr.h"
#include "../inc/math.h"
#include "../inc/str.h"

#define C_STR_BUF_SIZ 180
static char c_str_buf[C_STR_BUF_SIZ];

static void strip_trailing_ws(char* s) {
  int end = c_str_len(s) - 1;
  while (end > 0) {
    if (s[end] != ' '  &&
        s[end] != '\t' &&
        s[end] != '\n') {
      break;
    }
    // Convert trailing whitespace to null, and decrement length by one.
    s[end] = '\0';
    end--;
  }
}

static obj_t *_str_slice(obj_t *obj, int start, int end) {
  if (end > obj->bytearray->size) end = obj->bytearray->size;

  if (end < 0 ||
      start < 0 ||
      end < start) {
    printf("Bad range for slice\n");
    return string_obj(bytearray_alloc(0));
  }

  dim_t len = end - start;
  bytearray_t *ba = bytearray_alloc(len);
  for (int i = 0; i < end; i++) {
    ba->data[i] = obj->bytearray->data[start+i];
  }

  return string_obj(ba);
}

obj_t *str_hash(obj_t *obj, obj_t *args_obj) {
  return arr_hash(obj, args_obj);
}

obj_t *str_copy(obj_t *obj, obj_t *args_obj) {
  // string_obj copies the contents of the source.
  return string_obj(obj->bytearray);
}

obj_t *str_to_string(obj_t *obj, obj_t *args_obj) {
  return obj;
}

obj_t *str_contains(obj_t *obj, obj_t *args_obj) {
  return arr_contains(obj, args_obj);
}

obj_t *str_get(obj_t *obj, obj_t *args_obj) {
  return arr_get(obj, args_obj);
}

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

byte hex_char(int n) {
  switch(n) {
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

int hex_to_int(char* s) {
  int strlen = c_str_len(s);
  int val = 0;
  int j;
  for (int i = 0; i < 8 && i < strlen; i++) {
    switch(s[strlen - 1 - i]) {
      case '0': j =  0; break;
      case '1': j =  1; break;
      case '2': j =  2; break;
      case '3': j =  3; break;
      case '4': j =  4; break;
      case '5': j =  5; break;
      case '6': j =  6; break;
      case '7': j =  7; break;
      case '8': j =  8; break;
      case '9': j =  9; break;
      case 'a': j = 10; break;
      case 'b': j = 11; break;
      case 'c': j = 12; break;
      case 'd': j = 13; break;
      case 'e': j = 14; break;
      case 'f': j = 15; break;
      default:
        printf("'%c' in \"%s\"? what?\n", s[strlen - 1 - i], s);
        return -1;
    }
    val += (j << i*4);
  }

  return val;
}

int bin_to_int(char* s) {
  int strlen = c_str_len(s);
  int val = 0;
  int j;
  for (int i = 0; i < 32 && i < strlen; i++) {
    switch(s[strlen - 1 - i]) {
      case '0': j = 0; break;
      case '1': j = 1; break;
      default:
        printf("'%c' in \"%s\"? what?\n", s[strlen - 1 - i], s);
        return -1;
    }
    val += j << i;
  }

  return val;
}

bytearray_t *int_to_bin(unsigned int n) {
  int digits = 1;
  for (int i = 0; i < 32; i++) {
    if (n & (1 << i)) digits = i + 1;
  }
  bytearray_t *a = bytearray_alloc(digits + 2);

  a->data[0] = '0';
  a->data[1] = 'b';
  for (int i = 0; i < digits; i++) {
    a->data[digits + 1 - i] = ((n & (1 << i)) >> i) ? '1' : '0';
  }

  return a;
}

obj_t *str_to_int(obj_t *obj, obj_t *args_obj) {
  char *end = NULL;
  char *input = mem_alloc(obj->bytearray->size + 1);
  c_str_cp(input, bytearray_to_c_str(obj->bytearray));
  strip_trailing_ws(input);

  long l = strtol(input, &end, 10);
  // Expect to have read to the end of the string or to a decimal point.
  boolean bad_input = (*end != '\0') && (*end != '.');
  // Can only free input after we're done using the end pointer.
  mem_free(input);
  input = NULL;

  if (bad_input) return int_obj(0);
  if (l > INT_MAX) return int_obj(-1);
  if (l < INT_MIN) return int_obj(-1);

  return int_obj((int) l);
}

obj_t *str_to_byte(obj_t *obj, obj_t *args_obj) {
  char *end = NULL;
  char *input = mem_alloc(obj->bytearray->size + 1);
  c_str_cp(input, bytearray_to_c_str(obj->bytearray));
  strip_trailing_ws(input);

  long l = strtol(input, &end, 10);
  // Expect to have read to the end of the string or to a decimal point.
  boolean bad_input = (*end != '\0') && (*end != '.');
  // Can only free input after we're done using the end pointer.
  mem_free(input);
  input = NULL;

  if (bad_input) return byte_obj(0);

  return byte_obj(l & 0xff);
}

obj_t *str_to_float(obj_t *obj, obj_t *args_obj) {
  char *end = NULL;
  char *input = mem_alloc(obj->bytearray->size + 1);
  c_str_cp(input, bytearray_to_c_str(obj->bytearray));
  strip_trailing_ws(input);

  float f = strtof(input, &end);

  boolean bad_input = *end != '\0';
  mem_free(input);
  input = NULL;

  if (bad_input) return float_obj(-1);

  return float_obj(f);
}

bytearray_t *int_to_hex(unsigned int n) {
  int digits = 1;
  for (int i = 0; i < 8; i++) {
    if (n & (0xf << i*4)) digits = i + 1;
  }
  bytearray_t *a = bytearray_alloc(digits + 2);

  a->data[0] = '0';
  a->data[1] = 'x';
  for (int i = 0; i < digits; i++) {
    a->data[digits + 1 - i] = hex_char((n & (0xf << i*4)) >> i*4);
  }

  return a;
}

bytearray_t *bytearray_alloc(dim_t size) {
  bytearray_t *a = mem_alloc(size + 4);
  a->size = size;
  mem_set(a->data, '\0', size);
  return a;
}

bytearray_t *bytearray_clone(bytearray_t *src) {
  bytearray_t *dst = bytearray_alloc(src->size);
  dim_t i = 0;
  while (i < src->size) {
    dst->data[i] = src->data[i];
    i++;
  }
  return dst;
}

boolean bytearray_eq(bytearray_t *a, bytearray_t *b) {
  if (a->size != b->size) return False;

  for (int i = 0; i < a->size; i++) {
    if (a->data[i] != b->data[i]) return False;
  }

  return True;
}

boolean c_str_eq_bytearray(const char *s, bytearray_t *a) {
  if (c_str_len(s) != a->size) return False;

  for (int i = 0; i < a->size; i++) {
    if (s[i] != a->data[i]) return False;
  }

  return True;
}

char* bytearray_to_c_str(bytearray_t *a) {
  int size = min(a->size, C_STR_BUF_SIZ-1);
  mem_cp(c_str_buf, a->data, size);
  c_str_buf[size] = '\0';
  return c_str_buf;
}

bytearray_t *c_str_to_bytearray(const char* s) {
  dim_t size = c_str_len(s);
  bytearray_t *a = bytearray_alloc(size);
  mem_cp(a->data, (uint8_t*) s, size);
  return a;
}

obj_t *str_len(obj_t *obj, obj_t *args_obj) {
  return int_obj(obj->bytearray->size);
}

obj_t *str_eq(obj_t *obj, obj_t *args_obj) {
  return arr_eq(obj, args_obj);
}

obj_t *str_ne(obj_t *obj, obj_t *args_obj) {
  return arr_ne(obj, args_obj);
}

obj_t *str_as(obj_t *obj, obj_t *args_obj) {
  obj_method_args_t *args = args_obj->method_args;
  if (args == NULL || args->arg == NULL) return boolean_obj(False);
  obj_t *type_arg = args->arg;

  switch (type_arg->type) {
    case TYPE_STRING: return obj;
    case TYPE_INT: return str_to_int(obj, NULL);
    case TYPE_FLOAT: return str_to_float(obj, NULL);
    case TYPE_BOOLEAN: return boolean_obj(obj->bytearray->size ? True : False);
    default:
      printf("Cannot cast %s to type %s.\n",
             type_names[obj->type], type_names[type_arg->type]);
      return boolean_obj(False);
  }
}

obj_t *str_substring(obj_t *obj, obj_t *args_obj) {
  obj_method_args_t *args = args_obj->method_args;
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to substring()\n");
    return nil_obj();
  }

  obj_t *start_arg = args->arg;
  if (start_arg->type != TYPE_INT) return nil_obj();

  obj_t *end_arg = args->next->arg;
  if (end_arg->type != TYPE_INT)  return nil_obj();

  return _str_slice(obj, start_arg->intval, end_arg->intval);
}

obj_t *str_add(obj_t *obj, obj_t *args_obj) {
  obj_method_args_t *args = args_obj->method_args;
  if (args == NULL || args->arg == NULL) {
    printf("Null arg to add()\n");
    return obj;
  }

  obj_t *arg = args->arg;
  if (arg->type != TYPE_STRING) {
    printf("Cannot add %s to %s.\n",
           type_names[arg->type], type_names[obj->type]);
    return obj;
  }

  bytearray_t *new = bytearray_alloc(obj->bytearray->size + arg->bytearray->size);
  if (new == NULL) {
    printf("Couldn't allocate new string!\n");
    return obj;
  }
  for (dim_t i = 0; i < obj->bytearray->size; i++) {
    new->data[i] = obj->bytearray->data[i];
  }
  dim_t offset = obj->bytearray->size;
  for (dim_t i = 0; i < arg->bytearray->size; i++) {
    new->data[offset + i] = arg->bytearray->data[i];
  }

  bytearray_t *old = obj->bytearray;
  obj->bytearray = new;
  mem_free(old);
  old = NULL;

  return obj;
}

obj_t *str_random_choice(obj_t *obj, obj_t *args_obj) {
  return arr_random_choice(obj, args_obj);
}

obj_t *byte_dump(obj_t *byte_obj) {
  // 'x' -> "120  0x78  11111111"
  bytearray_t *a = bytearray_alloc(19);

  // Three-digit decimal representation.
  // Do not print leading zeroes, because that looks octal.
  int v = byte_obj->byteval;
  a->data[2] = '0';  // Edge case.
  for (int i = 0; i < 3; i++) {
    if (v) {
      a->data[2 - i] = '0' + (v % 10);
      v = v / 10;
    } else {
      a->data[2 - i] = ' ';
    }
  }

  a->data[3] = ' ';
  a->data[4] = ' ';

  // Hex value.
  a->data[5] = '0';
  a->data[6] = 'x';
  a->data[7] = hex_char((byte_obj->byteval & 0xf0) >> 4);
  a->data[8] = hex_char(byte_obj->byteval & 0xf);

  a->data[9] = ' ';
  a->data[10] = ' ';

  v = byte_obj->byteval;
  for (int i = 0; i < 8; i++) {
    char b = (v & (1 << (7-i))) >> (7-i) ? '1' : '0';
    a->data[11 + i] = b;
  }

  return string_obj(a);
}

obj_t *int32_dump(obj_t *int_obj) {
  // 2147483647 -> "0x7fffffff  01111111 11111111 11111111 11111111"
  bytearray_t *a = bytearray_alloc(47);

  a->data[0] = '0';
  a->data[1] = 'x';

  int v = int_obj->intval;
  for (int i = 0; i < 8; i++) {
    a->data[9 - i] = hex_char(v & 0xf);
    v >>= 4;
  }

  a->data[10] = ' ';
  a->data[11] = ' ';

  v = int_obj->intval;
  int offset = 11;
  for (int i = 0; i < 32; i++) {
    // Separate binary digits into groups of 8.
    if (i % 8 == 0) {
      a->data[offset] = ' ';
      offset++;
    }
    char b = (v & (1 << (31-i))) >> (31-i) ? '1' : '0';
    a->data[offset] = b;
    offset++;
  }

  return string_obj(a);
}

obj_t *float32_dump(obj_t *float_obj) {
  bytearray_t *a = bytearray_alloc(40);

  unsigned int *p = (unsigned int *) &(float_obj->floatval);
  unsigned int v = *p;

  int offset = 0;
  for (int i = 0; i < 32; i++) {
    switch(i) {
      case 1: // Sign bit; Start exponent
      case 9: // End exponent
        a->data[offset++] = ' ';
        a->data[offset++] = ' ';
        break;
      default:
        break;
    }
    char b = (v & (1 << (31-i))) >> (31-i) ? '1' : '0';
    a->data[offset] = b;
    offset++;
  }

  return string_obj(a);
}

obj_t *arr_dump(obj_t *arr_obj) {
  // Each row represents 16 bytes and is represented with 80 chars, e.g.,
  // 00000000  23 69 6e 63 6c 75 64 65  20 3c 73 74 64 69 6f 2e  |#include <stdio.|
  // 00000010  68 3e 0a                                          |h>.|
  // ^         ^                                                 ^                ^
  // 0        10                                                60               77
  //
  // We we need 79 bytes to store the representation for 16 bytes.
  dim_t byte_rows = (arr_obj->bytearray->size / 16) + ((arr_obj->bytearray->size % 16) ? 1 : 0);
  dim_t size = byte_rows * 79;
  bytearray_t *ba = bytearray_alloc(size - 1);

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
      byte b = arr_obj->bytearray->data[i + j];
      ba->data[row * 79 + 61 + j] = ((b >= 0x21) && (b <= 0x7e) ) ? b : '.';
      j++;
    }
    ba->data[row * 79 + 61 + j] = '|';

    // No newline after last line.
    if (row < byte_rows - 1) {
      ba->data[row * 79 + 61 + j + 1] = '\n';
    }
    row++;
  }

  return string_obj(ba);
}

static_method get_str_static_method(static_method_ident_t method_id) {
  switch(method_id) {
    case METHOD_HASH: return str_hash;
    case METHOD_COPY: return str_copy;
    case METHOD_TO_INT: return str_to_int;
    case METHOD_TO_STRING: return str_to_string;
    case METHOD_TO_FLOAT: return str_to_float;
    case METHOD_TO_BYTE: return str_to_byte;
    case METHOD_LENGTH: return str_len;
    case METHOD_CONTAINS: return str_contains;
    case METHOD_ADD: return str_add;
    case METHOD_GET: return str_get;
    case METHOD_EQ: return str_eq;
    case METHOD_NE: return str_ne;
    case METHOD_CAST: return str_as;
    case METHOD_SUBSTR: return str_substring;
    case METHOD_RANDOM_CHOICE: return str_random_choice;
    case METHOD_ITERATOR: return arr_iterator;
    default: return NULL;
  }
}

