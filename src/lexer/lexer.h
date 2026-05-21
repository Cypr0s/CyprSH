#ifndef LEXER_H
#define LEXER_H

#include "../utils/error.h"
#include <stdio.h>
#include <stdint.h>

typedef enum {
    TOKEN_WORD,         // any unquoted/quoted word
    TOKEN_NEWLINE,      // \n
    TOKEN_IO_NUM,       // digits immediately before < or >
    TOKEN_EOF,

    // control operators
    TOKEN_SEMI,         // ;
    TOKEN_PIPE,         // |
    TOKEN_AND_IF,       // &&
    TOKEN_OR_IF,        // ||
    TOKEN_DOUBLE_SEMI,  // ;;
    TOKEN_CLOBBER,      // >|
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_BANG,         // !
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_BG,           // &

    // redirectors
    TOKEN_LESS,         // <
    TOKEN_GREAT,        // >
    TOKEN_DLESS,        // <<
    TOKEN_DGREAT,       // >>
    TOKEN_LESSAND,      // <&
    TOKEN_GREATAND,     // >&
    TOKEN_LESSGREAT,    // <>
    TOKEN_DLESSDASH,    // <<-
    TOKEN_TLESS,        // <<<

    TOKEN_ERROR,
} TokenTypeEnum;

typedef struct {
    TokenTypeEnum type;
    char*         value;  // heap-allocated for WORD/IO_NUM; NULL for all others
} Token, *TokenPtr;

typedef struct {
    FILE* input;
    int   lookahead;  // one-char buffer (-1 = empty)
    int   line;       // current line number (1-based, for error reporting)
} Lexer;

void  lexerInit(Lexer* lex, FILE* input);
Token getToken(Lexer* lex);
void  tokenFree(Token* tok);

#endif