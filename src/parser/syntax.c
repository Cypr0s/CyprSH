#include "parser/syntax.h"

// utilities
static void advanceTokens(ParserPtr parser);
static ASTNodePtr createAndInsert(ASTNodePtr parent, NodeTypeEnum type, char* value);


// recursive descent parsing functions
static void analyseLineBreak(ParserPtr parser);
static StatusEnum analyseNewline(ParserPtr parser);

static StatusEnum analyzeProgram(ParserPtr parser, ASTNodePtr ast_root);
static StatusEnum analyzeCompleteCommand(ParserPtr parser, ASTNodePtr complete_command_node);
static StatusEnum analyzeList(ParserPtr parser, ASTNodePtr list_node);
static StatusEnum analyzeAndOr(ParserPtr parser, ASTNodePtr and_or_node);


static void advanceTokens(ParserPtr parser) {
    tokenFree(&parser->current_token);
    parser->current_token = parser->peek_token;
    parser->peek_token = getToken(parser->lexer);
}


static ASTNodePtr createAndInsert(ASTNodePtr parent, NodeTypeEnum type, char* value) {
    ASTNodePtr node = ASTNodeCtor(type, value);
    if(node == NULL) {
        return NULL;
    }

    if(ASTaddChild(parent, node) != SUCCESS) {
        ASTNodeDtor(node);
        return NULL;
    }
    return node;
}

static void analyseLineBreak(ParserPtr parser) {
    while(parser->current_token.type == TOKEN_NEWLINE) {
        advanceTokens(parser);
    }
}

static StatusEnum analyseNewline(ParserPtr parser) {
    if(parser->current_token.type != TOKEN_NEWLINE) {
        fprintf(stderr, "CyprSH: syntax error at line %d: expected newline, got '%s'\n", parser->lexer->line, parser->current_token.value);
        return ERROR_SHELL_MISUSE;
    }
    while(parser->current_token.type == TOKEN_NEWLINE) {
        advanceTokens(parser);
    }
    return SUCCESS;
}

static StatusEnum analyzeProgram(ParserPtr parser, ASTNodePtr ast_root) {
    // program -> newline complete_command newline EOF
    analyseLineBreak(parser);
    if(parser->current_token.type == TOKEN_EOF) {
        return SUCCESS;
    }

    // creat command node
    ASTNodePtr complete_command_node = createAndInsert(ast_root, NODE_COMPLETE_COMMAND, NULL);
    if(complete_command_node == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // analyze complete_command
    StatusEnum st = analyzeCompleteCommand(parser, complete_command_node);
    ERR_CHECK(st);

    // complete_commands: complete_commands newline_list complete_command
    while(parser->current_token.type == TOKEN_NEWLINE) {
        st = analyseNewline(parser);
        ERR_CHECK(st);

        if(parser->current_token.type == TOKEN_EOF) {
            break;
        }

        // creat command node
        ASTNodePtr complete_command_node = createAndInsert(ast_root, NODE_COMPLETE_COMMAND, NULL);
        if(complete_command_node == NULL) {
            return ERROR_MALLOC_FAILURE;
        }
        
        st = analyzeCompleteCommand(parser, complete_command_node);
        ERR_CHECK(st);
    
    }
    return SUCCESS;
}


static StatusEnum analyzeCompleteCommand(ParserPtr parser, ASTNodePtr complete_command) {
    StatusEnum st = analyzeList(parser, complete_command);
    ERR_CHECK(st);

    if(parser->current_token.type == TOKEN_BG) {
        complete_command->flags |= FLAG_BACKGROUND;
        advanceTokens(parser);
    } else if(parser->current_token.type == TOKEN_SEMI) {
        advanceTokens(parser);
    }

    return SUCCESS;
}


static StatusEnum analyzeList(ParserPtr parser, ASTNodePtr complete_command) {
    // create list
    ASTNodePtr list = ASTNodeCtor(NODE_LIST, NULL);
    if(list == NULL) { 
        return ERROR_MALLOC_FAILURE; 
    }
    // parse first and_or
    ASTNodePtr and_or = createAndInsert(list, NODE_AND_OR, NULL);
    if(and_or == NULL) {
        ASTNodeDtor(list);
        return ERROR_MALLOC_FAILURE; 
    }

    // analyse and or
    StatusEnum st = analyzeAndOr(parser, and_or);
    if(st != SUCCESS) {
        ASTFreeTree(list);
        return st;
    }


    while(parser->current_token.type == TOKEN_SEMI || parser->current_token.type == TOKEN_BG) {

        int8_t op = parser->current_token.type == TOKEN_SEMI ? FLAG_SEMICOLON : FLAG_BACKGROUND;
        advanceTokens(parser);

        ASTNodePtr new_list = ASTNodeCtor(NODE_LIST, NULL);
        if(new_list == NULL) { 
            ASTFreeTree(list); 
            return ERROR_MALLOC_FAILURE; 
        }
        new_list->flags = op;

        ASTaddChild(new_list, list);  // previous is lower in hierarchy

        ASTNodePtr and_or_right = createAndInsert(new_list, NODE_AND_OR, NULL);
        if(and_or_right == NULL) { 
            ASTFreeTree(new_list); // also frees previous lists
            return ERROR_MALLOC_FAILURE; 
        }

        st = analyzeAndOr(parser, and_or_right);
        if(st != SUCCESS) { 
            ASTFreeTree(new_list); 
            return st; 
        }

        list = new_list;
    }

    ASTaddChild(complete_command, list);
    return SUCCESS;
}


static StatusEnum analyzeAndOr(ParserPtr parser, ASTNodePtr and_or) {
    return SUCCESS;
}




StatusEnum parserCtor(ParserPtr parser, LexerPtr lexer) {
    if(lexer == NULL || parser == NULL) {
        fprintf(stderr, "CyprSH: NULL pointer in initializing parser\n");
        return ERROR_DEFAULT;
    }

    parser->lexer = lexer;
    parser->current_token = nullToken();
    parser->peek_token = nullToken();
    return SUCCESS;
}


void parserDtor(ParserPtr parser) {
    if(parser == NULL) {
        return;
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

    tokenFree(&parser->current_token);
    tokenFree(&parser->peek_token);
    parser->current_token = getToken(parser->lexer);
    parser->peek_token = getToken(parser->lexer);
}

StatusEnum analyze(ParserPtr parser, ASTNodePtr ast_root) {
    if(parser == NULL || parser->lexer == NULL || ast_root == NULL) {
        fprintf(stderr, "CyprSH: NULL pointer in analyzing syntax\n");
        return ERROR_SYNTAX_ERROR;
    }

    StatusEnum st = analyzeProgram(parser, ast_root);
    if(st != SUCCESS) {
        ASTFreeTree(ast_root);
        fprintf(stderr, "CyprSH: syntax analysis failed with error code %d\n", st);
        return st;
    }
    return SUCCESS;
}