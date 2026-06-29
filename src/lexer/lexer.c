/**
 * @file        lexer.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Lexical analyzer implementation
 */

#include "lexer/lexer.h"

// utils
static int32_t getCharacter(LexerPtr lex);
static Token createToken(TokenTypeEnum type, char* value, int8_t* types, StatusEnum st);
static Token errorToken(StatusEnum st);
static StatusEnum bufferPush(LexerPtr lex, const char c, int8_t type);
static int isWordDelimiter(int32_t c);

// state handlers
static Token handleStart(LexerPtr lex, int32_t c);
static Token handleComment(LexerPtr lex, int32_t c);
static Token handleWord(LexerPtr lex, int32_t c);
static Token handleBackslash(LexerPtr lex, int32_t c);
static Token handleSingleQuote(LexerPtr lex, int32_t c);
static Token handleDoubleQuote(LexerPtr lex, int32_t c);
static Token handleDQBackslash(LexerPtr lex, int32_t c);

//--------------------utils--------------------

static int32_t getCharacter(LexerPtr lex) {
    if(lex == NULL) {
        printError("getCharacter", "passing NULL pointer");
        return LEXER_READ_ERR;
    }
    // lookahead set
    if(lex->lookahead != -1) {
        int32_t c = lex->lookahead;
        lex->lookahead = -1;
        return c;
    }
    // lookaheade not set
    int32_t c = fgetc(lex->input);
    if(c == '\n') lex->line++;
    return c;
}


static Token createToken(TokenTypeEnum type, char* value, int8_t* types, StatusEnum st) {
    Token t;
    t.type = type;
    t.value = value;
    t.char_types = types;
    t.error_type = st;
    return t;
}


static Token errorToken(StatusEnum st) {
    return createToken(TOKEN_ERROR, NULL, NULL, st);
}


Token nullToken(void) {
    return createToken(TOKEN_NULL, NULL, NULL, SUCCESS);
}


static StatusEnum bufferPush(LexerPtr lex,const char c, int8_t type) {
    if(lex == NULL) {
        printError("bufferPush", "passing NULL pointer");
        return ERROR_DEFAULT;
    }

    StatusEnum st = charBufferAppendChar(&(lex->char_buff), c);
    if(st != SUCCESS) {
        return st;
    }
    return int8BufferAppend(&(lex->int8_buff), type);
}


static int isWordDelimiter(int32_t c) {
    if(c == EOF) return 1;
    if(c == ' ' || c == '\t' || c == '\n') return 1; // whitespace
    // different operators
    if(c == ';' || c == '&' || c == '|') return 1;
    if(c == '<' || c == '>') return 1;
    if(c == '(' || c == ')') return 1;
    return 0;
}

//---------------handlers--------------------

