#ifndef AST_H
#define AST_H

#include <stdint.h>
#include "error.h"

typedef enum {
    NODE_PROGRAM,
    NODE_COMPLETE_COMMAND,
    NODE_LIST,
    NODE_AND_OR,
    NODE_PIPELINE,
    NODE_SIMPLE_COMMAND,
    NODE_CMD_PREFIX,
    NODE_CMD_WORD,
    NODE_CMD_SUFFIX,
    NODE_REDIRECT,

    NODE_ASSIGNMENT_WORD,
    NODE_WORD,
    NODE_IO_NUM,
    
    NODE_SUBSHELL,
    NODE_BRACE_GROUP,
    NODE_IF_CLAUSE,
    NODE_ELSE_CLAUSE,
    NODE_WHILE_CLAUSE,
    NODE_UNTIL_CLAUSE,
    NODE_FOR_CLAUSE,
    NODE_CASE_CLAUSE,
    NODE_CASE_ITEM,
    NODE_FUNCTION_DEF
} NodeTypeEnum;

typedef enum {
    FLAG_NONE = 0,
    FLAG_BACKGROUND = 1,
    FLAG_OR = 2,
    FLAG_AND = 4,
    FLAG_PIPE = 8,
    FLAG_BANG = 16,
    FLAG_DOUBLE_SEMI = 32,
    FLAG_SEMI_AND = 64,
} NodeFlagEnum;

typedef enum {
    REDIR_NONE      = 0,
    REDIR_LESS      = 1,
    REDIR_GREAT     = 2,
    REDIR_DLESS     = 3,
    REDIR_DGREAT    = 4,
    REDIR_LESSAND   = 5,
    REDIR_GREATAND  = 6,
    REDIR_LESSGREAT = 7,
    REDIR_CLOBBER   = 8,
    REDIR_DLESSDASH = 9,
    REDIR_TLESS     = 10,
} RedirectTypeEnum;


typedef struct ASTNode {
    NodeTypeEnum type;
    char* value;
    int32_t flags;
    struct ASTNode* parent;
    struct ASTNode** children;
    int16_t num_children;
} ASTNode, *ASTNodePtr, **ASTNodePtrPtr;


ASTNodePtr ASTNodeCtor(NodeTypeEnum type, char* value);

void ASTNodeDtor(ASTNodePtr node);

StatusEnum ASTaddChild(ASTNodePtr parent, ASTNodePtr child);

void ASTFreeTree(ASTNodePtr node);

#endif // AST_H