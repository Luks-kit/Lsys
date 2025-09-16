#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum {
    TOKEN_EOF, TOKEN_INT, TOKEN_IDENTIFIER,
    TOKEN_ASSIGN, TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH,
    TOKEN_PLUS_EQ, TOKEN_MINUS_EQ, TOKEN_STAR_EQ, TOKEN_SLASH_EQ,
    TOKEN_SEMICOLON, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE,
    TOKEN_UNKNOWN
} token_type_t;

typedef struct {
    token_type_t type;
    char* start;
    size_t length;
    int value;
} token_t;

typedef struct {
    char* src;
    size_t pos;
    size_t length;
} lexer_t;

void lexer_init(lexer_t* lex, char* src, size_t length);
token_t lexer_next(lexer_t* lex);

#endif

