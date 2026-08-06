/**
 * @file        builtin-command-linked-list.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Builtin command list implementation
 */

#include "data-structures/builtin-command-linked-list.h"

/** @brief Initialize empty builtin command list
 *  @param bcl Builtin command list to initialize
 */
void builtinCommandListCtor(BuiltinCommandListPtr bcl) {
    bcl->head = NULL;
}

/** @brief Destroy builtin command list and free all entries
 *  @param bcl List to destroy
 */
void builtinCommandListDtor(BuiltinCommandListPtr bcl) {
    if(bcl == NULL) return;

    BuiltinCommandEntryPtr current = bcl->head;
    while(current != NULL) {
        BuiltinCommandEntryPtr next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    bcl->head = NULL;
}

/** @brief Insert or update builtin command definition
 *  @param bcl Target list
 *  @param name Builtin command name
 *  @param body AST node containing command body
 *  @return Status code
 */
StatusEnum builtinCommandListInsert(BuiltinCommandListPtr bcl, const char* name, ASTNodePtr body) {
    if(bcl == NULL || name == NULL) {
        printError("builtinCommandListInsert", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    BuiltinCommandEntryPtr current = bcl->head;
    while(current != NULL) {
        if(streq(current->name, name)) {
            current->body = body;
            return SUCCESS;
        }
        current = current->next;
    }

    BuiltinCommandEntryPtr entry = malloc(sizeof(BuiltinCommandEntry));
    if(entry == NULL) {
        printError("builtinCommandListInsert", "Malloc failure");
        return ERROR_MALLOC_FAILURE;
    }

    entry->name = strdup(name);
    if(entry->name == NULL) {
        free(entry);
        printError("builtinCommandListInsert", "Malloc failure");
        return ERROR_MALLOC_FAILURE;
    }

    entry->body = body;
    entry->next = bcl->head;
    bcl->head = entry;

    return SUCCESS;
}

/** @brief Find builtin command by name
 *  @param bcl Source list
 *  @param name Builtin command name to search for
 *  @return AST node of command body or NULL if not found
 */
ASTNodePtr builtinCommandListFind(BuiltinCommandListPtr bcl, const char* name) {
    if(bcl == NULL || name == NULL) {
        return NULL;
    }

    BuiltinCommandEntryPtr current = bcl->head;
    while(current != NULL) {
        if(streq(current->name, name)) {
            return current->body;
        }
        current = current->next;
    }
    return NULL;
}