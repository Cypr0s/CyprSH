/**
 * @file        function_list.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Function definition list (linked list)
 */

#ifndef BUILTIN_COMMAND_LIST_H
#define BUILTIN_COMMAND_LIST_H

#include "error.h"
#include "data-structures/abstract-syntax-tree.h"

typedef struct BuiltinCommandEntry {
    char* name;
    ASTNodePtr body;
    struct BuiltinCommandEntry* next;
} BuiltinCommandEntry, *BuiltinCommandEntryPtr;

typedef struct {
    BuiltinCommandEntryPtr head;
} BuiltinCommandList, *BuiltinCommandListPtr;

/** @brief Initialize empty builtin command list
 *  @param bcl Builtin command list to initialize
 */
void builtinCommandListCtor(BuiltinCommandListPtr bcl);

/** @brief Destroy builtin command list and free all entries
 *  @param bcl List to destroy
 */
void builtinCommandListDtor(BuiltinCommandListPtr bcl);

/** @brief Insert or update builtin command definition
 *  @param bcl Target list
 *  @param name Builtin command name
 *  @param body AST node containing command body
 *  @return Status code
 */
StatusEnum builtinCommandListInsert(BuiltinCommandListPtr bcl, const char* name, ASTNodePtr body);

/** @brief Find builtin command by name
 *  @param bcl Source list
 *  @param name Builtin command name to search for
 *  @return AST node of command body or NULL if not found
 */
ASTNodePtr builtinCommandListFind(BuiltinCommandListPtr bcl, const char* name);

#endif