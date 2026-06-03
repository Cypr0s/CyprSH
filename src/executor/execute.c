#include "executor/execute.h"

static StatusEnum executeProgram(ASTNodePtr program_node, ExecuteEnvironmentPtr env);
static StatusEnum executeCompleteCommandNode(ASTNodePtr command_node, ExecuteEnvironmentPtr env);
static StatusEnum executeListNode(ASTNodePtr list_node, ExecuteEnvironmentPtr env);
static StatusEnum executeAndOrNode(ASTNodePtr and_or_node, ExecuteEnvironmentPtr env);
static StatusEnum executePipelineNode(ASTNodePtr pipeline_node, ExecuteEnvironmentPtr env);
static StatusEnum executeSimpleCommandNode(ASTNodePtr command_node, ExecuteEnvironmentPtr env);


StatusEnum executeNode(ASTNodePtr node, ExecuteEnvironmentPtr env) {
    if(node == NULL) {
        return SUCCESS;
    }
    switch(node->type) {
        case NODE_PROGRAM: return executeProgramNode(node, env);
        case NODE_COMPLETE_COMMAND: return executeCompleteCommandNode(node, env);
        case NODE_LIST: return executeListNode(node, env);
        case NODE_AND_OR: return executeAndOrNode(node, env);
        case NODE_PIPELINE: return executePipelineNode(node, env);
        case NODE_SIMPLE_COMMAND: return executeSimpleCommandNode(node, env);
        case NODE_CMD_PREFIX:
        case NODE_CMD_WORD:
        case NODE_CMD_SUFFIX:
        case NODE_REDIRECT:
    }
}


static StatusEnum executeProgramNode(ASTNodePtr program_node, ExecuteEnvironmentPtr env) {
    StatusEnum st = SUCCESS;
    for(int16_t i = 0; i < program_node->num_children; i++) {
        st = executeNode(program_node->children[i], env);
        ERR_CHECK(st);
    }
    return st;
}


static StatusEnum executeCompleteCommandNode(ASTNodePtr command_node, ExecuteEnvironmentPtr env) {
    if(command_node->num_children == 0) {
        return ERROR_SYNTAX_ERROR;
    }
    return executeNode(command_node->children[0], env);
}


static StatusEnum executeListNode(ASTNodePtr list_node, ExecuteEnvironmentPtr env) {
     if(list_node->num_children == 0) {
        return ERROR_SYNTAX_ERROR;
    }

    StatusEnum st = SUCCESS;

    for(int16_t i = 0; i < list_node->num_children; i++) {
        ASTNodePtr child = list_node->children[i];

        // child flag, run whole on background
        if(child->flags & FLAG_BACKGROUND) {
            env->flags |= EXEC_FLAG_BG;
        }

        st = executeNode(child, env);
        ERR_CHECK(st);
        env->flags &= ~EXEC_FLAG_BG; // reset flag
    }

    return st;
}

static StatusEnum executeAndOrNode(ASTNodePtr and_or_node, ExecuteEnvironmentPtr env) {
    if(and_or_node->num_children == 0) {
        return ERROR_SYNTAX_ERROR;
    }

    // run first pipeline
    StatusEnum st = executeNode(and_or_node->children[0], env);
    ERR_CHECK(st);

    for(int16_t i = 1; i < and_or_node->num_children; i++) {
        ASTNodePtr prev = and_or_node->children[i - 1];

        if(prev->flags & FLAG_AND && env->last_exec_status != 0) {
            continue;
        } else if(prev->flags & FLAG_OR && env->last_exec_status == 0) {
            continue;
        }

        st = executeNode(and_or_node->children[i], env);
        ERR_CHECK(st);
    }

    return st;
}


static StatusEnum executePipelineNode(ASTNodePtr pipeline_node, ExecuteEnvironmentPtr env) {
    if(pipeline_node->num_children == 0) {
        return ERROR_SYNTAX_ERROR;
    }

    // single command
    StatusEnum st = SUCCESS;
    if(pipeline_node->num_children == 1) {
        st = executeNode(pipeline_node->children[0], env);
        ERR_CHECK(st);

        if(pipeline_node->flags & FLAG_BANG) {
            env->last_exec_status ^= 1U;
        }
        return SUCCESS;
    }

    // pipes... many hours wasted here trying to understand(so unpractical with the pipe() func)
    pid_t* pids = malloc(sizeof(pid_t) * pipeline_node->num_children);
    int32_t prev_stdin = -1;
    if(pids == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    for(int16_t i = 0; i < pipeline_node->num_children; i++) {
        // setup pipes
        int32_t file_descriptors[2] = {-1, -1}; // default

        if(i < pipeline_node->num_children - 1) {
            if(pipe(file_descriptors) == -1) {
                fprintf(stderr, "CyprSH: Failed to create Pipe file descriptors\n");
                free(pids);
                return ERROR_DEFAULT;
            }
        }

        // create new process
        pid_t p = fork();
        if(p == -1) {
            fprintf(stderr, "CyprSH: Failed to create new fork process\n");
            free(pids);
            return ERROR_DEFAULT;
        }

        // connect file descriptors
        if(p == 0) {
            if(prev_stdin != -1) { // not the first process, set 
                dup2(prev_stdin, STDIN_FILENO);
                close(prev_stdin);
            }

            if(i < pipeline_node->num_children - 1) {
                dup2(file_descriptors[1], STDOUT_FILENO);
                close(file_descriptors[1]);
                close(file_descriptors[0]);
            }

            env->flags |= EXEC_FLAG_CHILD_PROCESS;
            executeNode(pipeline_node->children[i], env);
            exit(env->last_exec_status);
        } else {
            pids[i] = p;

            if(prev_stdin != -1) {
                close(prev_stdin);
            }

            if(i < pipeline_node->num_children - 1) {
                close(file_descriptors[1]);
                prev_stdin = file_descriptors[0];
            }
        }
    }

    int32_t status = 0;
    for(int16_t i = 0; i < pipeline_node->num_children; i++) {
        waitpid(pids[i], &status, 0);
    }
    env->last_exec_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;

    if(pipeline_node->flags & FLAG_BANG) {
        env->last_exec_status ^= 1U;
    }

    free(pids);
    return SUCCESS;
}

static executeSimpleCommandNode(ASTNodePtr command_node, ExecuteEnvironmentPtr env) {
    // todo
    return SUCCESS;
}