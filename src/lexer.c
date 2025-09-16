#include "lexer.h"

static int is_digit(char c){return c>='0' && c<='9';}
static int is_alpha(char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_';}
static int is_alnum(char c){return is_alpha(c)||is_digit(c);}

void lexer_init(lexer_t* lex, char* src, size_t length){ lex->src=src; lex->pos=0; lex->length=length; }

token_t lexer_next(lexer_t* lex){
    while(lex->pos<lex->length){
        char c=lex->src[lex->pos];
        if(c==' '||c=='\n'||c=='\t'||c=='\r'){ lex->pos++; continue; }
        break;
    }
    if(lex->pos>=lex->length) return (token_t){TOKEN_EOF,0,0,0};
    char c=lex->src[lex->pos];

    if(is_digit(c)){
        size_t start=lex->pos; int val=0;
        while(lex->pos<lex->length && is_digit(lex->src[lex->pos])){
            val=val*10 + (lex->src[lex->pos]-'0'); lex->pos++;
        }
        return (token_t){TOKEN_INT,&lex->src[start],lex->pos-start,val};
    }
    if(is_alpha(c)){
        size_t start=lex->pos; while(lex->pos<lex->length && is_alnum(lex->src[lex->pos])) lex->pos++;
        return (token_t){TOKEN_IDENTIFIER,&lex->src[start],lex->pos-start,0};
    }

    lex->pos++;
    switch(c){
        case '=': return (token_t){TOKEN_ASSIGN,&lex->src[lex->pos-1],1,0};
        case '+': if(lex->pos<lex->length && lex->src[lex->pos]=='='){ lex->pos++; return (token_t){TOKEN_PLUS_EQ,&lex->src[lex->pos-2],2,0}; } else return (token_t){TOKEN_PLUS,&lex->src[lex->pos-1],1,0};
        case '-': if(lex->pos<lex->length && lex->src[lex->pos]=='='){ lex->pos++; return (token_t){TOKEN_MINUS_EQ,&lex->src[lex->pos-2],2,0}; } else return (token_t){TOKEN_MINUS,&lex->src[lex->pos-1],1,0};
        case '*': if(lex->pos<lex->length && lex->src[lex->pos]=='='){ lex->pos++; return (token_t){TOKEN_STAR_EQ,&lex->src[lex->pos-2],2,0}; } else return (token_t){TOKEN_STAR,&lex->src[lex->pos-1],1,0};
        case '/': if(lex->pos<lex->length && lex->src[lex->pos]=='='){ lex->pos++; return (token_t){TOKEN_SLASH_EQ,&lex->src[lex->pos-2],2,0}; } else return (token_t){TOKEN_SLASH,&lex->src[lex->pos-1],1,0};
        case ';': return (token_t){TOKEN_SEMICOLON,&lex->src[lex->pos-1],1,0};
        case '(': return (token_t){TOKEN_LPAREN,&lex->src[lex->pos-1],1,0};
        case ')': return (token_t){TOKEN_RPAREN,&lex->src[lex->pos-1],1,0};
        case '{': return (token_t){TOKEN_LBRACE,&lex->src[lex->pos-1],1,0};
        case '}': return (token_t){TOKEN_RBRACE,&lex->src[lex->pos-1],1,0};
        default: return (token_t){TOKEN_UNKNOWN,&lex->src[lex->pos-1],1,0};
    }
}

