/**
 * @file        syntax-parser.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Syntax analyzer implementation
 */

#include "syntax-analysis/syntax-parser.h"

// utilities
static const char* COMPOUND_END_KEYWORDS[] = {
    "then", "else", "elif", "fi",
    "do", "done", "esac", NULL
};

static StatusEnum advanceTokens(ParserPtr parser);
static ASTNodePtr createAndInsertEmpty(ASTNodePtr parent, NodeTypeEnum type);
static ASTNodePtr createAndInsertFromToken(ASTNodePtr parent, NodeTypeEnum type, TokenPtr token);
static uint8_t isRedirectOperator(TokenTypeEnum type);
static int32_t tokenToRedirectType(TokenTypeEnum type);
static uint8_t isAssignmentWord(const char* word);

// recursive descent parsing functions
static StatusEnum analyzeLineBreak(ParserPtr parser);
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
static StatusEnum analyzeTerm(ParserPtr parser, ASTNodePtr compound_node);
static StatusEnum analyzeSubshell(ParserPtr parser, ASTNodePtr pipeline_node);
static StatusEnum analyzeCompoundList(ParserPtr parser, ASTNodePtr parent);
static StatusEnum analyzeBraceGroup(ParserPtr parser, ASTNodePtr pipeline_node);
static StatusEnum analyzeIfClause(ParserPtr parser, ASTNodePtr pipeline_node);
static StatusEnum analyzeElsePart(ParserPtr parser, ASTNodePtr if_clause_node);
static StatusEnum analyzeWhileClause(ParserPtr parser, ASTNodePtr pipeline);
static StatusEnum analyzeDoGroup(ParserPtr parser, ASTNodePtr parent);
static StatusEnum analyzeUntilClause(ParserPtr parser, ASTNodePtr pipeline);
static StatusEnum analyzeForClause(ParserPtr parser, ASTNodePtr pipeline);
static StatusEnum analyzeCaseClause(ParserPtr parser, ASTNodePtr pipeline);
static StatusEnum analyzeCaseItem(ParserPtr parser, ASTNodePtr case_clause);
static StatusEnum analyzeFunctionDef(ParserPtr parser, ASTNodePtr pipeline);
static StatusEnum analyzeCompoundCommand(ParserPtr parser, ASTNodePtr parent);


static StatusEnum advanceTokens(ParserPtr parser) {
    tokenFree(&parser->current_token);
    parser->current_token = parser->peek_token;
    parser->peek_token = getToken(parser->lexer);
 
    return parser->current_token.error_type;
}


static ASTNodePtr createAndInsertEmpty(ASTNodePtr parent, NodeTypeEnum type) {
    if(parent == NULL) {
        printError("createAndInsertEmpty", "Passing NULL pointer");
        return NULL;
    }
 
    ASTNodePtr node = ASTNodeCtor(type, NULL, NULL);
    if(node == NULL) {
        return NULL;
    }
 
    if(ASTaddChild(parent, node) != SUCCESS) {
        ASTNodeDtor(node);
        return NULL;
    }
 
    return node;
}
 

