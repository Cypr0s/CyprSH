#include "data_structures/ast.h"


StatusEnum astNodeCtor(NodeTypeEnum type, char* value) {
    ASTNodePtr node = (ASTNodePtr) malloc(sizeof(ASTNode));
    if(node == NULL) {
        fprintf(stderr, "CyprSH: astNodeCtor: malloc failure\n");
        return ERROR_MALLOC_FAILURE;
    }
    node->type = type;
    node->value = strdup(value);
    node->children = NULL;
    node->num_children = 0;
    return SUCCESS;
} // astNodeCtor


void astNodeDtor(ASTNodePtr node) {
    if(node == NULL) {
        return;
    }

    free(node->value);
    free(node);
} // astNodeDtor


StatusEnum addChild(ASTNodePtr parent, ASTNodePtr child) {
    if(parent == NULL || child == NULL) {
        fprintf(stderr, "CyprSH: adding NULL pointer child in syntax analysis\n");
        return ERROR_DEFAULT;
    }

    ASTNodePtr* new_children = realloc(parent->children, sizeof(ASTNodePtr) * (parent->num_children + 1));
    if(new_children == NULL) {
        fprintf(stderr, "CyprSH: addChild: realloc failure\n");
        return ERROR_MALLOC_FAILURE;
    }
    parent->children = new_children;
    parent->children[parent->num_children++] = child;
    child->parent = parent;
    return SUCCESS;
} // addChild


void freeTree(ASTNodePtr node) {
    if(node == NULL) {
        return;
    }

    for(int16_t i = 0; i < node->num_children; i++) {
        freeTree(node->children[i]);
    }
    astNodeDtor(node);
} // freeTree