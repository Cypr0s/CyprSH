#include "data_structures/ast.h"


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


void ASTNodeDtor(ASTNodePtr node) {
    if(node == NULL) {
        return;
    }

    free(node->value_types);
    free(node->value);
    free(node);
} // ASTNodeDtor


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