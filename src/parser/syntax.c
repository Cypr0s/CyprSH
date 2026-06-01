#include "parser/syntax.h"

// utilities
static void advanceTokens(ParserPtr parser);
static ASTNodePtr createAndInsert(ASTNodePtr parent, NodeTypeEnum type, char* value);
static uint8_t isRedirectOperator(TokenTypeEnum type);

// recursive descent parsing functions
static void analyseLineBreak(ParserPtr parser);
static StatusEnum analyseNewline(ParserPtr parser);

static StatusEnum analyzeProgram(ParserPtr parser, ASTNodePtr ast_root);
static StatusEnum analyzeCompleteCommand(ParserPtr parser, ASTNodePtr complete_command_node);
static StatusEnum analyzeList(ParserPtr parser, ASTNodePtr list_node);
static StatusEnum analyzeAndOr(ParserPtr parser, ASTNodePtr and_or_node);
static StatusEnum analyzePipeline(ParserPtr parser, ASTNodePtr and_or_node);
static StatusEnum analyzeCommand(ParserPtr parser, ASTNodePtr pipeline_node);
static StatusEnum analyzeSimpleCommand(ParserPtr parser, ASTNodePtr pipeline_node);




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

static uint8_t isRedirectOperator(TokenTypeEnum type) {
    switch(type) {
        case TOKEN_LESS: // <
        case TOKEN_GREAT: // >
        case TOKEN_DLESS: // <<
        case TOKEN_DGREAT: // >>
        case TOKEN_LESSAND: // <&
        case TOKEN_GREATAND: // >&
        case TOKEN_LESSGREAT: // <>
        case TOKEN_DLESSDASH: // <<-
        case TOKEN_TLESS:
            return 1U;
        default:
            return 0U;
    }
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

    // analyse and or
    StatusEnum st = analyzeAndOr(parser, list);
    if(st != SUCCESS) {
        ASTFreeTree(list);
        return st;
    }


    while(parser->current_token.type == TOKEN_SEMI || parser->current_token.type == TOKEN_BG) {

        int8_t op = parser->current_token.type == TOKEN_SEMI ? FLAG_SEMICOLON : FLAG_BACKGROUND;
        advanceTokens(parser);

        if(parser->peek_token.type == TOKEN_NEWLINE || parser->peek_token.type == TOKEN_EOF) {
            break;  // trailing separator, stop here
        }

        ASTNodePtr new_list = ASTNodeCtor(NODE_LIST, NULL);
        if(new_list == NULL) { 
            ASTFreeTree(list); 
            return ERROR_MALLOC_FAILURE; 
        }
        new_list->flags = op;

        ASTaddChild(new_list, list);  // previous is lower in hierarchy

        st = analyzeAndOr(parser, new_list);
        if(st != SUCCESS) { 
            ASTFreeTree(new_list); 
            return st; 
        }

        list = new_list;
    }

    ASTaddChild(complete_command, list);
    return SUCCESS;
}


