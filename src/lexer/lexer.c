#include "lexer/lexer.h"

// utils
static int32_t getCharacter(LexerPtr lex);
static Token createToken(TokenTypeEnum type, char* value, int8_t* types);
static Token errorToken(void);
static uint8_t pushBuffer(LexerPtr lex, const char c, int8_t type);
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


static Token createToken(TokenTypeEnum type, char* value, int8_t* types) {
    Token t;
    t.type = type;
    t.value = value;
    t.char_types = types;
    return t;
}


static Token errorToken(void) {
    return createToken(TOKEN_ERROR, NULL, NULL);
}


Token nullToken(void) {
    return createToken(TOKEN_NULL, NULL, NULL);
}


static uint8_t pushBuffer(LexerPtr lex,const char c, int8_t type) {
    if(lex->buffer_pos >= MAX_TOKEN_LENGTH - 1) {
        fprintf(stderr, "lexer buffer overflow at line %d\n", lex->line);
        return 0;
    }
    lex->char_buffer[lex->buffer_pos] = c;
    lex->type_buffer[lex->buffer_pos++] = type;
    return 1;
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
    int32_t c2, c3;
    switch (c) {
        /* special characters */
        case EOF:
            return createToken(TOKEN_EOF, NULL, NULL);
        case '#':
            stackPush(&lex->token_stack, (int8_t)STATE_COMMENT);
            return nullToken();
        case '\n':
            return createToken(TOKEN_NEWLINE, NULL, NULL);

        /* whitespaces */
        case ' ': case '\t':
            return nullToken();

        /* operators */
        case ';':
            // semicolon and double semicolon
            c2 = getCharacter(lex);
            if(c2 == ';') return createToken(TOKEN_DOUBLE_SEMI, NULL, NULL);
            if(c2 == '&') return createToken(TOKEN_SEMI_AND, NULL, NULL);
            lex->lookahead = c2;
            return createToken(TOKEN_SEMI, NULL, NULL);
        case '|':
            // pipe and OR_IF
            c2 = getCharacter(lex);
            if(c2 == '|') return createToken(TOKEN_OR_IF, NULL, NULL);
            lex->lookahead = c2;
            return createToken(TOKEN_PIPE, NULL, NULL);
        case '&':
        // ampersand and AND_IF
            c2 = getCharacter(lex);
            if(c2 == '&') return createToken(TOKEN_AND_IF, NULL, NULL);
            lex->lookahead = c2;
            return createToken(TOKEN_BG, NULL, NULL);
        // simple braces, !
        case '(':
            return createToken(TOKEN_LPAREN, NULL, NULL);
        case ')':
            return createToken(TOKEN_RPAREN, NULL, NULL);
        case '{':
            return createToken(TOKEN_LBRACE, NULL, NULL);
        case '}':
            return createToken(TOKEN_RBRACE, NULL, NULL);
        case '!':
            return createToken(TOKEN_BANG, NULL, NULL);
        case '>':
        // >, >>, >&, >| (clobber)
            c2 = getCharacter(lex);
            if(c2 == '>') return createToken(TOKEN_DGREAT, NULL, NULL);
            if(c2 == '&') return createToken(TOKEN_GREATAND, NULL, NULL);
            if(c2 == '|') return createToken(TOKEN_CLOBBER, NULL, NULL);
            lex->lookahead = c2;
            return createToken(TOKEN_GREAT, NULL, NULL);
        case '<':
        // <, <<, <<<, <<-, <a&, <>
            c2 = getCharacter(lex);
            // handle <<, <<<, <<-
            if(c2 == '<') {
                c3 = getCharacter(lex);
                if(c3 == '<') return createToken(TOKEN_TLESS, NULL, NULL);
                if(c3 == '-') return createToken(TOKEN_DLESSDASH, NULL, NULL);
                lex->lookahead = c3;
                return createToken(TOKEN_DLESS, NULL, NULL);
            }
            // handle <&, <>
            if(c2 == '&') return createToken(TOKEN_LESSAND, NULL, NULL);
            if(c2 == '>') return createToken(TOKEN_LESSGREAT, NULL, NULL);
            lex->lookahead = c2;
            return createToken(TOKEN_LESS, NULL, NULL);
            // different word starts
        case '"': case '\'': case '\\':
            // word token starts
            stackPush(&lex->token_stack, (int8_t)STATE_WORD);
            if(c == '\'') {
                stackPush(&lex->token_stack, (int8_t)STATE_SINGLE_QUOTE);
            } else if(c == '"') {
                stackPush(&lex->token_stack, (int8_t)STATE_DOUBLE_QUOTE);
            } else if(c == '\\') {
                stackPush(&lex->token_stack, (int8_t)STATE_BACKSLASH);
            }
            return nullToken();
        default:
            // regular word tokens, all word delimiters are handled above
            stackPush(&lex->token_stack, (int8_t)STATE_WORD);
            if (!pushBuffer(lex, (char)c, QUOTE_UNQUOTED)) {
                return errorToken();
            }
            return nullToken();
    }
}


static Token handleComment(LexerPtr lex, int32_t c) {
    // if its newlind ore end exit otherwise ignore
    if(c == '\n' || c == EOF) {
        stackPop(&lex->token_stack);
        return c == EOF ? createToken(TOKEN_EOF, NULL, NULL) : createToken(TOKEN_NEWLINE, NULL, NULL);
    }
    return nullToken();
}