static Token handleStart(LexerPtr lex, int32_t c) {
    if(lex == NULL) {
        printError("handleStart", "passing NULL pointer");
        return errorToken(ERROR_DEFAULT);
    }

    int32_t c2, c3;
    StatusEnum st = SUCCESS;
    switch (c) {
        /* special characters */
        case EOF:
            return createToken(TOKEN_EOF, NULL, NULL, st);
        case '#':
            st = stackPush(&lex->token_stack, (int8_t)STATE_COMMENT);
            if(st != SUCCESS) {
                return errorToken(st);
            }
            return nullToken();
        case '\n':
            return createToken(TOKEN_NEWLINE, NULL, NULL, st);

        /* whitespaces */
        case ' ': case '\t':
            return nullToken();

        /* operators */
        case ';':
            // semicolon and double semicolon
            c2 = getCharacter(lex);
            if(c2 == LEXER_READ_ERR) {
                return errorToken(ERROR_DEFAULT);
            }
            if(c2 == ';') return createToken(TOKEN_DOUBLE_SEMI, NULL, NULL, st);
            if(c2 == '&') return createToken(TOKEN_SEMI_AND, NULL, NULL, st);
            lex->lookahead = c2;
            return createToken(TOKEN_SEMI, NULL, NULL, st);
        case '|':
            // pipe and OR_IF
            c2 = getCharacter(lex);
            if(c2 == LEXER_READ_ERR) {
                return errorToken(ERROR_DEFAULT);
            }
            if(c2 == '|') return createToken(TOKEN_OR_IF, NULL, NULL, st);
            lex->lookahead = c2;
            return createToken(TOKEN_PIPE, NULL, NULL, st);
        case '&':
        // ampersand and AND_IF
            c2 = getCharacter(lex);
            if(c2 == LEXER_READ_ERR) {
                return errorToken(ERROR_DEFAULT);
            }
            if(c2 == '&') return createToken(TOKEN_AND_IF, NULL, NULL, st);
            lex->lookahead = c2;
            return createToken(TOKEN_BG, NULL, NULL, st);
        // simple braces, !
        case '(':
            return createToken(TOKEN_LPAREN, NULL, NULL, st);
        case ')':
            return createToken(TOKEN_RPAREN, NULL, NULL, st);
        case '{':
            return createToken(TOKEN_LBRACE, NULL, NULL, st);
        case '}':
            return createToken(TOKEN_RBRACE, NULL, NULL, st);
        case '!':
            return createToken(TOKEN_BANG, NULL, NULL, st);
        case '>':
        // >, >>, >&, >| (clobber)
            c2 = getCharacter(lex);
            if(c2 == LEXER_READ_ERR) {
                return errorToken(ERROR_DEFAULT);
            }
            if(c2 == '>') return createToken(TOKEN_DGREAT, NULL, NULL, st);
            if(c2 == '&') return createToken(TOKEN_GREATAND, NULL, NULL, st);
            if(c2 == '|') return createToken(TOKEN_CLOBBER, NULL, NULL, st);
            lex->lookahead = c2;
            return createToken(TOKEN_GREAT, NULL, NULL, st);
        case '<':
        // <, <<, <<<, <<-, <a&, <>
            c2 = getCharacter(lex);
            if(c2 == LEXER_READ_ERR) {
                return errorToken(ERROR_DEFAULT);
            }
            // handle <<, <<<, <<-
            if(c2 == '<') {
                c3 = getCharacter(lex);
                if(c3 == LEXER_READ_ERR) {
                    return errorToken(ERROR_DEFAULT);
                }
                if(c3 == '<') return createToken(TOKEN_TLESS, NULL, NULL, st);
                if(c3 == '-') return createToken(TOKEN_DLESSDASH, NULL, NULL, st);
                lex->lookahead = c3;
                return createToken(TOKEN_DLESS, NULL, NULL, st);
            }
            // handle <&, <>
            if(c2 == '&') return createToken(TOKEN_LESSAND, NULL, NULL, st);
            if(c2 == '>') return createToken(TOKEN_LESSGREAT, NULL, NULL, st);
            lex->lookahead = c2;
            return createToken(TOKEN_LESS, NULL, NULL, st);
            // different word starts
        case '"': case '\'': case '\\':
            // word token starts
            st = stackPush(&lex->token_stack, (int8_t)STATE_WORD);
            if(st != SUCCESS) {
                return errorToken(st);
            }

            if(c == '\'') {
                st = stackPush(&lex->token_stack, (int8_t)STATE_SINGLE_QUOTE);
            } else if(c == '"') {
                st = stackPush(&lex->token_stack, (int8_t)STATE_DOUBLE_QUOTE);
            } else if(c == '\\') {
                st = stackPush(&lex->token_stack, (int8_t)STATE_BACKSLASH);
            }
            if(st != SUCCESS) {
                return errorToken(st);
            }
            return nullToken();
        default:
            // regular word tokens, all word delimiters are handled above
            st = stackPush(&lex->token_stack, (int8_t)STATE_WORD);
            if(st != SUCCESS) {
                return errorToken(st);
            }

            st = bufferPush(lex, (char)c, QUOTE_UNQUOTED);
            if(st != SUCCESS) {
                return errorToken(st);
            }
            return nullToken();
    }
}


static Token handleComment(LexerPtr lex, int32_t c) {
    if(lex == NULL) {
        printError("handleComment", "passing NULL pointer");
        return errorToken(ERROR_DEFAULT);
    }

    // if its newlind ore end exit otherwise ignore
    if(c == '\n' || c == EOF) {
        StatusEnum st = stackPop(&lex->token_stack);
        if(st != SUCCESS) {
            return errorToken(st);
        }

        return c == EOF ? createToken(TOKEN_EOF, NULL, NULL, SUCCESS) : createToken(TOKEN_NEWLINE, NULL, NULL, SUCCESS);
    }
    return nullToken();
}


