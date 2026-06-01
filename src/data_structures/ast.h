#include <stdint.h>
#include "error.h"

typedef enum {
    NODE_PROGRAM,
    NODE_COMPLETE_COMMAND,
    NODE_LIST,
    NODE_AND_OR,

} NodeTypeEnum;

typedef enum {
    FLAG_NONE = 0,
    FLAG_SEMICOLON = 1,
    FLAG_BACKGROUND = 2,
    FLAG_OR = 4,
    FLAG_AND = 8,
    
} TokenTypeEnum;

typedef struct ASTNode {
    NodeTypeEnum type;
    char* value;
    int8_t flags;
    struct ASTNode* parent;
    struct ASTNode** children;
    int16_t num_children;
} ASTNode, *ASTNodePtr, **ASTNodePtrPtr;


ASTNodePtr ASTNodeCtor(NodeTypeEnum type, char* value);

void ASTNodeDtor(ASTNodePtr node);

StatusEnum ASTaddChild(ASTNodePtr parent, ASTNodePtr child);

void ASTFreeTree(ASTNodePtr node);