static Token handleWord(LexerPtr lex, int32_t c) {
    if(isWordDelimiter(c)) {
        lex->lookahead = c;
        stackPop(&lex->token_stack);  /* pop WORD */
        lex->char_buffer[lex->buffer_pos] = '\0';
        char* word = strdup(lex->char_buffer);
        if(word == NULL) {
            return errorToken();
        }

        int8_t* quotes = NULL;
        if(lex->buffer_pos > 0) {
            quotes = malloc(lex->buffer_pos);
            if(quotes == NULL) {
                free(word);
                return errorToken();
            }
            memcpy(quotes, lex->type_buffer, lex->buffer_pos);
        }

        if(c == '<' || c == '>') {
            for(int i = 0; i < lex->buffer_pos; i++) {
                if(word[i] < '0' || word[i] > '9') {
                    return createToken(TOKEN_WORD, word, quotes); // word
                }
            }
            return createToken(TOKEN_IO_NUM, word, NULL); // io
        }
        return createToken(TOKEN_WORD, word, quotes); // word
    }

    if(c == '\'') {
        stackPush(&lex->token_stack, (int8_t)STATE_SINGLE_QUOTE);
    } else if(c == '"') {
        stackPush(&lex->token_stack, (int8_t)STATE_DOUBLE_QUOTE);
    } else if(c == '\\') {
        stackPush(&lex->token_stack, (int8_t)STATE_BACKSLASH);
    } else {
        if (!pushBuffer(lex, (char)c, QUOTE_UNQUOTED)) {
             return errorToken();
        }
    }
    return nullToken();
}


static Token handleBackslash(LexerPtr lex, int32_t c) {
    if(c == EOF) {
        fprintf(stderr, "unexpected EOF after backslash at line %d\n", lex->line);
        return errorToken();
    }
    // explicit '\n', line continuation
    if(c == '\n') {
        lex->line++;
        stackPop(&lex->token_stack);
        return nullToken();
    }
    // push token
    if(!pushBuffer(lex, (char)c, QUOTE_ESCAPED)) {
        return errorToken();
    }
    // pop state
    stackPop(&lex->token_stack);
    return nullToken();
}


static Token handleSingleQuote(LexerPtr lex, int32_t c) {
    if(c == EOF) {
        fprintf(stderr, "unterminated single quote at line %d\n", lex->line);
        return errorToken();
    }

    if (c != '\'') {
        if(!pushBuffer(lex, (char)c, QUOTE_SINGLE_QUOTED)) {
            return errorToken();
        } 
    } else {
        stackPop(&lex->token_stack); // exit single quote
    }
    return nullToken();
}


static Token handleDoubleQuote(LexerPtr lex, int32_t c) {
    if(c == EOF) {
        fprintf(stderr, "unterminated double quote at line %d\n", lex->line);
        return errorToken();
    }
    if(c == '"') {
        stackPop(&lex->token_stack);
    } else if(c == '\\') {
        stackPush(&lex->token_stack, (int8_t)STATE_DQ_BACKSLASH);
    } else {
        if (!pushBuffer(lex, (char)c, QUOTE_DOUBLE_QUOTED)) {
            return errorToken();
        }
    }
    return nullToken();
}


static Token handleDQBackslash(LexerPtr lex, int32_t c) {
    if(c == EOF) {
        fprintf(stderr, "unterminated double quote at line %d\n", lex->line);
        return errorToken();
    }
    if(c == '\n') {
        lex->line++;
        stackPop(&lex->token_stack);
        return nullToken();
    }
    // escaped characters based on POSIX
    if(c != '"' && c != '\\' && c != '$' && c != '`') {
        if(!pushBuffer(lex, '\\', QUOTE_DOUBLE_QUOTED)) {
            return errorToken();
        }
        if(!pushBuffer(lex, (char)c, QUOTE_DOUBLE_QUOTED)) { 
            return errorToken();
        }
    }
    if(!pushBuffer(lex, (char)c, QUOTE_ESCAPED)) { 
        return errorToken();
    }
    stackPop(&lex->token_stack);
    return nullToken();
}


StatusEnum lexerCtor(LexerPtr lex, FILE* input) {
    if(!lex) return ERROR_DEFAULT;
    lex->input = input;
    lex->line = 1;
    lex->lookahead = -1;
    lex->buffer_pos = 0;
    if(stackInit(&lex->token_stack) != SUCCESS) { 
        return ERROR_DEFAULT;
    }
    stackPush(&lex->token_stack, (int8_t)STATE_START);
    return SUCCESS;
}


void lexerDtor(LexerPtr lex) {
    if(lex == NULL) return;
    if(lex->input != NULL && lex->input != stdin) {
        fclose(lex->input);
    }
}


Token getToken(LexerPtr lex) {
    if(lex == NULL) {
        return errorToken();
    }

    lex->buffer_pos = 0; // reset buffer
    lex->char_buffer[0] = '\0';
    int32_t c;
    int8_t state_val;
    Token t = nullToken();

    while(t.type == TOKEN_NULL) {
        c = getCharacter(lex);
        
        // get top state
        if(stackTop(&lex->token_stack, &state_val) != SUCCESS) {
            return errorToken();
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
    if(!tok) return;
    
    if(tok->value) {
        free(tok->value);
        tok->value = NULL;
    }

    if(tok->char_types) {
        free(tok->char_types);
        tok->char_types = NULL;
    }
}

void lexerReset(LexerPtr lex, FILE* input) {
    if(lex == NULL) {
        return;
    }

    if(lex->input != NULL && lex->input != stdin) {
        fclose(lex->input);  // close old file before replacing
    }

    // reset buffer and line number
    lex->buffer_pos = 0;
    lex->char_buffer[0] = '\0';
    lex->line = 1;
    lex->lookahead = -1;

    // reset state stack to initial state
    while(!stackIsEmpty(&lex->token_stack)) {
        stackPop(&lex->token_stack);
    }
    stackPush(&lex->token_stack, (int8_t)STATE_START);
    
    // reset input
    lex->input = input;
}