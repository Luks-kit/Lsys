#define _GNU_SOURCE
#include <unistd.h>
#include "lmem.h"
#include <sys/mman.h>

typedef struct lblock {
    size_t size;
    struct lblock* next;
    uint8_t data[];
} lblock_t;

static lblock_t* heap_head = NULL;

void* lmalloc(size_t sz) {
    size_t total = sizeof(lblock_t) + sz;
    lblock_t* block = mmap(0, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (block == (void*)-1) return NULL;
    block->size = sz;
    block->next = heap_head;
    heap_head = block;
    return block->data;
}

void lfree(void* ptr) {
    if (!ptr) return;
    lblock_t* block = (lblock_t*)((uint8_t*)ptr - offsetof(lblock_t, data));
    if (block->next == heap_head) heap_head = block->next;
    munmap(block, sizeof(lblock_t) + block->size);
}

void* lrealloc(void* ptr, size_t new_sz) {
    if (!ptr) return lmalloc(new_sz);
    void* new_mem = lmalloc(new_sz);
    lblock_t* block = (lblock_t*)((uint8_t*)ptr - offsetof(lblock_t, data));
    size_t copy_sz = block->size < new_sz ? block->size : new_sz;
    for (size_t i=0;i<copy_sz;i++) ((uint8_t*)new_mem)[i]=((uint8_t*)ptr)[i];
    lfree(ptr);
    return new_mem;
}

void lmemset(void* ptr, uint8_t value, size_t sz) {
    for (size_t i=0;i<sz;i++) ((uint8_t*)ptr)[i]=value;
}

