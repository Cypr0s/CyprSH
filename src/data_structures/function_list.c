/**
 * @file        function_list.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Function list implementation
 */

#include "data_structures/function_list.h"

/** @brief Initialize empty function list
 *  @param fl Function list to initialize
 */
void functionListCtor(FunctionListPtr fl) {
    fl->head = NULL;
}

/** @brief Destroy function list and free all entries
 *  @param fl List to destroy
 */
void functionListDtor(FunctionListPtr fl) {
    if(fl == NULL) return;

    FunctionEntryPtr current = fl->head;
    while(current != NULL) {
        FunctionEntryPtr next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    fl->head = NULL;
}

/** @brief Insert or update function definition
 *  @param fl Target list
 *  @param name Function name
 *  @param body AST node containing function body
 *  @return Status code
 */
StatusEnum functionListInsert(FunctionListPtr fl, const char* name, ASTNodePtr body) {
    if(fl == NULL || name == NULL) {
        printError("functionListInsert", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    FunctionEntryPtr current = fl->head;
    while(current != NULL) {
        if(streq(current->name, name)) {
            current->body = body;
            return SUCCESS;
        }
        current = current->next;
    }

    FunctionEntryPtr entry = malloc(sizeof(FunctionEntry));
    if(entry == NULL) {
        printError("functionListInsert", "Malloc failure");
        return ERROR_MALLOC_FAILURE;
    }

    entry->name = strdup(name);
    if(entry->name == NULL) {
        free(entry);
        printError("functionListInsert", "Malloc failure");
        return ERROR_MALLOC_FAILURE;
    }

    entry->body = body;
    entry->next = fl->head;
    fl->head = entry;

    return SUCCESS;
}

/** @brief Find function by name
 *  @param fl Source list
 *  @param name Function name to search for
 *  @return AST node of function body or NULL if not found
 */
ASTNodePtr functionListFind(FunctionListPtr fl, const char* name) {
    if(fl == NULL || name == NULL) {
        return NULL;
    }

    FunctionEntryPtr current = fl->head;
    while(current != NULL) {
        if(streq(current->name, name)) {
            return current->body;
        }
        current = current->next;
    }
    return NULL;
}