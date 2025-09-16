#include "scope.h"
#include "lstr.h"
#include <stddef.h>

/* helper to duplicate identifier into nul-terminated string via allocator */
static char* dup_ident(scope_t* s, const char* src, size_t len) {
    char* d = (char*) s->alloc(len + 1);
    for (size_t i=0;i<len;i++) d[i] = src[i];
    d[len] = '\0';
    return d;
}

/* helper to create label string like v_123 */
static char* make_label(scope_t* s) {
    /* decimal into buffer (small) */
    char buf[32];
    size_t i = 0;
    size_t v = s->next_id++;
    if (v == 0) { buf[i++] = '0'; }
    else {
        char temp[32];
        size_t t = 0;
        while (v) { temp[t++] = '0' + (v % 10); v /= 10; }
        for (size_t j = 0; j < t; ++j) buf[i + j] = temp[t - 1 - j];
        i += t;
    }
    buf[i] = '\0';

    /* label prefix "v_" + digits */
    size_t prelen = 2;
    char* out = (char*) s->alloc(prelen + lstrlen(buf) + 1);
    out[0] = 'v'; out[1] = '_';
    size_t j = 0;
    while (buf[j]) { out[prelen + j] = buf[j]; ++j; }
    out[prelen + j] = '\0';
    return out;
}

void scope_init(scope_t* s, void* (*alloc)(size_t), void (*free_fn)(void*)) {
    s->alloc = alloc;
    s->free_fn = free_fn;
    s->entries = (sym_entry_t*) alloc(sizeof(sym_entry_t) * 8);
    s->count = 0;
    s->cap = 8;
    s->next_id = 0;
    /* zero entries */
    for (size_t i=0;i<s->cap;i++){ s->entries[i].name = NULL; s->entries[i].label = NULL; s->entries[i].name_len = 0; }
}

char* scope_get_label(scope_t* s, const char* name, size_t name_len) {
    /* search */
    for (size_t i=0;i<s->count;i++) {
        if (s->entries[i].name_len == name_len) {
            if (lstrncmp(s->entries[i].name, name, name_len) == 0) {
                return s->entries[i].label;
            }
        }
    }
    /* create */
    if (s->count >= s->cap) {
        size_t newcap = s->cap * 2;
        sym_entry_t* newarr = (sym_entry_t*) s->alloc(sizeof(sym_entry_t) * newcap);
        /* copy */
        for (size_t i=0;i<s->count;i++) newarr[i] = s->entries[i];
        /* initialize rest */
        for (size_t i=s->count;i<newcap;i++) { newarr[i].name = NULL; newarr[i].label = NULL; newarr[i].name_len = 0; }
        s->entries = newarr;
        s->cap = newcap;
    }
    char* ncpy = dup_ident(s, name, name_len);
    char* lbl = make_label(s);
    s->entries[s->count].name = ncpy;
    s->entries[s->count].name_len = name_len;
    s->entries[s->count].label = lbl;
    s->count++;
    return lbl;
}

sym_entry_t* scope_entry_at(scope_t* s, size_t idx) {
    if (idx >= s->count) return NULL;
    return &s->entries[idx];
}

