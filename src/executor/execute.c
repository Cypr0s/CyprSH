#include "executor/execute.h"

static StatusEnum handlePrefix(ASTNodePtr command_prefix, ExecuteEnvironmentPtr env);
static StatusEnum handleSuffix(ASTNodePtr command_suffix, char** argv, int16_t* argc);
static StatusEnum handleRedirect(ASTNodePtr redirect_node);
static void getDefaultFD(RedirectTypeEnum type, int32_t* fd);
static int8_t isBuiltin(const char* function_name);
static char** buildEnvp(HashTablePtr env);
static char* findPath(const char* cmd, const char* path_env);

static StatusEnum executeProgramNode(ASTNodePtr program_node, ExecuteEnvironmentPtr env);
static StatusEnum executeCompleteCommandNode(ASTNodePtr command_node, ExecuteEnvironmentPtr env);
static StatusEnum executeListNode(ASTNodePtr list_node, ExecuteEnvironmentPtr env);
static StatusEnum executeAndOrNode(ASTNodePtr and_or_node, ExecuteEnvironmentPtr env);
static StatusEnum executePipelineNode(ASTNodePtr pipeline_node, ExecuteEnvironmentPtr env);
static StatusEnum executeSimpleCommandNode(ASTNodePtr command_node, ExecuteEnvironmentPtr env);


static int8_t isBuiltin(const char* function_name) {
    (void)function_name;
    return 0;  // no builtins yet
}


static StatusEnum handlePrefix(ASTNodePtr command_prefix, ExecuteEnvironmentPtr env) {
    char* key;
    char* value;
    StatusEnum st = SUCCESS;
    for(int16_t i = 0; i < command_prefix->num_children; i++) {
        if(command_prefix->children[i]->type == NODE_ASSIGNMENT_WORD) {
            st = splitAssignment(command_prefix->children[i]->value, &key, &value);
            ERR_CHECK(st);

            st = hashTableInsert(env->env_table, key, value);
            free(key);
            free(value);
            ERR_CHECK(st);

        } else if(command_prefix->children[i]->type == NODE_REDIRECT) {
            st = handleRedirect(command_prefix->children[i]);
            ERR_CHECK(st);
        } else {
            return ERROR_SYNTAX_ERROR;
        }
    }
    return st;
}


static StatusEnum handleSuffix(ASTNodePtr command_suffix, char** argv, int16_t* argc) {
    StatusEnum st = SUCCESS;
    for(int16_t i = 0; i < command_suffix->num_children; i++) {
        if(command_suffix->children[i]->type == NODE_WORD) {
            argv[(*argc)++] = command_suffix->children[i]->value;
        } else if(command_suffix->children[i]->type == NODE_REDIRECT) {
            st = handleRedirect(command_suffix->children[i]);
            ERR_CHECK(st);
        } else {
            return ERROR_SYNTAX_ERROR;
        }
    }
    return SUCCESS;
}