static StatusEnum analyzeAndOr(ParserPtr parser, ASTNodePtr list) {
    // create and_or
    ASTNodePtr and_or = ASTNodeCtor(NODE_AND_OR, NULL);
    if(and_or == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // analyse pipeline
    StatusEnum st = analyzePipeline(parser, and_or);
    if(st != SUCCESS) { 
        ASTFreeTree(and_or); 
        return st; 
    }

    while(parser->current_token.type == TOKEN_AND_IF || parser->current_token.type == TOKEN_OR_IF) {

        int8_t op = parser->current_token.type == TOKEN_AND_IF ? FLAG_AND : FLAG_OR;
        advanceTokens(parser);
        analyseLineBreak(parser); // linebreak between AND/OR and pipeline

        ASTNodePtr new_and_or = ASTNodeCtor(NODE_AND_OR, NULL);
        if(new_and_or == NULL) { 
            ASTFreeTree(and_or); 
            return ERROR_MALLOC_FAILURE; 
        }
    
        new_and_or->flags = op;

        ASTaddChild(new_and_or, and_or);  // previous is lower

        st = analyzePipeline(parser, new_and_or);
        if(st != SUCCESS) { 
            ASTFreeTree(new_and_or); 
            return st; 
        }

        and_or = new_and_or;
    }

    ASTaddChild(list, and_or);
    return SUCCESS;
}


static StatusEnum analyzePipeline(ParserPtr parser, ASTNodePtr and_or) {
    // create pipeline node
    ASTNodePtr pipeline = ASTNodeCtor(NODE_PIPELINE, NULL);
    if(!pipeline) return ERROR_MALLOC_FAILURE;

    // optional Bang
    int8_t has_bang = 0;
    if(parser->current_token.type == TOKEN_BANG) {
        has_bang = 1;
        advanceTokens(parser);
    }

    // parse first command
    StatusEnum st = analyzeCommand(parser, pipeline);
    if(st != SUCCESS) { 
        ASTFreeTree(pipeline); 
        return st; 
    }

    // pipe_sequence : pipe_sequence '|' linebreak command
    while(parser->current_token.type == TOKEN_PIPE) {
        advanceTokens(parser);
        analyseLineBreak(parser); // linebreak

        ASTNodePtr new_pipeline = ASTNodeCtor(NODE_PIPELINE, NULL);
        if(new_pipeline == NULL) { 
            ASTFreeTree(pipeline); 
            return ERROR_MALLOC_FAILURE; 
        }
        new_pipeline->flags = FLAG_PIPE;

        ASTaddChild(new_pipeline, pipeline);  // previous is right, lower in hierarchy

        st = analyzeCommand(parser, new_pipeline);
        if(st != SUCCESS) { 
            ASTFreeTree(new_pipeline);
            return st;
        }

        pipeline = new_pipeline;
    }

    ASTaddChild(and_or, pipeline);
    if(has_bang) {
        pipeline->flags |= FLAG_BANG;
    }
    return SUCCESS;
}


StatusEnum analyzeCommand(ParserPtr parser, ASTNodePtr pipeline) {
    // function
    if(parser->current_token.type == TOKEN_WORD && parser->peek_token.type == TOKEN_LPAREN) {
        return analyzeFunctionDef(parser, pipeline);
    }
    
    // compound command
    if(streq(parser->current_token.value, "if")) {
        return analyzeIfClause(parser, pipeline);
    } else if(streq(parser->current_token.value, "while")) {
        return analyzeWhileClause(parser, pipeline);
    } else if(streq(parser->current_token.value, "until")) {
        return analyzeUntilClause(parser, pipeline);
    } else if(streq(parser->current_token.value, "for")) {
        return analyzeForClause(parser, pipeline);
    } else if(streq(parser->current_token.value, "case")) {
        return analyzeCaseClause(parser, pipeline);
    } else if (parser->current_token.type == TOKEN_LPAREN) {
        return analyzeSubshell(parser, pipeline);
    } else if (parser->current_token.type == TOKEN_LBRACE) {
        return analyzeGroup(parser, pipeline);
    }

    // simple command
    return analyzeSimpleCommand(parser, pipeline);
}


static StatusEnum analyzeSimpleCommand(ParserPtr parser, ASTNodePtr pipeline) {
    ASTNodePtr command = ASTNodeCtor(NODE_SIMPLE_COMMAND, NULL);
    if(command == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // cmd prefix, !todo TOKEN_IO_LOCATION!
    if((parser->current_token.type == TOKEN_WORD && isAssignemntWord(parser->current_token.value)) ||
        parser->current_token.type == TOKEN_IO_NUM || 
        isRedirectOperator(parser->current_token.type)) {
        ASTNodePtr cmd_prefix = createAndInsert(command, NODE_CMD_PREFIX, NULL);
        if(cmd_prefix == NULL) {
            ASTFreeTree(command);
            return ERROR_MALLOC_FAILURE;
        }
    
        StatusEnum st = analyzeCmdPrefix(parser, cmd_prefix);
        if(st != SUCCESS) {
            ASTFreeTree(command);
            return st;
        }
    }
    // todo
    // cmd word

    // cmd suffix
    return ERROR_DEFAULT;
}


static StatusEnum analyzeCmdPrefix(ParserPtr parser, ASTNodePtr cmd_prefix) {
    while((parser->current_token.type == TOKEN_WORD && isAssignemntWord(parser->current_token.value)) ||
           parser->current_token.type == TOKEN_IO_NUM || 
           isRedirectOperator(parser->current_token.type)) 
    {
        // assignment word
        if(parser->current_token.type == TOKEN_WORD && 
            isAssignemntWord(parser->current_token.value)) 
        {
            ASTNodePtr assign = createAndInsert(cmd_prefix, NODE_ASSIGNMENT_WORD, parser->current_token.value);
    
            if(assign == NULL) {
                 return ERROR_MALLOC_FAILURE;
            }
            advanceTokens(parser);
        } else {
            // IO_NUM or redirect operator — both handled by analyzeRedirect
            StatusEnum st = analyzeRedirect(parser, cmd_prefix);
            ERR_CHECK(st);
        }
    }
    return SUCCESS;
}


static StatusEnum analyzeRedirect(ParserPtr parser, ASTNodePtr cmd_prefix) {
    ASTNodePtr redirect = createAndInsert(cmd_prefix, NODE_REDIRECT, NULL);
    if(redirect == NULL) { 
        return ERROR_MALLOC_FAILURE;
    }

    // optional IO_NUM
    if(parser->current_token.type == TOKEN_IO_NUM) {
        ASTNodePtr io_num = createAndInsert(redirect, NODE_IO_NUM, parser->current_token.value);
        if(io_num == NULL) {
            return ERROR_MALLOC_FAILURE;
        }
        advanceTokens(parser);
    }

    // operator
    if(!isRedirectOperator(parser->current_token.type)) {
        fprintf(stderr, "CyprSH: syntax error at line %d: expected redirect operator\n", parser->lexer->line);
        return ERROR_SYNTAX_ERROR;
    }
    redirect->flags = tokenToRedirectType(parser->current_token.type);
    advanceTokens(parser);

    // filename or delimiter as second child
    if(parser->current_token.type != TOKEN_WORD) {
        fprintf(stderr, "CyprSH: syntax error at line %d: expected redirect target\n", parser->lexer->line);
        return ERROR_SYNTAX_ERROR;
    }
    ASTNodePtr word = createAndInsert(redirect, NODE_WORD, parser->current_token.value);
    if(word == NULL) { 
        return ERROR_MALLOC_FAILURE; 
    }

    advanceTokens(parser);
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