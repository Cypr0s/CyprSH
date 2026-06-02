#include "parser/syntax.h"

// utilities
static const char* COMPOUND_END_KEYWORDS[] = {
    "then", "else", "elif", "fi",
    "do", "done", "esac", NULL
};

static void advanceTokens(ParserPtr parser);
static ASTNodePtr createAndInsert(ASTNodePtr parent, NodeTypeEnum type, char* value);
static uint8_t isRedirectOperator(TokenTypeEnum type);
static int32_t tokenToRedirectType(TokenTypeEnum type);

// recursive descent parsing functions
static void analyzeLineBreak(ParserPtr parser);
static StatusEnum analyzeNewline(ParserPtr parser);

static StatusEnum analyzeProgram(ParserPtr parser, ASTNodePtr ast_root);
static StatusEnum analyzeCompleteCommand(ParserPtr parser, ASTNodePtr complete_command_node);
static StatusEnum analyzeList(ParserPtr parser, ASTNodePtr list_node);
static StatusEnum analyzeAndOr(ParserPtr parser, ASTNodePtr and_or_node);
static StatusEnum analyzePipeline(ParserPtr parser, ASTNodePtr and_or_node);
static StatusEnum analyzeCommand(ParserPtr parser, ASTNodePtr pipeline_node);
static StatusEnum analyzeSimpleCommand(ParserPtr parser, ASTNodePtr pipeline_node);
static StatusEnum analyzeCmdPrefix(ParserPtr parser, ASTNodePtr command_node);
static StatusEnum analyzeCmdSuffix(ParserPtr parser, ASTNodePtr command_node);
static StatusEnum analyzeRedirect(ParserPtr parser, ASTNodePtr command_node);


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
        case TOKEN_CLOBBER: // >|
        case TOKEN_TLESS:  // <<<
            return 1U;
        default:
            return 0U;
    }
}

static int32_t tokenToRedirectType(TokenTypeEnum type) {
    switch(type) {
        case TOKEN_LESS:      return REDIR_LESS;
        case TOKEN_GREAT:     return REDIR_GREAT;
        case TOKEN_DLESS:     return REDIR_DLESS;
        case TOKEN_DGREAT:    return REDIR_DGREAT;
        case TOKEN_LESSAND:   return REDIR_LESSAND;
        case TOKEN_GREATAND:  return REDIR_GREATAND;
        case TOKEN_LESSGREAT: return REDIR_LESSGREAT;
        case TOKEN_CLOBBER:   return REDIR_CLOBBER;
        case TOKEN_DLESSDASH: return REDIR_DLESSDASH;
        case TOKEN_TLESS:     return REDIR_TLESS;
        default:              return REDIR_NONE;
    }
}

static uint8_t isCompoundListEnd(ParserPtr parser) {
    if(parser->current_token.type == TOKEN_EOF)    return 1U;
    if(parser->current_token.type == TOKEN_RPAREN) return 1U;
    if(parser->current_token.type == TOKEN_RBRACE) return 1U;
    if(parser->current_token.type != TOKEN_WORD)   return 0U;
    if(parser->current_token.value == NULL)        return 0U;

    for(uint8_t i = 0; COMPOUND_END_KEYWORDS[i] != NULL; i++) {
        if(streq(parser->current_token.value, COMPOUND_END_KEYWORDS[i])) {
            return 1U;
        }
    }
    return 0U;
}

static void analyzeLineBreak(ParserPtr parser) {
    while(parser->current_token.type == TOKEN_NEWLINE) {
        advanceTokens(parser);
    }
}