static StatusEnum handleRedirect(ASTNodePtr redirect_node) {
    int32_t source_fd = -1;
    char* target_path = NULL;
    int32_t target_fd = -1;
    StatusEnum st = SUCCESS;

    for(int16_t i = 0; i < redirect_node->num_children; i++) {
        if(redirect_node->children[i]->type == NODE_IO_NUM) {
            char* endptr;
            long val = strtol(redirect_node->children[i]->value, &endptr, 10);
            if(*endptr != '\0' || val < 0 || val > INT32_MAX) {
                fprintf(stderr, "CyprSH: invalid fd '%s'\n", redirect_node->children[i]->value);
                return ERROR_SYNTAX_ERROR;
            }
            source_fd = (int32_t)val;
        } else if(redirect_node->children[i]->type == NODE_WORD) {
            target_path = redirect_node->children[i]->value;
        } else {
            return ERROR_SYNTAX_ERROR;
        }
    }

    if(target_path == NULL) {
        return ERROR_SYNTAX_ERROR;
    }

    getDefaultFD(redirect_node->flags, &source_fd);

    switch(redirect_node->flags) {
        case REDIR_LESS: // <
            st = openFile(target_path, O_RDONLY, &target_fd);
            break;
        case REDIR_GREAT: // >
        case REDIR_CLOBBER: // >|
            st = openFile(target_path, O_WRONLY | O_CREAT | O_TRUNC, &target_fd);
            break;
        case REDIR_DGREAT: // >>
            st = openFile(target_path, O_WRONLY | O_CREAT | O_APPEND, &target_fd);
            break;
        case REDIR_LESSGREAT: // <>
            st = openFile(target_path, O_RDWR | O_CREAT, &target_fd);
            break;
        case REDIR_GREATAND: // >&
        case REDIR_LESSAND: {  // <&
            if(streq(target_path, "-")) {
                close(source_fd);
                return SUCCESS;
            }
            
            char* endptr;
            long val = strtol(target_path, &endptr, 10);
            if(*endptr != '\0' || val < 0 || val > INT32_MAX) {
                fprintf(stderr, "CyprSH: invalid fd '%s'\n", target_path);
                return ERROR_SYNTAX_ERROR;
            }
            target_fd = (int32_t)val;
            break;
        }
        // heredocs TODO
        case REDIR_DLESS: // <<
        case REDIR_DLESSDASH: // <<-
        case REDIR_TLESS: // <<<
            fprintf(stderr, "CyprSH: heredocs not supported.. yet");
            return SUCCESS;
        default: // none or unspecified
            fprintf(stderr, "CyprSH: redirect type not supported\n");
            return ERROR_DEFAULT;
    }

    ERR_CHECK(st);

    if(dup2(target_fd, source_fd) == -1) {
        fprintf(stderr, "CyprSH: failed to redirect fd\n");
        if(target_fd >= 3) close(target_fd);
        return ERROR_DEFAULT;
    }

    if(target_fd >= 3) close(target_fd);

    return st;
}


static void getDefaultFD(RedirectTypeEnum type, int32_t* fd) {
    if(*fd != -1) {
        return;
    }

    switch(type) {
        case REDIR_LESS:
        case REDIR_LESSGREAT:
        case REDIR_LESSAND:
        case REDIR_DLESS:
        case REDIR_DLESSDASH:
        case REDIR_TLESS:
            *fd = STDIN_FILENO;
            break;
        case REDIR_GREAT:
        case REDIR_DGREAT:
        case REDIR_CLOBBER:
        case REDIR_GREATAND:
            *fd = STDOUT_FILENO;
            break;
        default:
            break;
    }
}


static char** buildEnvp(HashTablePtr env) {
    uint32_t htab_size = hashTableGetCurrSize(env);
    char** envp = malloc(sizeof(char*) * (htab_size + 1)); // NULL end
    if(envp == NULL) {
        return NULL;
    }

    HashTableIter iterator;
    hashTableIterCtor(&iterator, env);

    char* key;
    char* value;
    uint32_t index = 0;
    while(hashTableIterNext(&iterator, &key, &value)) {
        size_t item_size = strlen(key) + strlen(value) + 2;  // '=' and '\0'
        char* item = malloc(item_size);

        if(item == NULL) {
            for(uint32_t i = 0; i < index; i++) {
                free(envp[i]);
            }
            free(envp);
            return NULL;
        }
        snprintf(item, item_size, "%s=%s", key, value);
        envp[index++] = item;
    }
    envp[index] = NULL;
    return envp;
}


static char* findPath(const char* cmd, const char* path_env) {
    if(cmd == NULL) {
        return NULL;
    }

    if(strchr(cmd, '/') != NULL) {
        if(access(cmd, X_OK) == 0) {
            return strdup(cmd);
        }
        return NULL;
    }

    if(path_env == NULL || *path_env == '\0') {
        return NULL;
    }

    // copy( strtok modifies)
    char* path_copy = strdup(path_env);
    if(path_copy == NULL) {
        return NULL;
    }

    char* dir = strtok(path_copy, ":");
    while(dir != NULL) {
        // empty - curr dir
        const char* effective_dir = (*dir == '\0') ? "." : dir;

        // build "dir/cmd"
        size_t full_path_size = strlen(effective_dir) + strlen(cmd) + 2;  // '/' + '\0'
        char* full_path = malloc(full_path_size);
        if(full_path == NULL) {
            free(path_copy);
            return NULL;
        }
        snprintf(full_path, full_path_size, "%s/%s", effective_dir, cmd);

        if(access(full_path, X_OK) == 0) {
            free(path_copy);
            return full_path;
        }

        free(full_path);
        dir = strtok(NULL, ":");
    }

    free(path_copy);
    return NULL;
}


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
        default:
            return SUCCESS;
    }
    return SUCCESS;
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
        int8_t saved_flags = env->flags;
        if(child->flags & FLAG_BACKGROUND) {
            env->flags |= EXEC_FLAG_BG;
        }

        st = executeNode(child, env);
        ERR_CHECK(st);
        env->flags = saved_flags;
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
            env->last_exec_status = env->last_exec_status == 0 ? 1 : 0;
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
        env->last_exec_status = env->last_exec_status == 0 ? 1 : 0;
    }

    free(pids);
    return SUCCESS;
}

