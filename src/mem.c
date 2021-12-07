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

  hdr->type = type;
  hdr->flags = flags;

  // TODO mark and sweep
  hdr->children = 0;
}

