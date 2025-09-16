#ifndef LMEM_H
#define LMEM_H

#include <stddef.h>
#include <stdint.h>

void* lmalloc(size_t sz);
void lfree(void* ptr);
void* lrealloc(void* ptr, size_t new_sz);
void lmemset(void* ptr, uint8_t value, size_t sz);

#endif

