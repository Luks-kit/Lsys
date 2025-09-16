#ifndef SCOPE_H
#define SCOPE_H

#include <stddef.h>

typedef struct scope scope_t;

/* an entry describing a variable: stores a label name in data section */
typedef struct {
    char* name;      /* nul-terminated copy owned by scope */
    size_t name_len;
    char* label;     /* nul-terminated label string like "v_0" */
} sym_entry_t;

/* opaque scope type */
struct scope {
    sym_entry_t* entries; /* dynamically allocated array (simple vector) */
    size_t count;
    size_t cap;
    void* (*alloc)(size_t);
    void (*free_fn)(void*);
    size_t next_id;       /* used to generate labels v_0, v_1, ... */
};

/* initialize scope context */
void scope_init(scope_t* s, void* (*alloc)(size_t), void (*free_fn)(void*));

/* lookup or create variable label; returns pointer to label string owned by scope */
char* scope_get_label(scope_t* s, const char* name, size_t name_len);

/* iterate entries (start index) */
sym_entry_t* scope_entry_at(scope_t* s, size_t idx);

#endif

