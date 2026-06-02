#include "data_structures/ast.h"


ASTNodePtr ASTNodeCtor(NodeTypeEnum type, char* value) {
    ASTNodePtr node = (ASTNodePtr) malloc(sizeof(ASTNode));
    if(node == NULL) {
        fprintf(stderr, "CyprSH: ASTNodeCtor: malloc failure\n");
        return NULL;
    }
    node->type = type;
    node->value = value ? strdup(value) : NULL;
    node->children = NULL;
    node->num_children = 0;
    node->flags = 0;
    return node;
} // ASTNodeCtor


void ASTNodeDtor(ASTNodePtr node) {
    if(node == NULL) {
        return;
    }

    free(node->value);
    free(node);
} // ASTNodeDtor


StatusEnum ASTaddChild(ASTNodePtr parent, ASTNodePtr child) {
    if(parent == NULL || child == NULL) {
        fprintf(stderr, "CyprSH: adding NULL pointer child in syntax analysis\n");
        return ERROR_DEFAULT;
    }

    ASTNodePtr* new_children = realloc(parent->children, sizeof(ASTNodePtr) * (parent->num_children + 1));
    if(new_children == NULL) {
        fprintf(stderr, "CyprSH: AST tree realloc failure\n");
        return ERROR_MALLOC_FAILURE;
    }
    parent->children = new_children;
    parent->children[parent->num_children++] = child;
    child->parent = parent;
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