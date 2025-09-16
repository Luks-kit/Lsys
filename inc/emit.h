#ifndef EMIT_H
#define EMIT_H

#include "parser.h"
#include "scope.h"
#include <stddef.h>

/* emitter context */
typedef struct {
    int out_fd;
    scope_t* scope;
    void* (*alloc)(size_t);
    void (*free_fn)(void*);
} emitter_t;

void emitter_init(emitter_t* e, int out_fd, scope_t* scope, void* (*alloc)(size_t), void (*free_fn)(void*));
int emitter_emit_program(emitter_t* e, ast_node_t* prog); /* returns 0 on success */
void emitter_close(emitter_t* e);

#endif

