#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "lmem.h"

/* AST node kinds */
typedef enum {
    NODE_PROGRAM,   // linked list of statements
    NODE_ASSIGN,    // var = expr
    NODE_INT,       // integer literal
    NODE_VAR,       // identifier
    NODE_BINOP      // left op right
} node_type_t;

typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_NONE
} binop_type_t;

typedef struct ast_node {
    node_type_t type;
    struct ast_node* next;   // for program: linked list of statements

    /* for assign */
    char* name;              // for NODE_ASSIGN or NODE_VAR: pointer into source (not nul-terminated)
    size_t name_len;

    /* for int */
    int int_value;

    /* for binop */
    binop_type_t op;
    struct ast_node* left;
    struct ast_node* right;
} ast_node_t;

typedef struct {
    lexer_t* lex;
    token_t cur;
    void* (*alloc)(size_t);  // allocator (from lmem)
} parser_t;

void parser_init(parser_t* p, lexer_t* lex, void* (*alloc_fn)(size_t));
ast_node_t* parser_parse_program(parser_t* p);    // returns linked list (NODE_PROGRAM -> statements)
void parser_free_ast(parser_t* p, ast_node_t* root); // frees using p->alloc? (noop for now)

#endif

