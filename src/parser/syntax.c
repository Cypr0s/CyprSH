#include "parser/syntax.h"

static void advanceTokens(ParserPtr parser);
static StatusEnum analyzeProgram(ParserPtr parser);
static StatusEnum analyzeCommand(ParserPtr parser);

static void advanceTokens(ParserPtr parser) {
    tokenFree(&parser->current_token);
    parser->current_token = parser->peek_token;
    parser->peek_token = getToken(parser->lexer);
}


static StatusEnum analyseNewline(ParserPtr parser) {
    while(parser->current_token.type == TOKEN_NEWLINE) {
        advanceTokens(parser);
    }
    return SUCCESS;
}

static StatusEnum analyzeProgram(ParserPtr parser) {
    StatusEnum st = analyseNewline(parser);
    ERR_CHECK(st);

    if(parser->current_token.type == TOKEN_EOF) {
        return SUCCESS;
    }

    st = analyzeCompleteCommand(parser);
    ERR_CHECK(st);

    st = analyseNewline(parser);
    ERR_CHECK(st);

    if(parser->current_token.type != TOKEN_EOF) {
        fprintf(stderr, "CyprSH: syntax error at line %d: unexpected token '%s'\n", parser->lexer->line, parser->current_token.value);
        return ERROR_SHELL_MISUSE;
    }
    return SUCCESS;
}



StatusEnum parserCtor(ParserPtr parser, LexerPtr lexer) {
    if(lexer == NULL || parser == NULL) {
        fprintf(stderr, "CyprSH: NULL pointer in initializing parser\n");
        return ERROR_DEFAULT;
    }

    parser->lexer = lexer;
    parser->ast_root = NULL;
    parser->current_token = nullToken();
    parser->peek_token = nullToken();
    return SUCCESS;
}


void parserDtor(ParserPtr parser) {
    if(parser == NULL) {
        return;
    }
    if(parser->ast_root != NULL) {
        freeTree(parser->ast_root);
        parser->ast_root = NULL;
    }
    tokenFree(&parser->current_token);
    tokenFree(&parser->peek_token);
    parser->lexer = NULL;
}


void parserReset(ParserPtr parser) {
    if(parser == NULL || parser->lexer == NULL) {
        fprintf(stderr, "CyprSH: NULL pointer in reseting parser\n");
        return;
    }

    if(parser->ast_root != NULL) {
        freeTree(parser->ast_root);
        parser->ast_root = NULL;
    }
    tokenFree(&parser->current_token);
    tokenFree(&parser->peek_token);
    parser->current_token = getToken(parser->lexer);
    parser->peek_token = getToken(parser->lexer);
}

StatusEnum analyze(ParserPtr parser) {
    if(parser == NULL) {
        fprintf(stderr, "CyprSH: NULL pointer in analyzing syntax\n");
        return ERROR_DEFAULT;
    }

    StatusEnum st = analyzeProgram(parser);
    if(st != SUCCESS) {
        fprintf(stderr, "CyprSH: syntax analysis failed with error code %d\n", st);
        return st;
    }
    return SUCCESS;
}