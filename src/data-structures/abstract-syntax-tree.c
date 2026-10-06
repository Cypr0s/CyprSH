/**
 * @file        ast.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Abstract Syntax Tree implementation
 */

#include "data-structures/abstract-syntax-tree.h"

/** @brief Create new AST node
 *  @param type Node type enum
 *  @param value Node value string
 *  @param value_types Array of character type flags
 *  @return Pointer to new AST node or NULL on failure
 */
ASTNodePtr ASTNodeCtor(NodeTypeEnum type, char* value, int8_t* value_types) {
    ASTNodePtr node = (ASTNodePtr) malloc(sizeof(ASTNode));
    if(node == NULL) {
        printError("ASTNodeCtor", "Malloc failure");
        return NULL;
    }
    node->type = type;
    node->value = value;
    node->value_types = value_types;
    node->children = NULL;
    node->num_children = 0;
    node->flags = 0;
    return node;
} // ASTNodeCtor

/** @brief Destroy single AST node
 *  @param node Node to destroy
 */
void ASTNodeDtor(ASTNodePtr node) {
    if(node == NULL) {
        return;
    }

    free(node->value_types);
    free(node->value);
    free(node);
} // ASTNodeDtor

/** @brief Add child node to parent
 *  @param parent Parent node
 *  @param child Child node to add
 *  @return Status code
 */
StatusEnum ASTaddChild(ASTNodePtr parent, ASTNodePtr child) {
    if(parent == NULL || child == NULL) {
        printError("ASTaddChild", "Trying to insert `NULL` in AST");
        return ERROR_DEFAULT;
    }

    ASTNodePtr* new_children = realloc(parent->children, sizeof(ASTNodePtr) * (parent->num_children + 1));
    if(new_children == NULL) {
        printError("ASTaddChild", "Realloc failure");
        return ERROR_MALLOC_FAILURE;
    }
    parent->children = new_children;
    parent->children[parent->num_children++] = child;
    return SUCCESS;
} // ASTaddChild

/** @brief Recursively free AST tree
 *  @param node Root node to free
 */
void ASTFreeTree(ASTNodePtr node) {
    if(node == NULL) {
        return;
    }

    for(int16_t i = 0; i < node->num_children; i++) {
        ASTFreeTree(node->children[i]);
    }
    free(node->children);
    ASTNodeDtor(node);
} // ASTFreeTree


ASTNodePtr ASTCopyTree(ASTNodePtr node) {
    if(node == NULL) {
        return NULL;
    }

    char* value = node->value == NULL ? NULL : strdup(node->value);
    if(node->value != NULL && value == NULL) {
        return NULL;
    }

    int8_t* value_types = NULL;
    if(node->value_types != NULL) {
        size_t value_length = strlen(node->value);
        value_types = malloc(sizeof(int8_t) * value_length);
        if(value_types == NULL) {
            free(value);
            return NULL;
        }
        memcpy(value_types, node->value_types, sizeof(int8_t) * value_length);
    }

    ASTNodePtr copy = ASTNodeCtor(node->type, value, value_types);
    if(copy == NULL) {
        free(value);
        free(value_types);
        return NULL;
    }

    copy->flags = node->flags;

    for(int16_t i = 0; i < node->num_children; i++) {
        ASTNodePtr child = ASTCopyTree(node->children[i]);
        if(child == NULL || ASTaddChild(copy, child) != SUCCESS) {
            ASTFreeTree(child);
            ASTFreeTree(copy);
            return NULL;
        }
    }

    return copy;
}