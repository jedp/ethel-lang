#include <assert.h>
#include "../inc/type.h"
#include "../inc/mem.h"
#include "../inc/heap.h"

void* mem_alloc(size_t size) {
  return ealloc(size);
}

void* mem_realloc(void *b, size_t size) {
  return erealloc(b, size);
}

void mem_free(void *b) {
  return efree(b);
}

void mem_init(unsigned char initval) {
  heap_init(initval);
}

void mark_traceable(void *obj, type_t type, flags_t flags) {
  gc_header_t *hdr = (gc_header_t*) obj;
  assert(type > TYPE_ERR_DO_NOT_USE);

  hdr->type = type;
  hdr->flags = flags;

  switch(TYPEOF(obj)) {
    // Primitive and simple types with no children.
    case TYPE_UNKNOWN:
    case TYPE_NOTHING:
    case TYPE_UNDEF:
    case TYPE_NIL:
    case TYPE_ERROR:
    case TYPE_INT:
    case TYPE_FLOAT:
    case TYPE_BYTE:
    case TYPE_BOOLEAN:
    case TYPE_BREAK:
    case TYPE_CONTINUE:
      hdr->children = 0;
      break;

    // Fancy types have one pointer to the child structure.
    case TYPE_LIST:
    case TYPE_DICT:
    case TYPE_BYTEARRAY:
    case TYPE_STRING:
    case TYPE_FUNCTION:
    case TYPE_RANGE:
    case TYPE_ITERATOR:
      hdr->children = 1;
      break;

    // Various child data structures.
    case TYPE_BYTEARRAY_DATA: hdr->children = 0; break;
    case TYPE_RANGE_DATA: hdr->children = 0; break;
    case TYPE_VARIABLE_ARGS: hdr->children = 2; break;
    case TYPE_LIST_DATA: hdr->children = 1; break;
    case TYPE_LIST_ELEM_DATA: hdr->children = 2; break;
    case TYPE_DICT_DATA: hdr->children = 1; break;
    case TYPE_DICT_KV_DATA: hdr->children = 3; break;
    case TYPE_FUNCTION_PTR_DATA: hdr->children = 2; break;
    case TYPE_ITERATOR_DATA: hdr->children = 2; break;

    // Environment.
    case ENV_SYM: hdr->children = 4; break;

    default:
      printf("Unhandled GC type. Object will be untraceable: %s\n", type_names[TYPEOF(obj)]);
      break;
  }
}