static Token handleWord(LexerPtr lex, int32_t c) {
    if(lex == NULL) {
        printError("handleWord", "passing NULL pointer");
        return errorToken(ERROR_DEFAULT);
    }

    StatusEnum st = SUCCESS;

    if(isWordDelimiter(c)) {
        lex->lookahead = c;
        st = stackPop(&lex->token_stack);  /* pop WORD */
        if(st != SUCCESS) {
            return errorToken(st);
        }

        size_t word_length = lex->char_buff.size;
        
        char* word = charBufferTransfer(&(lex->char_buff));

        int8_t* quotes = int8BufferTransfer(&(lex->int8_buff));
        
        if(c == '<' || c == '>') {
            for(size_t i = 0; i < word_length; i++) {
                if(word[i] < '0' || word[i] > '9') {
                    return createToken(TOKEN_WORD, word, quotes, st); // word
                }
            }
            free(quotes); // io number doesnt need quotes
            return createToken(TOKEN_IO_NUM, word, NULL, st); // io
        }
        return createToken(TOKEN_WORD, word, quotes, st); // word
    }

    if(c == '\'') {
        st = stackPush(&lex->token_stack, (int8_t)STATE_SINGLE_QUOTE);
    } else if(c == '"') {
        st = stackPush(&lex->token_stack, (int8_t)STATE_DOUBLE_QUOTE);
    } else if(c == '\\') {
        st = stackPush(&lex->token_stack, (int8_t)STATE_BACKSLASH);
    } else {
        st = bufferPush(lex, (char)c, QUOTE_UNQUOTED);

    }
    if(st != SUCCESS) {
        return errorToken(st);
    }
    return nullToken();
}


static Token handleBackslash(LexerPtr lex, int32_t c) {
    if(lex == NULL) {
        printError("handleBackslash", "passing NULL pointer");
        return errorToken(ERROR_DEFAULT);
    }

    StatusEnum st = SUCCESS;
    if(c == EOF) {
        printError("handleBackslash", "Unexpected EOF after backslash at line %d", lex->line);
        return errorToken(ERROR_LEXICAL_ERROR);
    }

    // explicit '\n', line continuation
    if(c == '\n') {
        lex->line++;
        st = stackPop(&lex->token_stack);
        if(st != SUCCESS) {
            return errorToken(st);
        }
        return nullToken();
    }

    // push token
    st = bufferPush(lex, (char)c, QUOTE_ESCAPED);
    if(st != SUCCESS) {
        return errorToken(st);
    }

    // pop state
    st = stackPop(&lex->token_stack);
    if(st != SUCCESS) {
        return errorToken(st);
    }
    return nullToken();
}


static Token handleSingleQuote(LexerPtr lex, int32_t c) {
    if(lex == NULL) {
        printError("handleSingleQuote", "passing NULL pointer");
        return errorToken(ERROR_DEFAULT);
    }

    if(c == EOF) {
        printError("handleSingleQuote", "Unterminated single quote at line %d", lex->line);
        return errorToken(ERROR_LEXICAL_ERROR);
    }

    StatusEnum st = SUCCESS;

    if (c != '\'') {
        st = bufferPush(lex, (char)c, QUOTE_SINGLE_QUOTED);
    } else {
        st = stackPop(&lex->token_stack); // exit single quote
    }

    if(st != SUCCESS) {
        return errorToken(st);
    }
    return nullToken();
}


static Token handleDoubleQuote(LexerPtr lex, int32_t c) {
    if(lex == NULL) {
        printError("handleDoubleQuote", "passing NULL pointer");
        return errorToken(ERROR_DEFAULT);
    }

    if(c == EOF) {
        printError("handleDoubleQuote", "unterminated double quote at line %d", lex->line);
        return errorToken(ERROR_LEXICAL_ERROR);
    }

    StatusEnum st = SUCCESS;

    if(c == '"') {
        st = stackPop(&lex->token_stack);
    } else if(c == '\\') {
        st = stackPush(&lex->token_stack, (int8_t)STATE_DQ_BACKSLASH);
    } else {
        st = bufferPush(lex, (char)c, QUOTE_DOUBLE_QUOTED);
    }

    if(st != SUCCESS) {
        return errorToken(st);
    }
    return nullToken();
}


static Token handleDQBackslash(LexerPtr lex, int32_t c) {
    if(lex == NULL) {
        printError("handleDQBackslash", "passing NULL pointer");
        return errorToken(ERROR_DEFAULT);
    }

    if(c == EOF) {
        printError("handleDQBackslash", "Unterminated double quote at line %d\n", lex->line);
        return errorToken(ERROR_LEXICAL_ERROR);
    }

    StatusEnum st = SUCCESS;
    if(c == '\n') {
        lex->line++;
        st = stackPop(&lex->token_stack);
        if(st != SUCCESS) {
            return errorToken(st);
        }
        return nullToken();
    }
    // escaped characters based on POSIX
    if(c != '"' && c != '\\' && c != '$' && c != '`') {
        st = bufferPush(lex, '\\', QUOTE_DOUBLE_QUOTED);
        if(st != SUCCESS) {
            return errorToken(st);
        }
        st = bufferPush(lex, (char)c, QUOTE_DOUBLE_QUOTED);
        if(st != SUCCESS) {
            return errorToken(st);
        }
        
    }
    st = bufferPush(lex, (char)c, QUOTE_ESCAPED);
    if(st != SUCCESS) {
        return errorToken(st);
    }
    st = stackPop(&lex->token_stack);
    if(st != SUCCESS) {
        return errorToken(st);
    }
    return nullToken();
}