static StatusEnum executeSimpleCommandNode(ASTNodePtr command_node, ExecuteEnvironmentPtr env) {
    if(command_node->num_children == 0) {
        return ERROR_SYNTAX_ERROR;
    }
    StatusEnum st;
    ASTNodePtr cmd_word = NULL;
    for(int16_t i = 0; i < command_node->num_children; i++) {
        if(command_node->children[i]->type == NODE_CMD_WORD) {
            cmd_word = command_node->children[i];
            break;
        }
    }

    if(cmd_word == NULL) {
        // assignments/redirections? only
        if(command_node->children[0]->type != NODE_CMD_PREFIX) {
            return ERROR_SYNTAX_ERROR;
        }
        return handlePrefix(command_node->children[0], env);
    }

    // builtin, not pipelined
    if(isBuiltin(cmd_word->value) && !(env->flags & EXEC_FLAG_CHILD_PROCESS)) {
        env->last_exec_status = 0;
        return SUCCESS;
    }

    // pipelined or non builtin
    pid_t pid = 0;
    if(!(env->flags & EXEC_FLAG_CHILD_PROCESS)) {
        pid = fork();
        if(pid == -1) {
            fprintf(stderr, "CyprSH: fork failed\n");
            return ERROR_DEFAULT;
        }
    }

    if(pid == 0) {
        char* argv[MAX_ARGS + 1]; // maximum 64 arguments + binary name
        argv[0] = cmd_word->value;
        int16_t argc = 1;
        // fill env, apply redirections, fill argv
        for(int16_t i = 0; i < command_node->num_children; i++) {
            if(command_node->children[i]->type == NODE_CMD_PREFIX) {
                st = handlePrefix(command_node->children[i], env);
                if(st != SUCCESS) {
                    exit(st);
                }
            } else if(command_node->children[i]->type == NODE_CMD_WORD) {
                continue;
            } else if(command_node->children[i]->type == NODE_CMD_SUFFIX) {
                st = handleSuffix(command_node->children[i], argv, &argc);
                if(st != SUCCESS) {
                    exit(st);
                }
            } else {
                exit(ERROR_SYNTAX_ERROR);
            }
        }
    
        argv[argc] = NULL;

        // build env
        char** envp = buildEnvp(env->env_table);
        if(envp == NULL) {
            fprintf(stderr, "CyprSH: failed to build environment\n");
            exit(ERROR_DEFAULT);
        }

        // get executable path
        char* env_path;
        st = hashTableGetValue(env->env_table, "PATH", &env_path);
        if(st != SUCCESS) {
            exit(ERROR_DEFAULT);
        }
        char* exec_path = findPath(argv[0], env_path);
        if(exec_path == NULL) {
            fprintf(stderr, "CyprSH: %s: command not found\n", argv[0]);
            exit(ERROR_COMMAND_NOT_FOUND);
        }

        execve(exec_path, argv, envp);
        fprintf(stderr, "CyprSH: %s: cannot execute\n", argv[0]);
        exit(ERROR_COMM_CANNOT_EXEC);
    } else {
        if(env->flags & EXEC_FLAG_BG) {
            // TODO: handler for zombie cleanup, pid storing for $
        } else {
            int status;
            waitpid(pid, &status, 0);
            if(WIFEXITED(status)) {
                env->last_exec_status = WEXITSTATUS(status);
            } else if(WIFSIGNALED(status)) {
                env->last_exec_status = 128 + WTERMSIG(status);
            }
        }
    }
    
    return SUCCESS;
}