static StatusEnum analyzeNewline(ParserPtr parser) {
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
    analyzeLineBreak(parser);
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
        st = analyzeNewline(parser);
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
        // get the list node (last child of complete_command)
        ASTNodePtr list = complete_command->children[complete_command->num_children - 1];
        // get last and_or in the list
        ASTNodePtr last = list->children[list->num_children - 1];
        last->flags |= FLAG_BACKGROUND;
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

    // analyze and or
    StatusEnum st = analyzeAndOr(parser, list);
    if(st != SUCCESS) {
        ASTFreeTree(list);
        return st;
    }


    while(parser->current_token.type == TOKEN_SEMI || parser->current_token.type == TOKEN_BG) {

        int8_t op = parser->current_token.type == TOKEN_SEMI ? FLAG_NONE : FLAG_BACKGROUND;
        advanceTokens(parser);

        if(parser->current_token.type == TOKEN_NEWLINE || parser->current_token.type == TOKEN_EOF) {
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

    // analyze pipeline
    StatusEnum st = analyzePipeline(parser, and_or);
    if(st != SUCCESS) { 
        ASTFreeTree(and_or); 
        return st; 
    }

    while(parser->current_token.type == TOKEN_AND_IF || parser->current_token.type == TOKEN_OR_IF) {

        int8_t op = parser->current_token.type == TOKEN_AND_IF ? FLAG_AND : FLAG_OR;
        advanceTokens(parser);
        analyzeLineBreak(parser); // linebreak between AND/OR and pipeline

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
        analyzeLineBreak(parser); // linebreak

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
    ASTNodePtr command = createAndInsert(pipeline, NODE_SIMPLE_COMMAND, NULL);
    if(command == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // cmd prefix, !todo TOKEN_IO_LOCATION!
    if((parser->current_token.type == TOKEN_WORD && isAssignemntWord(parser->current_token.value)) ||
        parser->current_token.type == TOKEN_IO_NUM || 
        isRedirectOperator(parser->current_token.type)) {
        ASTNodePtr cmd_prefix = createAndInsert(command, NODE_CMD_PREFIX, NULL);
        if(cmd_prefix == NULL) {
            return ERROR_MALLOC_FAILURE;
        }
    
        StatusEnum st = analyzeCmdPrefix(parser, cmd_prefix);
        ERR_CHECK(st);
    }

    // cmd word
    if(parser->current_token.type == TOKEN_WORD) {
        ASTNodePtr cmd_word = createAndInsert(command, NODE_CMD_WORD, parser->current_token.value);
        if(cmd_word == NULL) {
            return ERROR_MALLOC_FAILURE;
        }
        advanceTokens(parser);

        // cmd suffix
        if(parser->current_token.type == TOKEN_WORD ||
           parser->current_token.type == TOKEN_IO_NUM ||
           isRedirectOperator(parser->current_token.type)) 
        {

            ASTNodePtr cmd_suffix = createAndInsert(command, NODE_CMD_SUFFIX, NULL);
            if(!cmd_suffix) { 
                return ERROR_MALLOC_FAILURE;
            }

            StatusEnum st = analyzeCmdSuffix(parser, cmd_suffix);
            ERR_CHECK(st);
        }
    }

    // simple command must have at least a prefix or a word
    if(command->num_children == 0) {
        return ERROR_SYNTAX_ERROR;
    }

    return SUCCESS;
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


static StatusEnum analyzeCmdSuffix(ParserPtr parser, ASTNodePtr cmd_suffix) {
    while(parser->current_token.type == TOKEN_WORD ||
          parser->current_token.type == TOKEN_IO_NUM ||
          isRedirectOperator(parser->current_token.type)) 
    {
        if(parser->current_token.type == TOKEN_WORD) {
            ASTNodePtr word = createAndInsert(cmd_suffix, NODE_WORD, parser->current_token.value);
            if(word == NULL) { 
                return ERROR_MALLOC_FAILURE; 
            }
            advanceTokens(parser);
        } else if(parser->current_token.type == TOKEN_IO_NUM || isRedirectOperator(parser->current_token.type)) {
            StatusEnum st = analyzeRedirect(parser, cmd_suffix);
            ERR_CHECK(st);
        }
    }
    return SUCCESS;
}


static StatusEnum analyzeSubShell(ParserPtr parser, ASTNodePtr pipeline) {
    // create subshell node
    ASTNodePtr subshell = createAndInsert(pipeline, NODE_SUBSHELL, NULL);
    if(subshell == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // expect LPAREN
    if(parser->current_token.type != TOKEN_LPAREN) {
        fprintf(stderr, "CyprSH: syntax error at line %d: expected '('\n", parser->lexer->line);
        return ERROR_SYNTAX_ERROR;
    }
    advanceTokens(parser);

    // parse complete_command
    StatusEnum st = analyzeCompoundList(parser, subshell);
    ERR_CHECK(st);

    // expect RPAREN
    if(parser->current_token.type != TOKEN_RPAREN) {
        fprintf(stderr, "CyprSH: syntax error at line %d: expected ')'\n", parser->lexer->line);
        return ERROR_SYNTAX_ERROR;
    }

    advanceTokens(parser);

    return SUCCESS;
}


static StatusEnum analyzeCompoundList(ParserPtr parser, ASTNodePtr parent) {
    analyzeLineBreak(parser); // linebreaks
    // analyze and or
    StatusEnum st = analyzeTerm(parser, parent);
    ERR_CHECK(st);

    // optional trailing separator
    if(parser->current_token.type == TOKEN_SEMI) {
        advanceTokens(parser);  // just consume
    } else if(parser->current_token.type == TOKEN_BG) {
        ASTNodePtr list = parent->children[parent->num_children - 1];
        ASTNodePtr last = list->children[list->num_children - 1];
        last->flags |= FLAG_BACKGROUND;
        advanceTokens(parser);
    }

    analyzeLineBreak(parser);
    return SUCCESS;
}

static StatusEnum analyzeTerm(ParserPtr parser, ASTNodePtr compound_node) {
    ASTNodePtr term = ASTNodeCtor(NODE_LIST, NULL);
    if(term == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    StatusEnum st = analyzeAndOr(parser, term);
    if(st != SUCCESS) {
        ASTFreeTree(term);
        return st;
    }

    while(parser->current_token.type == TOKEN_SEMI || parser->current_token.type == TOKEN_BG) {

        int8_t op = parser->current_token.type == TOKEN_SEMI ? FLAG_NONE : FLAG_BACKGROUND;
        advanceTokens(parser);
        analyzeLineBreak(parser);

        // trailing separator — closing keyword instead of newline/EOF
        if(isCompoundListEnd(parser)) {
            term->flags |= FLAG_BACKGROUND;
            break;
        }

        ASTNodePtr new_term = ASTNodeCtor(NODE_LIST, NULL);
        if(new_term == NULL) {
            ASTFreeTree(term);
            return ERROR_MALLOC_FAILURE;
        }
        new_term->flags = op;

        ASTaddChild(new_term, term);

        st = analyzeAndOr(parser, new_term);
        if(st != SUCCESS) {
            ASTFreeTree(new_term);
            return st;
        }

        term = new_term;
    }

    ASTaddChild(compound_node, term);
    return SUCCESS;
}

static StatusEnum analyzeBraceGroup(ParserPtr parser, ASTNodePtr pipeline) {
    // create group node
    ASTNodePtr group = createAndInsert(pipeline, NODE_BRACE_GROUP, NULL);
    if(group == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // expect LBRACE
    if(parser->current_token.type != TOKEN_LBRACE) {
        fprintf(stderr, "CyprSH: syntax error at line %d: expected '{'\n", parser->lexer->line);
        return ERROR_SYNTAX_ERROR;
    }
    advanceTokens(parser);

    // parse complete_command
    StatusEnum st = analyzeCompoundList(parser, group);
    ERR_CHECK(st);

    // expect RBRACE
    if(parser->current_token.type != TOKEN_RBRACE) {
        fprintf(stderr, "CyprSH: syntax error at line %d: expected '}'\n", parser->lexer->line);
        return ERROR_SYNTAX_ERROR;
    }

    advanceTokens(parser);

    return SUCCESS;
}

static StatusEnum analyzeIfClause(ParserPtr parser, ASTNodePtr pipeline) {
    // create if node
    ASTNodePtr if_clause = createAndInsert(pipeline, NODE_IF_CLAUSE, NULL);
    if(if_clause == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    if(!streq(parser->current_token.value, "if")) {
        fprintf(stderr, "CyprSH: syntax error at line %d: expected 'if'\n", parser->lexer->line);
        return ERROR_SYNTAX_ERROR;
    }
    advanceTokens(parser);

    StatusEnum st = analyzeCompoundList(parser, if_clause);
    ERR_CHECK(st);

    if(!streq(parser->current_token.value, "then")) {
        fprintf(stderr, "CyprSH: syntax error at line %d: expected 'then'\n", parser->lexer->line);
        return ERROR_SYNTAX_ERROR;
    }
    advanceTokens(parser);

    if(streq(parser->current_token.value, "else") || streq(parser->current_token.value, "elif")) {
        analyzeElsePart(parser, if_clause);
        ERR_CHECK(st);
    }

    if(!streq(parser->current_token.value, "fi")) {
        fprintf(stderr, "CyprSH: syntax error at line %d: expected 'fi'\n", parser->lexer->line);
        return ERROR_SYNTAX_ERROR;
    }
    advanceTokens(parser);

    return SUCCESS;
}


StatusEnum analyzeElsePart(ParserPtr parser, ASTNodePtr if_clause) {
    if(streq(parser->current_token.value, "else")) {
        advanceTokens(parser);
        ASTNodePtr else_clause = createAndInsert(if_clause, NODE_ELSE_CLAUSE, NULL);
        StatusEnum st = analyzeCompoundList(parser, else_clause);
        ERR_CHECK(st);
    } else if(streq(parser->current_token.value, "elif")) {
        advanceTokens(parser);

        ASTNodePtr elif_clause = createAndInsert(if_clause, NODE_IF_CLAUSE, NULL);
        if(!elif_clause) return ERROR_MALLOC_FAILURE;

        StatusEnum st = analyzeCompoundList(parser, elif_clause);
        ERR_CHECK(st);

        if(!streq(parser->current_token.value, "then")) {
            fprintf(stderr, "CyprSH: syntax error at line %d: expected 'then'\n", parser->lexer->line);
            return ERROR_SYNTAX_ERROR;
        }
        advanceTokens(parser);

        st = analyzeCompoundList(parser, elif_clause);
        ERR_CHECK(st);

        if(streq(parser->current_token.value, "elif") ||
           streq(parser->current_token.value, "else")) 
        {
            st = analyzeElsePart(parser, elif_clause);
            ERR_CHECK(st);
        }
    }
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