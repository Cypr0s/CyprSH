#include "data_structures/ast.h"


ASTNodePtr ASTNodeCtor(NodeTypeEnum type, char* value, int8_t* value_types) {
    ASTNodePtr node = (ASTNodePtr) malloc(sizeof(ASTNode));
    if(node == NULL) {
        fprintf(stderr, "CyprSH: ASTNodeCtor: malloc failure\n");
        return NULL;
    }
    // defaults
    node->type = type;
    node->value = NULL;
    node->value_types = NULL;
    node->children = NULL;
    node->num_children = 0;
    node->flags = 0;
    // allocate, store value and value types
    if(value != NULL) {
        node->value = strdup(value);
        if(node->value == NULL) {
            free(node);
            fprintf(stderr, "CyprSH: ASTNodeCtor: malloc failure\n");
            return NULL;
        }

        size_t len = strlen(value);
        if(value_types != NULL && len > 0) {
            node->value_types = malloc(len);
            if(node->value_types == NULL) {
                free(node->value);
                free(node);
                fprintf(stderr, "CyprSH: ASTNodeCtor: malloc failure\n");
                return NULL;
            }
            memcpy(node->value_types, value_types, len);
        }
    }

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