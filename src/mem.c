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

void mark_traceable(void *obj, uint16_t type) {
  ((traceable_obj_t*) obj)->type = type;
  ((traceable_obj_t*) obj)->flags = F_NONE;
}

