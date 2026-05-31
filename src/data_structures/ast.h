#include <stdint.h>
#include "error.h"

typedef enum {
    NODE_PROGRAM,


} NodeTypeEnum;

typedef struct ASTNode {
    NodeTypeEnum type;
    char* value;
    struct ASTNode* parent;
    struct ASTNode** children;
    int16_t num_children;
} ASTNode, *ASTNodePtr;


ASTNodePtr astNodeCtor(NodeTypeEnum type, char* value);

void astNodeDtor(ASTNodePtr node);

StatusEnum addChild(ASTNodePtr parent, ASTNodePtr child);

void freeTree(ASTNodePtr node);