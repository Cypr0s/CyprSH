/**
 * @file        function_list.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Function definition list (linked list)
 */

#ifndef FUNCTION_LIST_H
#define FUNCTION_LIST_H

#include "error.h"
#include "data_structures/ast.h"

typedef struct FunctionEntry {
    char* name;
    ASTNodePtr body;
    struct FunctionEntry* next;
} FunctionEntry, *FunctionEntryPtr;

typedef struct {
    FunctionEntryPtr head;
} FunctionList, *FunctionListPtr;

/** @brief Initialize empty function list
 *  @param fl Function list to initialize
 */
void functionListCtor(FunctionListPtr fl);

/** @brief Destroy function list and free all entries
 *  @param fl List to destroy
 */
void functionListDtor(FunctionListPtr fl);

/** @brief Insert or update function definition
 *  @param fl Target list
 *  @param name Function name
 *  @param body AST node containing function body
 *  @return Status code
 */
StatusEnum functionListInsert(FunctionListPtr fl, const char* name, ASTNodePtr body);

/** @brief Find function by name
 *  @param fl Source list
 *  @param name Function name to search for
 *  @return AST node of function body or NULL if not found
 */
ASTNodePtr functionListFind(FunctionListPtr fl, const char* name);

#endif