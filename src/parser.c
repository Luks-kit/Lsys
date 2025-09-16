#include "parser.h"
#include "lstr.h"
#include <stddef.h>

/* helper to allocate nodes through provided allocator */
static ast_node_t* new_node(parser_t* p) {
    ast_node_t* n = (ast_node_t*) p->alloc(sizeof(ast_node_t));
    if (!n) return NULL;
    /* zero init */
    size_t s = sizeof(ast_node_t);
    unsigned char* q = (unsigned char*) n;
    for (size_t i = 0; i < s; ++i) q[i] = 0;
    return n;
}

void parser_init(parser_t* p, lexer_t* lex, void* (*alloc_fn)(size_t)) {
    p->lex = lex;
    p->alloc = alloc_fn;
    p->cur = lexer_next(lex);
}

/* advance token */
static void advance(parser_t* p) { p->cur = lexer_next(p->lex); }

static int token_is_binop(token_type_t t) {
    return t == TOKEN_PLUS || t == TOKEN_MINUS || t == TOKEN_STAR || t == TOKEN_SLASH;
}

/* convert token to binop type */
static binop_type_t token_to_binop(token_type_t t) {
    switch (t) {
        case TOKEN_PLUS: return OP_ADD;
        case TOKEN_MINUS: return OP_SUB;
        case TOKEN_STAR: return OP_MUL;
        case TOKEN_SLASH: return OP_DIV;
        default: return OP_NONE;
    }
}

/* parse factor: INT | IDENT */
static ast_node_t* parse_factor(parser_t* p) {
    token_t t = p->cur;
    if (t.type == TOKEN_INT) {
        ast_node_t* n = new_node(p);
        n->type = NODE_INT;
        n->int_value = t.value;
        advance(p);
        return n;
    }
    if (t.type == TOKEN_IDENTIFIER) {
        ast_node_t* n = new_node(p);
        n->type = NODE_VAR;
        n->name = t.start;
        n->name_len = t.length;
        advance(p);
        return n;
    }
    return NULL;
}

/* simple expression parser with precedence: * / higher than + - */
static ast_node_t* parse_term(parser_t* p) {
    ast_node_t* node = parse_factor(p);
    while (p->cur.type == TOKEN_STAR || p->cur.type == TOKEN_SLASH) {
        binop_type_t op = token_to_binop(p->cur.type);
        advance(p);
        ast_node_t* rhs = parse_factor(p);
        ast_node_t* parent = new_node(p);
        parent->type = NODE_BINOP;
        parent->op = op;
        parent->left = node;
        parent->right = rhs;
        node = parent;
    }
    return node;
}

static ast_node_t* parse_expression(parser_t* p) {
    ast_node_t* node = parse_term(p);
    while (p->cur.type == TOKEN_PLUS || p->cur.type == TOKEN_MINUS) {
        binop_type_t op = token_to_binop(p->cur.type);
        advance(p);
        ast_node_t* rhs = parse_term(p);
        ast_node_t* parent = new_node(p);
        parent->type = NODE_BINOP;
        parent->op = op;
        parent->left = node;
        parent->right = rhs;
        node = parent;
    }
    return node;
}

/* parse assignment: IDENT '=' expression ';' */
static ast_node_t* parse_assignment(parser_t* p) {
    if (p->cur.type != TOKEN_IDENTIFIER) return NULL;
    token_t id = p->cur;
    advance(p);
    if (p->cur.type != TOKEN_ASSIGN) return NULL;
    advance(p);
    ast_node_t* expr = parse_expression(p);
    if (!expr) return NULL;
    /* optional semicolon */
    if (p->cur.type == TOKEN_SEMICOLON) advance(p);

    ast_node_t* n = new_node(p);
    n->type = NODE_ASSIGN;
    n->name = id.start;
    n->name_len = id.length;
    n->left = expr; /* store expression in left */
    return n;
}

/* parse single statement (for now only assignments) */
static ast_node_t* parse_statement(parser_t* p) {
    if (p->cur.type == TOKEN_IDENTIFIER) {
        return parse_assignment(p);
    }
    return NULL;
}

ast_node_t* parser_parse_program(parser_t* p) {
    ast_node_t* head = NULL;
    ast_node_t* tail = NULL;
    while (p->cur.type != TOKEN_EOF) {
        ast_node_t* stmt = parse_statement(p);
        if (!stmt) {
            /* skip unknown token to avoid infinite loop */
            advance(p);
            continue;
        }
        if (!head) head = tail = stmt;
        else { tail->next = stmt; tail = stmt; }
    }
    return head;
}

void parser_free_ast(parser_t* p, ast_node_t* root) {
    /* minimal: lfree not used here; user can extend if needed */
    (void)p; (void)root;
}

