#pragma once

#include <stddef.h>

/* Set the first len bytes in b to val. */
void mem_set(void *b, const unsigned char val, size_t len);

/* Copy n bytes from src to dst. Return pointer to dst. */
void *mem_cp(void *dst, const void *src, size_t size);
