/**
 * @file        execute.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   AST node execution engine
 */

#include "execution/execute.h"

// utilities

static const BuiltinEntry special_builtins[] = {
    {"exit",   builtinExit},
    {"export", builtinExport},
    {"unset",  builtinUnset},
    {":",      builtinTrue},
};

static const BuiltinEntry regular_builtins[] = {
    {"cd",     builtinCd},
    {"pwd",    builtinPwd},
    {"echo",   builtinEcho},
    {"true",   builtinTrue},
    {"false",  builtinFalse},
};

static BuiltIn findInBuiltinArray(const char* name, const BuiltinEntry* array, size_t count);
static StatusEnum executeBuiltinCommand(BuiltIn fn, ASTNodePtr command_node, ASTNodePtr cmd_word, ExecuteEnvironmentPtr env);
static StatusEnum executeExternalProgram(ASTNodePtr command_node, ASTNodePtr cmd_word, ExecuteEnvironmentPtr env);
static StatusEnum handlePrefix(ASTNodePtr command_prefix, ExecuteEnvironmentPtr env);
static StatusEnum handleSuffix(ASTNodePtr command_suffix, char** argv, int16_t* argc);
static StatusEnum handleRedirect(ASTNodePtr redirect_node);
static void getDefaultFD(RedirectTypeEnum type, int32_t* fd);
static char** buildEnvp(HashTablePtr env);
static char* findPath(const char* cmd, const char* path_env);
static int8_t matchCaseItem(ASTNodePtr item_node, const char* match_value);

// execution of nodes

static StatusEnum executeProgramNode(ASTNodePtr program_node, ExecuteEnvironmentPtr env);
static StatusEnum executeCompleteCommandNode(ASTNodePtr command_node, ExecuteEnvironmentPtr env);
static StatusEnum executeListNode(ASTNodePtr list_node, ExecuteEnvironmentPtr env);
static StatusEnum executeAndOrNode(ASTNodePtr and_or_node, ExecuteEnvironmentPtr env);
static StatusEnum executePipelineNode(ASTNodePtr pipeline_node, ExecuteEnvironmentPtr env);
static StatusEnum executeSimpleCommandNode(ASTNodePtr command_node, ExecuteEnvironmentPtr env);
static StatusEnum executeBraceGroupNode(ASTNodePtr brace_group_node, ExecuteEnvironmentPtr env);
static StatusEnum executeSubshellNode(ASTNodePtr subshell_node, ExecuteEnvironmentPtr env);
static StatusEnum executeIfClauseNode(ASTNodePtr if_node, ExecuteEnvironmentPtr env);
static StatusEnum executeElseClauseNode(ASTNodePtr else_node, ExecuteEnvironmentPtr env);
static StatusEnum executeWhileClauseNode(ASTNodePtr while_node, ExecuteEnvironmentPtr env);
static StatusEnum executeUntilClauseNode(ASTNodePtr until_node, ExecuteEnvironmentPtr env);
static StatusEnum executeForClauseNode(ASTNodePtr for_node, ExecuteEnvironmentPtr env);
static StatusEnum executeCaseClauseNode(ASTNodePtr case_node, ExecuteEnvironmentPtr env);

static BuiltIn findInBuiltinArray(const char* name, const BuiltinEntry* array, size_t count) {
    if(name == NULL) return NULL;
    for(size_t i = 0; i < count; i++) {
        if(streq(name, array[i].name)) {
            return array[i].func;
        }
    }
    return NULL;
}


static StatusEnum executeBuiltinCommand(BuiltIn fn, ASTNodePtr command_node, ASTNodePtr cmd_word, ExecuteEnvironmentPtr env) {
    int32_t saved_stdin = dup(STDIN_FILENO);
    int32_t saved_stdout = dup(STDOUT_FILENO);
    int32_t saved_stderr = dup(STDERR_FILENO);

    char* argv[MAX_ARGS + 1];
    argv[0] = cmd_word->value;
    int16_t argc = 1;

    StatusEnum st = SUCCESS;
    for(int16_t i = 0; i < command_node->num_children; i++) {
        ASTNodePtr child = command_node->children[i];
        if(child->type == NODE_CMD_PREFIX) {
            st = handlePrefix(child, env);
            if(st != SUCCESS) break;
        } else if(child->type == NODE_CMD_SUFFIX) {
            st = handleSuffix(child, argv, &argc);
            if(st != SUCCESS) break;
        }
    }
    argv[argc] = NULL;

    if(st == SUCCESS) {
        fn(argc, argv, env);
    }

    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);
    dup2(saved_stderr, STDERR_FILENO);
    close(saved_stdin);
    close(saved_stdout);
    close(saved_stderr);

    if(env->flags & EXEC_FLAG_CHILD_PROCESS) {
        fflush(stdout);
        exit(env->last_exec_status);
    }

    return SUCCESS;
}