static ASTNodePtr createAndInsertFromToken(ASTNodePtr parent, NodeTypeEnum type, TokenPtr token) {
    if(parent == NULL || token == NULL) {
        printError("createAndInsertFromToken", "Passing NULL pointer");
        return NULL;
    }
 
    ASTNodePtr node = ASTNodeCtor(type, token->value, token->char_types);
    if(node == NULL) {
        return NULL;
    }
 
    if(ASTaddChild(parent, node) != SUCCESS) {
        // ASTaddChild failed — prevent ASTNodeDtor from freeing token's memory
        node->value = NULL;
        node->value_types = NULL;
        ASTNodeDtor(node);
        return NULL;
    }
 
    // Transfer complete — token no longer owns these pointers
    token->value = NULL;
    token->char_types = NULL;
 
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


static uint8_t isCompoundListEnd(Token t) {
    if(t.type == TOKEN_EOF)    return 1U;
    if(t.type == TOKEN_RPAREN) return 1U;
    if(t.type == TOKEN_RBRACE) return 1U;
    if(t.type != TOKEN_WORD)   return 0U;
    if(t.value == NULL)        return 0U;

    for(uint8_t i = 0; COMPOUND_END_KEYWORDS[i] != NULL; i++) {
        if(streq(t.value, COMPOUND_END_KEYWORDS[i])) {
            return 1U;
        }
    }
    return 0U;
}


static uint8_t isAssignmentWord(const char* word) {
    if(word == NULL || !isNameStart(*word)) return 0U;

    const char* p = word + 1;

    while(*p && *p != '=') {
        if(!isNameChar(*p)) {
            return 0U;
        }
        p++;
    }

    return *p == '=' ? 1U : 0U;
}


static StatusEnum analyzeLineBreak(ParserPtr parser) {
    while(parser->current_token.type == TOKEN_NEWLINE) {
        StatusEnum st = advanceTokens(parser);
        if(st != SUCCESS) {
            return st;
        }
    }
    return SUCCESS;
}

static StatusEnum analyzeNewline(ParserPtr parser) {
    if(parser->current_token.type != TOKEN_NEWLINE) {
        printError("analyzeNewline", 
                    "Syntax error at line %d, expected `\\n`", 
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }

    while(parser->current_token.type == TOKEN_NEWLINE) {
        StatusEnum st = advanceTokens(parser);
        if(st != SUCCESS) {
            return st;
        }
    }
    return SUCCESS;
}


static StatusEnum analyzeProgram(ParserPtr parser, ASTNodePtr ast_root) {
    // program -> newline complete_command newline EOF
    StatusEnum st = analyzeLineBreak(parser);
    ERR_CHECK(st);

    if(parser->current_token.type == TOKEN_EOF) {
        return SUCCESS;
    }

    // creat command node
    ASTNodePtr complete_command_node = createAndInsertEmpty(ast_root, NODE_COMPLETE_COMMAND);
    if(complete_command_node == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // analyze complete_command
    st = analyzeCompleteCommand(parser, complete_command_node);
    ERR_CHECK(st);

    // complete_commands: complete_commands newline_list complete_command
    while(parser->current_token.type == TOKEN_NEWLINE) {
        st = analyzeNewline(parser);
        ERR_CHECK(st);

        if(parser->current_token.type == TOKEN_EOF) {
            break;
        }

        // create command node
        ASTNodePtr complete_command_node = createAndInsertEmpty(ast_root, NODE_COMPLETE_COMMAND);
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
        ASTNodePtr list = complete_command->children[complete_command->num_children - 1];
        list->children[list->num_children - 1]->flags |= FLAG_BACKGROUND;
        st = advanceTokens(parser);
        ERR_CHECK(st);
    } else if(parser->current_token.type == TOKEN_SEMI) {
        st = advanceTokens(parser);
        ERR_CHECK(st);
    }

    return SUCCESS;
}


static StatusEnum analyzeList(ParserPtr parser, ASTNodePtr complete_command) {
    // create list
    ASTNodePtr list = ASTNodeCtor(NODE_LIST, NULL, NULL);
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


        if(parser->peek_token.type == TOKEN_NEWLINE || parser->peek_token.type == TOKEN_EOF) {
            break;  // don't consume — leave for analyzeCompleteCommand
        }

        // separator follows the last and_or — flag it
        if(parser->current_token.type == TOKEN_BG) {
            ASTNodePtr last = list->children[list->num_children - 1];
            last->flags |= FLAG_BACKGROUND;
        }
        st = advanceTokens(parser);
        if(st != SUCCESS) {
            ASTFreeTree(list);
            return st;
        }

        st = analyzeAndOr(parser, list);
        if(st != SUCCESS) { 
            ASTFreeTree(list); 
            return st; 
        }
    }

    ASTaddChild(complete_command, list);
    return SUCCESS;
}


static StatusEnum analyzeAndOr(ParserPtr parser, ASTNodePtr list) {
    // create and_or
    ASTNodePtr and_or = ASTNodeCtor(NODE_AND_OR, NULL, NULL);
    if(and_or == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // analyze pipeline
    StatusEnum st = analyzePipeline(parser, and_or);
    if(st != SUCCESS) { 
        ASTFreeTree(and_or); 
        return st; 
    }

    while(parser->current_token.type == TOKEN_AND_IF || 
            parser->current_token.type == TOKEN_OR_IF) 
        {

        // operator follows the last pipeline — flag it
        ASTNodePtr last = and_or->children[and_or->num_children - 1];
        if(parser->current_token.type == TOKEN_AND_IF) {
            last->flags |= FLAG_AND;
        } else {
            last->flags |= FLAG_OR;
        }

        st = advanceTokens(parser);
        if(st != SUCCESS) {
            ASTFreeTree(and_or);
            return st;
        }
        st = analyzeLineBreak(parser); // linebreak between AND/OR and pipeline
        if(st != SUCCESS) {
            ASTFreeTree(and_or);
            return st;
        }

        st = analyzePipeline(parser, and_or);
        if(st != SUCCESS) { 
            ASTFreeTree(and_or); 
            return st; 
        }
    }

    ASTaddChild(list, and_or);
    return SUCCESS;
}


static StatusEnum analyzePipeline(ParserPtr parser, ASTNodePtr and_or) {
    // create pipeline node
    ASTNodePtr pipeline = ASTNodeCtor(NODE_PIPELINE, NULL, NULL);
    if(!pipeline) return ERROR_MALLOC_FAILURE;

    // optional Bang
    if(parser->current_token.type == TOKEN_BANG) {
        pipeline->flags |= FLAG_BANG;

        StatusEnum st = advanceTokens(parser);
        if(st != SUCCESS) {
            ASTFreeTree(pipeline);
            return st;
        }
    }

    // parse first command
    StatusEnum st = analyzeCommand(parser, pipeline);
    if(st != SUCCESS) { 
        ASTFreeTree(pipeline); 
        return st; 
    }

    // pipe_sequence : pipe_sequence '|' linebreak command
    while(parser->current_token.type == TOKEN_PIPE) {

        st = advanceTokens(parser);
        if(st != SUCCESS) {
            ASTFreeTree(pipeline);
            return st;
        }

        st = analyzeLineBreak(parser); // linebreak
        if(st != SUCCESS) {
            ASTFreeTree(pipeline);
            return st;
        }

        st = analyzeCommand(parser, pipeline);
        if(st != SUCCESS) { 
            ASTFreeTree(pipeline);
            return st;
        }
    }

    ASTaddChild(and_or, pipeline);
    return SUCCESS;
}


StatusEnum analyzeCommand(ParserPtr parser, ASTNodePtr pipeline) {
    // check for lexer error token
    if(parser->current_token.type == TOKEN_ERROR) {
        return parser->current_token.error_type;
    }

    // function
    if(parser->current_token.type == TOKEN_WORD && parser->peek_token.type == TOKEN_LPAREN) {
        return analyzeFunctionDef(parser, pipeline);
    }
    
    // compound command
    if(streq(parser->current_token.value, "if") ||
       streq(parser->current_token.value, "while") ||
       streq(parser->current_token.value, "until") ||
       streq(parser->current_token.value, "for") ||
       streq(parser->current_token.value, "case") ||
       parser->current_token.type == TOKEN_LPAREN ||
       parser->current_token.type == TOKEN_LBRACE) 
    {
        return analyzeCompoundCommand(parser, pipeline);
    }

    // simple command
    return analyzeSimpleCommand(parser, pipeline);
}


static StatusEnum analyzeSimpleCommand(ParserPtr parser, ASTNodePtr pipeline) {
    ASTNodePtr command = createAndInsertEmpty(pipeline, NODE_SIMPLE_COMMAND);
    if(command == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // cmd prefix, !todo TOKEN_IO_LOCATION!
    if((parser->current_token.type == TOKEN_WORD && 
        isAssignmentWord(parser->current_token.value)) ||
        parser->current_token.type == TOKEN_IO_NUM || 
        isRedirectOperator(parser->current_token.type)) 
    {
        ASTNodePtr cmd_prefix = createAndInsertEmpty(command, NODE_CMD_PREFIX);
        if(cmd_prefix == NULL) {
            return ERROR_MALLOC_FAILURE;
        }
    
        StatusEnum st = analyzeCmdPrefix(parser, cmd_prefix);
        ERR_CHECK(st);
    }

    // cmd word
    if(parser->current_token.type == TOKEN_WORD) {
        ASTNodePtr cmd_word = createAndInsertFromToken(command, NODE_CMD_WORD, &parser->current_token);
        if(cmd_word == NULL) {
            return ERROR_MALLOC_FAILURE;
        }
        StatusEnum st = advanceTokens(parser);
        ERR_CHECK(st);

        // cmd suffix
        if(parser->current_token.type == TOKEN_WORD ||
           parser->current_token.type == TOKEN_IO_NUM ||
           isRedirectOperator(parser->current_token.type)) 
        {

            ASTNodePtr cmd_suffix = createAndInsertEmpty(command, NODE_CMD_SUFFIX);
            if(!cmd_suffix) { 
                return ERROR_MALLOC_FAILURE;
            }

            st = analyzeCmdSuffix(parser, cmd_suffix);
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
    while((parser->current_token.type == TOKEN_WORD && 
            isAssignmentWord(parser->current_token.value)) ||
            parser->current_token.type == TOKEN_IO_NUM || 
            isRedirectOperator(parser->current_token.type)) 
    {
        // assignment word
        if(parser->current_token.type == TOKEN_WORD && 
            isAssignmentWord(parser->current_token.value)) 
        {
            ASTNodePtr assign = createAndInsertFromToken(cmd_prefix, 
                                                NODE_ASSIGNMENT_WORD,
                                                &(parser->current_token));
    
            if(assign == NULL) {
                 return ERROR_MALLOC_FAILURE;
            }
            StatusEnum st = advanceTokens(parser);
            ERR_CHECK(st);
        } else {
            // IO_NUM or redirect operator — both handled by analyzeRedirect
            StatusEnum st = analyzeRedirect(parser, cmd_prefix);
            ERR_CHECK(st);
        }
    }
    return SUCCESS;
}


static StatusEnum analyzeRedirect(ParserPtr parser, ASTNodePtr cmd_prefix) {
    ASTNodePtr redirect = createAndInsertEmpty(cmd_prefix, NODE_REDIRECT);
    if(redirect == NULL) { 
        return ERROR_MALLOC_FAILURE;
    }

    // optional IO_NUM
    if(parser->current_token.type == TOKEN_IO_NUM) {
        ASTNodePtr io_num = createAndInsertFromToken(redirect, NODE_IO_NUM, &parser->current_token);
        if(io_num == NULL) {
            return ERROR_MALLOC_FAILURE;
        }
        StatusEnum st = advanceTokens(parser);
        ERR_CHECK(st);
    }

    // operator
    if(!isRedirectOperator(parser->current_token.type)) {
        printError("analyzeRedirect", 
                    "Syntax error at line %d, expected `redirect operator`", 
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    redirect->flags = tokenToRedirectType(parser->current_token.type);
    StatusEnum st = advanceTokens(parser);
    ERR_CHECK(st);

    // filename or delimiter as second child
    if(parser->current_token.type != TOKEN_WORD) {
        printError("analyzeRedirect", 
                "Syntax error at line %d, expected `redirect target`", 
                parser->lexer->line
            );
        return ERROR_SYNTAX_ERROR;
    }
    ASTNodePtr word = createAndInsertFromToken(redirect, NODE_WORD, &parser->current_token);
    if(word == NULL) { 
        return ERROR_MALLOC_FAILURE; 
    }

    st = advanceTokens(parser);
    ERR_CHECK(st);
    return SUCCESS;
}


static StatusEnum analyzeCmdSuffix(ParserPtr parser, ASTNodePtr cmd_suffix) {
    while(parser->current_token.type == TOKEN_WORD ||
          parser->current_token.type == TOKEN_IO_NUM ||
          isRedirectOperator(parser->current_token.type)) 
    {
        if(parser->current_token.type == TOKEN_WORD) {
            ASTNodePtr word = createAndInsertFromToken(cmd_suffix, NODE_WORD, &parser->current_token);
            if(word == NULL) { 
                return ERROR_MALLOC_FAILURE; 
            }
            StatusEnum st = advanceTokens(parser);
            ERR_CHECK(st);
        } else if(parser->current_token.type == TOKEN_IO_NUM || 
                    isRedirectOperator(parser->current_token.type)) 
        {
            StatusEnum st = analyzeRedirect(parser, cmd_suffix);
            ERR_CHECK(st);
        }
    }
    return SUCCESS;
}


static StatusEnum analyzeSubshell(ParserPtr parser, ASTNodePtr pipeline) {
    // create subshell node
    ASTNodePtr subshell = createAndInsertEmpty(pipeline, NODE_SUBSHELL);
    if(subshell == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // expect LPAREN
    if(parser->current_token.type != TOKEN_LPAREN) {
        printError("analyzeSubshell", 
                    "Syntax error at line %d: expected `(`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    StatusEnum st = advanceTokens(parser);
    ERR_CHECK(st);

    // parse complete_command
    st = analyzeCompoundList(parser, subshell);
    ERR_CHECK(st);

    // expect RPAREN
    if(parser->current_token.type != TOKEN_RPAREN) {
        printError("analyzeSubshell", 
                    "Syntax error at line %d: expected `)`", 
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }

    st = advanceTokens(parser);
    ERR_CHECK(st);

    return SUCCESS;
}


static StatusEnum analyzeCompoundList(ParserPtr parser, ASTNodePtr parent) {
    StatusEnum st = analyzeLineBreak(parser); // linebreaks
    ERR_CHECK(st);

    // analyze and or
    st = analyzeTerm(parser, parent);
    ERR_CHECK(st);

    // optional trailing separator
    if(parser->current_token.type == TOKEN_SEMI) {
        st = advanceTokens(parser);  // just consume
        ERR_CHECK(st);
    } else if(parser->current_token.type == TOKEN_BG) {
        ASTNodePtr list = parent->children[parent->num_children - 1];
        list->flags |= FLAG_BACKGROUND;
        st = advanceTokens(parser);
        ERR_CHECK(st);
    }

    st = analyzeLineBreak(parser);
    ERR_CHECK(st);
    return SUCCESS;
}

static StatusEnum analyzeTerm(ParserPtr parser, ASTNodePtr compound_node) {
    ASTNodePtr term = ASTNodeCtor(NODE_LIST, NULL, NULL);
    if(term == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    StatusEnum st = analyzeAndOr(parser, term);
    if(st != SUCCESS) {
        ASTFreeTree(term);
        return st;
    }

    while(parser->current_token.type == TOKEN_SEMI || parser->current_token.type == TOKEN_BG) {

        TokenTypeEnum sep_type = parser->current_token.type;
        st = advanceTokens(parser);

        if(st != SUCCESS) {
            ASTFreeTree(term);
            return st;
        }

        st = analyzeLineBreak(parser);
        if(st != SUCCESS) {
            ASTFreeTree(term);
            return st;
        }

        // trailing separator — closing keyword instead of newline/EOF
        if(isCompoundListEnd(parser->current_token)) {
            if(sep_type == TOKEN_BG) {
                ASTNodePtr last = term->children[term->num_children - 1];
                last->flags |= FLAG_BACKGROUND;
            }
            break;
        }

        // separator follows last and_or — flag it
        if(sep_type == TOKEN_BG) {
            ASTNodePtr last = term->children[term->num_children - 1];
            last->flags |= FLAG_BACKGROUND;
        }

        st = analyzeAndOr(parser, term);
        if(st != SUCCESS) {
            ASTFreeTree(term);
            return st;
        }
    }

    ASTaddChild(compound_node, term);
    return SUCCESS;
}

static StatusEnum analyzeBraceGroup(ParserPtr parser, ASTNodePtr pipeline) {
    // create group node
    ASTNodePtr group = createAndInsertEmpty(pipeline, NODE_BRACE_GROUP);
    if(group == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // expect LBRACE
    if(parser->current_token.type != TOKEN_LBRACE) {
        printError("analyzeBraceGroup",
                    "CyprSH: Syntax error at line %d: expected '{'",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    StatusEnum st = advanceTokens(parser);
    ERR_CHECK(st);

    // parse complete_command
    st = analyzeCompoundList(parser, group);
    ERR_CHECK(st);

    // expect RBRACE
    if(parser->current_token.type != TOKEN_RBRACE) {
        printError("analyzeBraceGroup",
                    "CyprSH: Syntax error at line %d: expected '}'",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }

    st = advanceTokens(parser);
    ERR_CHECK(st);

    return SUCCESS;
}

static StatusEnum analyzeIfClause(ParserPtr parser, ASTNodePtr pipeline) {
    // create if node
    ASTNodePtr if_clause = createAndInsertEmpty(pipeline, NODE_IF_CLAUSE);
    if(if_clause == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    if(!streq(parser->current_token.value, "if")) {
        printError("analyzeIfClause", 
                    "Syntax error at line %d: expected `if`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    StatusEnum st = advanceTokens(parser);
    ERR_CHECK(st);

    st = analyzeCompoundList(parser, if_clause);
    ERR_CHECK(st);

    if(!streq(parser->current_token.value, "then")) {
        printError("analyzeIfClause", 
                    "Syntax error at line %d: expected `then`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    st = advanceTokens(parser);
    ERR_CHECK(st);

    st = analyzeCompoundList(parser, if_clause);
    ERR_CHECK(st);

    if(streq(parser->current_token.value, "else") || streq(parser->current_token.value, "elif")) {
        st = analyzeElsePart(parser, if_clause);
        ERR_CHECK(st);
    }

    if(!streq(parser->current_token.value, "fi")) {
        printError("analyzeIfClause", 
                    "Syntax error at line %d: expected `fi`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    st = advanceTokens(parser);
    ERR_CHECK(st);

    return SUCCESS;
}


static StatusEnum analyzeElsePart(ParserPtr parser, ASTNodePtr if_clause) {
    if(streq(parser->current_token.value, "else")) {
        StatusEnum st = advanceTokens(parser);
        ERR_CHECK(st);

        ASTNodePtr else_clause = createAndInsertEmpty(if_clause, NODE_ELSE_CLAUSE);
        if(else_clause == NULL) {
            return ERROR_MALLOC_FAILURE;
        }
        st = analyzeCompoundList(parser, else_clause);
        ERR_CHECK(st);
    } else if(streq(parser->current_token.value, "elif")) {
        StatusEnum st = advanceTokens(parser);
        ERR_CHECK(st);

        ASTNodePtr elif_clause = createAndInsertEmpty(if_clause, NODE_IF_CLAUSE);
        if(!elif_clause) return ERROR_MALLOC_FAILURE;

        st = analyzeCompoundList(parser, elif_clause);
        ERR_CHECK(st);

        if(!streq(parser->current_token.value, "then")) {
            printError("analyzeElsePart", 
                    "Syntax error at line %d: expected `then`",
                    parser->lexer->line
                );
            return ERROR_SYNTAX_ERROR;
        }
        st = advanceTokens(parser);
        ERR_CHECK(st);

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


static StatusEnum analyzeWhileClause(ParserPtr parser, ASTNodePtr pipeline) {
    // create while node
    ASTNodePtr while_clause = createAndInsertEmpty(pipeline, NODE_WHILE_CLAUSE);
    if(while_clause == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    if(!streq(parser->current_token.value, "while")) {
        printError("analyzeWhileClause", 
                    "Syntax error at line %d: expected `while`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    StatusEnum st = advanceTokens(parser);
    ERR_CHECK(st);

    st = analyzeCompoundList(parser, while_clause);
    ERR_CHECK(st);

    st = analyzeDoGroup(parser, while_clause);
    ERR_CHECK(st);
    return SUCCESS;
}


static StatusEnum analyzeDoGroup(ParserPtr parser, ASTNodePtr parent) {

    if(!streq(parser->current_token.value, "do")) {
        printError("analyzeDoGroup", 
                    "Syntax error at line %d: expected `do`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    StatusEnum st = advanceTokens(parser);
    ERR_CHECK(st);

    st = analyzeCompoundList(parser, parent);
    ERR_CHECK(st);

    if(!streq(parser->current_token.value, "done")) {
        printError("analyzeDoGroup", 
                    "Syntax error at line %d: expected `done`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    st = advanceTokens(parser);
    ERR_CHECK(st);

    return SUCCESS;
}


static StatusEnum analyzeUntilClause(ParserPtr parser, ASTNodePtr pipeline) {
    // create until node
    ASTNodePtr until_clause = createAndInsertEmpty(pipeline, NODE_UNTIL_CLAUSE);
    if(until_clause == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    if(!streq(parser->current_token.value, "until")) {
        printError("analyzeUntilClause", 
                    "Syntax error at line %d: expected `until`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    StatusEnum st = advanceTokens(parser);
    ERR_CHECK(st);

    st = analyzeCompoundList(parser, until_clause);
    ERR_CHECK(st);

    st = analyzeDoGroup(parser, until_clause);
    ERR_CHECK(st);
    return SUCCESS;
}


static StatusEnum analyzeForClause(ParserPtr parser, ASTNodePtr pipeline) {
    // create for node
    ASTNodePtr for_clause = createAndInsertEmpty(pipeline, NODE_FOR_CLAUSE);
    if(for_clause == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // for
    if(!streq(parser->current_token.value, "for")) {
        printError("analyzeForClause", 
                    "Syntax error at line %d: expected `for`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    StatusEnum st = advanceTokens(parser);
    ERR_CHECK(st);

    // name
    if(parser->current_token.type != TOKEN_WORD) {
        printError("analyzeForClause", 
                    "Syntax error at line %d: expected another word after `for`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    ASTNodePtr var = createAndInsertFromToken(for_clause, NODE_WORD, &parser->current_token);
    if(var == NULL) {
        return ERROR_MALLOC_FAILURE;
    }
    st = advanceTokens(parser);
    ERR_CHECK(st);

    // optional in
    if(streq(parser->current_token.value, "in") || parser->current_token.type == TOKEN_NEWLINE) {
        if(parser->current_token.type == TOKEN_NEWLINE) {
            st = analyzeLineBreak(parser);
            ERR_CHECK(st);
        }
        if(!streq(parser->current_token.value, "in")) {
            printError("analyzeForClause", 
                    "Syntax error at line %d: expected 'in' or newline after for variable",
                    parser->lexer->line
                );
            return ERROR_SYNTAX_ERROR;
        }
        // consume 'in'
        st = advanceTokens(parser);
        ERR_CHECK(st);
        // word list optional
        while(parser->current_token.type == TOKEN_WORD) {
            ASTNodePtr word = createAndInsertFromToken(for_clause, NODE_WORD, &parser->current_token);
            if(word == NULL) {
                return ERROR_MALLOC_FAILURE;
            }

            st = advanceTokens(parser);
            ERR_CHECK(st);
        }

        // sequential step mandatory
        if(parser->current_token.type == TOKEN_SEMI) {
            st = advanceTokens(parser);
            ERR_CHECK(st);

            st = analyzeLineBreak(parser);
            ERR_CHECK(st);
        } else {
            st = analyzeNewline(parser);
            ERR_CHECK(st);
        }
    } else {
        // optional sequential step
        if(parser->current_token.type == TOKEN_SEMI) {
            st = advanceTokens(parser);
            ERR_CHECK(st);
        } 
        st = analyzeLineBreak(parser); // here we dont need to have a newline(optional)
        ERR_CHECK(st);
    }

    // do group
    st = analyzeDoGroup(parser, for_clause);
    ERR_CHECK(st);

    return SUCCESS;
}


static StatusEnum analyzeCaseClause(ParserPtr parser, ASTNodePtr pipeline) {
    // create case node
    ASTNodePtr case_clause = createAndInsertEmpty(pipeline, NODE_CASE_CLAUSE);
    if(case_clause == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    if(!streq(parser->current_token.value, "case")) {
        printError("analyzeCaseClause", 
                    "Syntax error at line %d: expected `case`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    StatusEnum st = advanceTokens(parser);
    ERR_CHECK(st);

    if(parser->current_token.type != TOKEN_WORD) {
        printError("analyzeCaseClause", 
                    "Syntax error at line %d: expected another word after `case`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    ASTNodePtr word = createAndInsertFromToken(case_clause, NODE_WORD, &parser->current_token);
    if(word == NULL) {
        return ERROR_MALLOC_FAILURE;
    }
    st = advanceTokens(parser);
    ERR_CHECK(st);

    st = analyzeLineBreak(parser);
    ERR_CHECK(st);

    if(!streq(parser->current_token.value, "in")) {
        printError("analyzeCaseClause", 
                    "syntax error at line %d: expected `in` after `case word`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    st = advanceTokens(parser);
    ERR_CHECK(st);

    st = analyzeLineBreak(parser);
    ERR_CHECK(st);

    while(parser->current_token.type != TOKEN_EOF && 
        !streq(parser->current_token.value, "esac")) 
    {
        st = analyzeCaseItem(parser, case_clause);
        ERR_CHECK(st);

        st = analyzeLineBreak(parser);
        ERR_CHECK(st);
    }

    // expect 'esac'
    if(!streq(parser->current_token.value, "esac")) {
        printError("analyzeCaseClause", 
                    "Syntax error at line %d: expected `easc`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    st = advanceTokens(parser);
    ERR_CHECK(st);

    return SUCCESS;
}


static StatusEnum analyzeCaseItem(ParserPtr parser, ASTNodePtr case_clause) {
    ASTNodePtr item = createAndInsertEmpty(case_clause, NODE_CASE_ITEM);
    if(!item) return ERROR_MALLOC_FAILURE;

    // optional leading (
    int8_t has_lparen = 0U;
    if(parser->current_token.type == TOKEN_LPAREN) {
        has_lparen = 1U;
        StatusEnum st = advanceTokens(parser);
        ERR_CHECK(st);
    }

    // pattern_list     :  WORD    /* Apply rule 4 */
    if(!has_lparen && streq(parser->current_token.value, "esac")) {
        return SUCCESS;
    }

    // first pattern
    if(parser->current_token.type != TOKEN_WORD) {
        printError("analyzeCaseItem", 
                    "Syntax error at line %d: expected case item pattern",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }

    ASTNodePtr pattern = createAndInsertFromToken(item, NODE_WORD, &parser->current_token);
    if(pattern == NULL) {
        return ERROR_MALLOC_FAILURE;
    }
    StatusEnum st = advanceTokens(parser);
    ERR_CHECK(st);

    // more patterns separated by |
    while(parser->current_token.type == TOKEN_PIPE) {
        st = advanceTokens(parser);  // consume |
        ERR_CHECK(st);

        if(parser->current_token.type != TOKEN_WORD) {
            printError("analyzeCaseItem", 
                    "Syntax error at line %d: expected case item pattern after `|`",
                    parser->lexer->line
                );
            return ERROR_SYNTAX_ERROR;
        }
        ASTNodePtr p = createAndInsertFromToken(item, NODE_WORD, &parser->current_token);
        if(p == NULL) {
            return ERROR_MALLOC_FAILURE;
        }

        st = advanceTokens(parser);
        ERR_CHECK(st);
    }

    // expect )
    if(parser->current_token.type != TOKEN_RPAREN) {
        printError("analyzeCaseItem", 
                    "Syntax error at line %d: expected ')'",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    st = advanceTokens(parser);
    ERR_CHECK(st);

    // optional body, after it should be either ;; or ;& or esac
    if(parser->current_token.type != TOKEN_DOUBLE_SEMI &&
       parser->current_token.type != TOKEN_SEMI_AND &&
       !streq(parser->current_token.value, "esac")) 
    {
        st = analyzeCompoundList(parser, item);
        ERR_CHECK(st);
    }

    // ;; or ;& or nothing (last item)
    if(parser->current_token.type == TOKEN_DOUBLE_SEMI) {
        item->flags = FLAG_DOUBLE_SEMI;

        st = advanceTokens(parser);
        ERR_CHECK(st);

        st = analyzeLineBreak(parser);
        ERR_CHECK(st);
    } else if(parser->current_token.type == TOKEN_SEMI_AND) {
        item->flags = FLAG_SEMI_AND;

        st = advanceTokens(parser);
        ERR_CHECK(st);

        st = analyzeLineBreak(parser);
        ERR_CHECK(st);
    }
    // else — case_item_ns, no flag needed
    return SUCCESS;
}


static StatusEnum analyzeFunctionDef(ParserPtr parser, ASTNodePtr pipeline) {
    // create function node
    ASTNodePtr function = createAndInsertEmpty(pipeline, NODE_FUNCTION_DEF);
    if(function == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    // name
    if(parser->current_token.type != TOKEN_WORD) {
        printError("analyzeFunctionDef", 
                    "Syntax error at line %d: expected `function name`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }
    ASTNodePtr name = createAndInsertFromToken(function, NODE_WORD, &parser->current_token);
    if(name == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    StatusEnum st = advanceTokens(parser);
    ERR_CHECK(st);

    // expect LPAREN
    if(parser->current_token.type != TOKEN_LPAREN) {
        printError("analyzeFunctionDef", 
                    "Syntax error at line %d: expected `(`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }

    st = advanceTokens(parser);
    ERR_CHECK(st);

    // expect RPAREN
    if(parser->current_token.type != TOKEN_RPAREN) {
        printError("analyzeFunctionDef", 
                    "Syntax error at line %d: expected `)`",
                    parser->lexer->line
                );
        return ERROR_SYNTAX_ERROR;
    }

    st = advanceTokens(parser);
    ERR_CHECK(st);

    st = analyzeLineBreak(parser);
    ERR_CHECK(st);

    // parse compound command as body
    st = analyzeCompoundCommand(parser, function);
    ERR_CHECK(st);

    // optional redirect list
    while(isRedirectOperator(parser->current_token.type) ||
          parser->current_token.type == TOKEN_IO_NUM) {
        st = analyzeRedirect(parser, function);
        ERR_CHECK(st);
    }

    return SUCCESS;
}

static StatusEnum analyzeCompoundCommand(ParserPtr parser, ASTNodePtr pipeline) {
    if(parser->current_token.type == TOKEN_LBRACE)
        return analyzeBraceGroup(parser, pipeline);
    if(parser->current_token.type == TOKEN_LPAREN)
        return analyzeSubshell(parser, pipeline);
    if(streq(parser->current_token.value, "if"))
        return analyzeIfClause(parser, pipeline);
    if(streq(parser->current_token.value, "while"))
        return analyzeWhileClause(parser, pipeline);
    if(streq(parser->current_token.value, "until"))
        return analyzeUntilClause(parser, pipeline);
    if(streq(parser->current_token.value, "for"))
        return analyzeForClause(parser, pipeline);
    if(streq(parser->current_token.value, "case"))
        return analyzeCaseClause(parser, pipeline);

    printError("analyzeCompoundCommand", 
                "Syntax error at line %d: expected compound command",
                parser->lexer->line
            );
    return ERROR_SYNTAX_ERROR;
}


StatusEnum parserCtor(ParserPtr parser, LexerPtr lexer) {
    if(lexer == NULL || parser == NULL) {
        printError("parserCtor", "Passing NULL pointer");
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
        printError("parserReset", "Passing NULL pointer");
        return;
    }

    tokenFree(&parser->current_token);
    tokenFree(&parser->peek_token);
    parser->current_token = getToken(parser->lexer);
    parser->peek_token = getToken(parser->lexer);
}

StatusEnum analyze(ParserPtr parser, ASTNodePtr ast_root) {
    if(parser == NULL || parser->lexer == NULL || ast_root == NULL) {
        printError("analyze", "Passing NULL pointer");
        return ERROR_SYNTAX_ERROR;
    }

    StatusEnum st = analyzeProgram(parser, ast_root);
    if(st != SUCCESS) {
        ASTFreeTree(ast_root);
        return st;
    }
    return SUCCESS;
}