StatusEnum lexerCtor(LexerPtr lex, FILE* input) {
    if(lex == NULL) {
        printError("lexerCtor", "passing NULL pointer");
        return ERROR_DEFAULT;
    }
    lex->input = input;
    lex->line = 1;
    lex->lookahead = -1;
    StatusEnum st = SUCCESS;
    st = stackCtor(&lex->token_stack);
    ERR_CHECK(st);
    st = stackPush(&lex->token_stack, (int8_t)STATE_START);
    ERR_CHECK(st);
    st = charBufferCtor(&(lex->char_buff), DEFAULT_BUFFER_SIZE);
    ERR_CHECK(st);
    st = int8BufferCtor(&(lex->int8_buff), DEFAULT_BUFFER_SIZE);
    if(st != SUCCESS) {
        charBufferDtor(&(lex->char_buff));
        return st;
    }

    return st;
}


void lexerDtor(LexerPtr lex) {
    if(lex == NULL) {
        return;
    }
    if(lex->input != NULL && lex->input != stdin) {
        fclose(lex->input);
    }

    charBufferDtor(&(lex->char_buff));
    int8BufferDtor(&(lex->int8_buff));
}


Token getToken(LexerPtr lex) {
    if(lex == NULL) {
        printError("getToken", "passing NULL pointer");
        return errorToken(ERROR_DEFAULT);
    }

    charBufferReset(&(lex->char_buff));
    int8BufferReset(&(lex->int8_buff));

    int32_t c;
    int8_t state_val;
    Token t = nullToken();

    while(t.type == TOKEN_NULL) {
        c = getCharacter(lex);
        if(c == LEXER_READ_ERR) {
            return errorToken(ERROR_DEFAULT);
        }
        
        // get top state
        StatusEnum st = stackTop(&lex->token_stack, &state_val);
        if(st != SUCCESS) {
            return errorToken(st);
        }
        FSMState state = (FSMState)state_val;

        switch (state) {
            case STATE_START:
                t = handleStart(lex, c);
                break;
            case STATE_COMMENT:
                t = handleComment(lex, c);
                break;
            case STATE_WORD:
                t = handleWord(lex, c);
                break;
            case STATE_BACKSLASH:
                t = handleBackslash(lex, c);
                break;
            case STATE_SINGLE_QUOTE:
                t = handleSingleQuote(lex, c);
                break;
            case STATE_DOUBLE_QUOTE:    
                t = handleDoubleQuote(lex, c);
                break;
            case STATE_DQ_BACKSLASH:
                t = handleDQBackslash(lex, c);
                break;
        }
    }
    return t;
}


void tokenFree(TokenPtr tok) {
    if(tok == NULL) { 
        return;
    }
    
    free(tok->value);
    tok->value = NULL;

    free(tok->char_types);
    tok->char_types = NULL;
}

StatusEnum lexerReset(LexerPtr lex, FILE* input) {
    if(lex == NULL) {
        printError("lexerReset", "passing NULL pointer");
        return ERROR_DEFAULT;
    }

    if(lex->input != NULL && lex->input != stdin) {
        fclose(lex->input);  // close old file before replacing
    }

    // reset buffer and line number
    charBufferReset(&(lex->char_buff));
    int8BufferReset(&(lex->int8_buff));
    lex->line = 1;
    lex->lookahead = -1;
    StatusEnum st = SUCCESS;

    int8_t flag = stackIsEmpty(&lex->token_stack);
    if(flag == -1) {
        return ERROR_DEFAULT;
    }
    // reset state stack to initial state (pop until empty)
    while(!flag) {
        st = stackPop(&lex->token_stack);
        ERR_CHECK(st);
        flag = stackIsEmpty(&lex->token_stack);
        if(flag == -1) {
            return ERROR_DEFAULT;
        }
    }
    st = stackPush(&lex->token_stack, (int8_t)STATE_START);
    ERR_CHECK(st);
    
    // reset input
    lex->input = input;
    return st;
}