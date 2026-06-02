#include "executor/execute.h"

static StatusEnum executeProgram(ASTNodePtr program_node);
static StatusEnum executeCompleteCommandNode(ASTNodePtr command_node);
static StatusEnum executeListNode(ASTNodePtr list_node);

StatusEnum executeNode(ASTNodePtr node) {
    if(node == NULL) {
        return SUCCESS;
    }
    switch(node->type) {
        case NODE_PROGRAM: return executeProgram(node);
        case NODE_COMPLETE_COMMAND: return executeCompleteCommandNode(node);
        case NODE_LIST: return executeListNode(node);
        case NODE_AND_OR:
        case NODE_PIPELINE:
        case NODE_SIMPLE_COMMAND:
        case NODE_CMD_PREFIX:
        case NODE_CMD_WORD:
        case NODE_CMD_SUFFIX:
        case NODE_REDIRECT:
    }
}

static StatusEnum executeProgramNode(ASTNodePtr program_node) {
    StatusEnum st = SUCCESS;
    for(int16_t i = 0; i < program_node->num_children; i++) {
        st = executeNode(program_node->children[i]);
        ERR_CHECK(st);
    }
    return st;
}

static StatusEnum executeCompleteCommandNode(ASTNodePtr command_node) {
    if(command_node->num_children == 0) {
        return ERROR_SYNTAX_ERROR;
    }
    return executeListNode(command_node->children[0]);
}

static StatusEnum executeListNode(ASTNodePtr list_node) {

}