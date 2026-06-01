#ifndef LEXER_H
#define LEXER_H

#include "error.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "data_structures/stack.h"
#include "utils/strings.h"

#define MAX_TOKEN_LENGTH 1024

typedef enum {
    STATE_START, // starting state
    STATE_WORD,  // inside word
    STATE_BACKSLASH,  // backlash in word
    STATE_SINGLE_QUOTE,  // inside single quotes
    STATE_DOUBLE_QUOTE,  // inside double quotes
    STATE_DQ_BACKSLASH,  // backslash in double quotes
    STATE_COMMENT,  // inside comment
} FSMState;

typedef enum {
    TOKEN_WORD,         // any unquoted/quoted word
    TOKEN_NEWLINE,      // \n
    TOKEN_IO_NUM,       // digits immediately before < or >
    TOKEN_IO_LOCATION, // not supported, reserved for future use based on bash's IO_LOCATION token

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

    // utils
    TOKEN_ERROR,
    TOKEN_NULL,          // for uninitialized tokens
} TokenTypeEnum;

typedef struct {
    TokenTypeEnum type;
    char* value;  // heap-allocated for WORD/IO_NUM; NULL for all others
} Token, *TokenPtr;

typedef struct {
    FILE* input;
    Stack token_stack;       // stack for storing FSM states
    int32_t line;            // current line number 
    int32_t lookahead;       // one-char buffer (-1 = none)
    char buffer[MAX_TOKEN_LENGTH];
    int16_t buffer_pos;
} Lexer, *LexerPtr;

StatusEnum lexerCtor(LexerPtr lex, FILE* input);

void lexerDtor(LexerPtr lex);

Token getToken(LexerPtr lex);

void tokenFree(TokenPtr tok);

void lexerReset(LexerPtr lex, FILE* input);

Token nullToken(void);

#endif