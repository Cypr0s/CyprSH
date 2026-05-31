#include "parser/syntax.h"


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

void analyze(ParserPtr parser) {
    return;
}