static StatusEnum executeExternalProgram(ASTNodePtr command_node, ASTNodePtr cmd_word, ExecuteEnvironmentPtr env) {
    pid_t pid = 0;
    if(!(env->flags & EXEC_FLAG_CHILD_PROCESS)) {
        pid = fork();
        if(pid == -1) {
            fprintf(stderr, "CyprSH: fork failed\n");
            return ERROR_DEFAULT;
        }
    }

    if(pid == 0) {
        char* argv[MAX_ARGS + 1];
        argv[0] = cmd_word->value;
        int16_t argc = 1;
        StatusEnum st;

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

        char** envp = buildEnvp(env->env_table);
        if(envp == NULL) {
            fprintf(stderr, "CyprSH: failed to build environment\n");
            exit(ERROR_DEFAULT);
        }

        char* env_path;
        StatusEnum path_st = hashTableGetValue(env->env_table, "PATH", &env_path);
        if(path_st != SUCCESS) {
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


static int8_t matchCaseItem(ASTNodePtr item_node, const char* match_value) {
    int16_t pattern_count = item_node->num_children;
    if(pattern_count > 0 && item_node->children[pattern_count - 1]->type == NODE_LIST) {
        pattern_count--;
    }

    for(int16_t i = 0; i < pattern_count; i++) {
        const char* pattern = item_node->children[i]->value;
        if(fnmatch(pattern, match_value, 0) == 0) {
            return 1;
        }
    }
    return 0;
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
        } else {
            env->flags &= ~EXEC_FLAG_BG;
        }

        st = executeNode(child, env);
        ERR_CHECK(st);

        // stop if exit was triggered
        if(env->flags & EXEC_FLAG_EXIT) break;
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

    ASTNodePtr cmd_word = NULL;
    for(int16_t i = 0; i < command_node->num_children; i++) {
        if(command_node->children[i]->type == NODE_CMD_WORD) {
            cmd_word = command_node->children[i];
            break;
        }
    }

    if(cmd_word == NULL) {
        if(command_node->children[0]->type != NODE_CMD_PREFIX) {
            return ERROR_SYNTAX_ERROR;
        }
        return handlePrefix(command_node->children[0], env);
    }

    // special builtins
    BuiltIn fn = findInBuiltinArray(cmd_word->value, special_builtins,
                                     sizeof(special_builtins) / sizeof(special_builtins[0])
                                );
    if(fn != NULL) {
        return executeBuiltinCommand(fn, command_node, cmd_word, env);
    }

    // defined function
    ASTNodePtr func_body = functionListFind(&(env->function_list), cmd_word->value);
    if(func_body != NULL) {
        return executeNode(func_body, env);
    }

    // regular builtin
    fn = findInBuiltinArray(cmd_word->value, regular_builtins,
                             sizeof(regular_builtins) / sizeof(regular_builtins[0])
                        );
    if(fn != NULL) {
        return executeBuiltinCommand(fn, command_node, cmd_word, env);
    }

    // external
    return executeExternalProgram(command_node, cmd_word, env);
}


static StatusEnum executeBraceGroupNode(ASTNodePtr brace_group_node, ExecuteEnvironmentPtr env) {
    if(brace_group_node->num_children == 0) {
        return ERROR_SYNTAX_ERROR;
    }
    return executeNode(brace_group_node->children[0], env);
}


static StatusEnum executeSubshellNode(ASTNodePtr subshell_node, ExecuteEnvironmentPtr env) {
    if(subshell_node->num_children == 0) {
        return ERROR_SYNTAX_ERROR;
    }

    pid_t p = fork();
    if(p == -1) {
        return ERROR_DEFAULT;
    }

    if(p == 0) {
        StatusEnum st = executeNode(subshell_node->children[0], env);
        if(st != SUCCESS) {
            exit(st);
        }
        exit(env->last_exec_status);
    }
    int32_t status; 
    waitpid(p, &status, 0);
    env->last_exec_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    return SUCCESS;
}


static StatusEnum executeIfClauseNode(ASTNodePtr if_node, ExecuteEnvironmentPtr env) {
    if(if_node->num_children < 2 || if_node->num_children > 3) {
        return ERROR_SYNTAX_ERROR;
    }

    StatusEnum st = executeNode(if_node->children[0], env);
    ERR_CHECK(st);

    if(env->last_exec_status == 0) {
        return executeNode(if_node->children[1], env);
    }
    // non zero 
    if(if_node->num_children == 3) {
        return executeNode(if_node->children[2], env);
    }
    return SUCCESS;
}


static StatusEnum executeElseClauseNode(ASTNodePtr else_node, ExecuteEnvironmentPtr env) {
    if(else_node->num_children == 0) {
        return ERROR_SYNTAX_ERROR;
    }
    return executeNode(else_node->children[0], env);
}


static StatusEnum executeWhileClauseNode(ASTNodePtr while_node, ExecuteEnvironmentPtr env) {
    if(while_node->num_children != 2) {
        return ERROR_SYNTAX_ERROR;
    }

    StatusEnum st;
    int8_t executed_body = 0;

    while(1) {
        st = executeNode(while_node->children[0], env);
        ERR_CHECK(st);

        if(env->last_exec_status != 0) {
            break;
        }

        executed_body = 1;
        st = executeNode(while_node->children[1], env);
        ERR_CHECK(st);

        if(env->flags & EXEC_FLAG_EXIT) {
            break;
        }
    }

    if(!executed_body) {
        env->last_exec_status = 0;
    }

    return SUCCESS;
}


static StatusEnum executeUntilClauseNode(ASTNodePtr until_node, ExecuteEnvironmentPtr env) {
    if(until_node->num_children != 2) {
        return ERROR_SYNTAX_ERROR;
    }

    StatusEnum st;
    int8_t executed_body = 0;

    while(1) {
        st = executeNode(until_node->children[0], env);
        ERR_CHECK(st);

        if(env->last_exec_status == 0) {
            break;
        }

        executed_body = 1;
        st = executeNode(until_node->children[1], env);
        ERR_CHECK(st);

        if(env->flags & EXEC_FLAG_EXIT) {
            break;
        }
    }

    if(executed_body == 0) {
        env->last_exec_status = 0;
    }

    return SUCCESS;
}


static StatusEnum executeForClauseNode(ASTNodePtr for_node, ExecuteEnvironmentPtr env) {
    if(for_node->num_children < 2) {
        return ERROR_SYNTAX_ERROR;
    }

    char* var_name = for_node->children[0]->value;
    int16_t word_count = for_node->num_children - 2; // - `var` [0] and `body` [num_children - 1]

    StatusEnum st;
    int8_t executed_body = 0;

    for(int16_t i = 1; i <= word_count; i++) {
        char* word_value = for_node->children[i]->value;

        st = hashTableInsert(env->env_table, var_name, word_value);
        ERR_CHECK(st);

        executed_body = 1;
        st = executeNode(for_node->children[for_node->num_children - 1], env);
        ERR_CHECK(st);

        if(env->flags & EXEC_FLAG_EXIT) {
            break;
        }
    }

    if(executed_body == 0) {
        env->last_exec_status = 0;
    }

    return SUCCESS;
}


static StatusEnum executeCaseClauseNode(ASTNodePtr case_node, ExecuteEnvironmentPtr env) {
    if(case_node->num_children < 1) {
        return ERROR_SYNTAX_ERROR;
    }

    const char* match_value = case_node->children[0]->value;

    StatusEnum st;
    int8_t executed_any = 0;
    int8_t fall_through = 0;

    for(int16_t i = 1; i < case_node->num_children; i++) {
        ASTNodePtr item = case_node->children[i];
        if(item->num_children == 0) {
            return ERROR_SYNTAX_ERROR;
        } 

        if(fall_through == 0 && matchCaseItem(item, match_value) == 0) {
            continue;
        }

        executed_any = 1;

        ASTNodePtr body = item->children[item->num_children - 1];
        if(body->type == NODE_LIST) {
            st = executeNode(body, env);
            ERR_CHECK(st);
        }

        if(item->flags & FLAG_SEMI_AND) { // ;& -> fall through
            fall_through = 1;
            continue;
        }

        break;  // ;; -> end
    }

    if(executed_any == 0) {
        env->last_exec_status = 0;
    }

    return SUCCESS;
}


static StatusEnum executeFunctionDefinition(ASTNodePtr function_node, ExecuteEnvironmentPtr env) {
    if(function_node->num_children < 2) {
        return ERROR_SYNTAX_ERROR;
    }

    char* name = function_node->children[0]->value;
    ASTNodePtr body = function_node->children[1];

    return functionListInsert(&(env->function_list), name, body);
}



void executorCtor(ExecuteEnvironmentPtr env, HashTablePtr p_env) {
   env->env_table = p_env;
   env->last_exec_status = 0;
   env->flags = EXEC_FLAG_NONE;
   functionListCtor(&(env->function_list));
}


void executorDtor(ExecuteEnvironmentPtr env) {
    functionListDtor(&(env->function_list));
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
        case NODE_BRACE_GROUP: return executeBraceGroupNode(node, env);
        case NODE_SUBSHELL: return executeSubshellNode(node, env);
        case NODE_IF_CLAUSE: return executeIfClauseNode(node, env);
        case NODE_ELSE_CLAUSE: return executeElseClauseNode(node, env);
        case NODE_WHILE_CLAUSE: return executeWhileClauseNode(node, env);
        case NODE_UNTIL_CLAUSE: return executeUntilClauseNode(node, env);
        case NODE_FOR_CLAUSE: return executeForClauseNode(node, env);
        case NODE_CASE_CLAUSE: return executeCaseClauseNode(node, env);
        case NODE_FUNCTION_DEF: return executeFunctionDefinition(node, env);
        default:
            return SUCCESS;
    }
    return SUCCESS;
}