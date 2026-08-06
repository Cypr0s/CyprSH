/**
 * @file        tokenize.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Lexical analyzer for shell syntax
 */

#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "error.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "data-structures/stack.h"
#include "data-structures/char-buffer.h"
#include "data-structures/int8-buffer.h"
#include "utilities/strings.h"

#define LEXER_READ_ERR -2

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
    TOKEN_SEMI_AND,     // ;&
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

typedef enum {
    QUOTE_UNQUOTED = 0,
    QUOTE_SINGLE_QUOTED = 1,
    QUOTE_DOUBLE_QUOTED = 2,
    QUOTE_ESCAPED = 3,
} QuoteTypeEnum;

typedef struct {
    TokenTypeEnum type;
    char* value;  // heap-allocated for WORD/IO_NUM; NULL for all others
    int8_t* char_types;
    StatusEnum error_type;
} Token, *TokenPtr;

typedef struct {
    FILE* input;
    Stack token_stack;       // stack for storing FSM states
    int32_t line;            // current line number 
    int32_t lookahead;       // one-char buffer (-1 = none)
    CharBuffer char_buff;
    Int8Buffer int8_buff;
} Lexer, *LexerPtr;

/** @brief Initialize lexer with input file
 *  @param lex Lexer to initialize
 *  @param input Input file stream
 *  @return Status code
 */
StatusEnum lexerCtor(LexerPtr lex, FILE* input);

/** @brief Destroy lexer and free resources
 *  @param lex Lexer to destroy
 */
void lexerDtor(LexerPtr lex);

/** @brief Get next token from input
 *  @param lex Source lexer
 *  @return Next token
 */
Token getToken(LexerPtr lex);

/** @brief Free token's allocated memory
 *  @param tok Token to free
 */
void tokenFree(TokenPtr tok);

/** @brief Reset lexer with new input file
 *  @param lex Lexer to reset
 *  @param input New input file stream
 *  @return Status code
 */
StatusEnum lexerReset(LexerPtr lex, FILE* input);

/** @brief Create null token
 *  @return Null-initialized token
 */
Token nullToken(void);

#endif