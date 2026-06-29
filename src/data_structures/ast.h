/**
 * @file        ast.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Abstract Syntax Tree node definitions and operations
 */

#ifndef AST_H
#define AST_H

#include <stdint.h>
#include "error.h"
#include "utils/strings.h"

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
    FLAG_NONE           = 0,
    FLAG_BACKGROUND     = 1,
    FLAG_OR             = 2,
    FLAG_AND            = 4,
    FLAG_BANG           = 8,
    FLAG_DOUBLE_SEMI    = 16,
    FLAG_SEMI_AND       = 32,
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
    int8_t* value_types;
    int32_t flags;
    struct ASTNode** children;
    int16_t num_children;
} ASTNode, *ASTNodePtr, **ASTNodePtrPtr;

/** @brief Create new AST node
 *  @param type Node type enum
 *  @param value Node value string
 *  @param value_types Array of character type flags
 *  @return Pointer to new AST node or NULL on failure
 */
ASTNodePtr ASTNodeCtor(NodeTypeEnum type, char* value, int8_t* value_types);

/** @brief Destroy single AST node
 *  @param node Node to destroy
 */
void ASTNodeDtor(ASTNodePtr node);

/** @brief Add child node to parent
 *  @param parent Parent node
 *  @param child Child node to add
 *  @return Status code
 */
StatusEnum ASTaddChild(ASTNodePtr parent, ASTNodePtr child);

/** @brief Recursively free AST tree
 *  @param node Root node to free
 */
void ASTFreeTree(ASTNodePtr node);

#endif // AST_H