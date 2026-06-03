#ifndef EXECUTE_H
#define EXETUCE_H

#include "data_structures/ast.h"
#include "error.h"
#include "utils/env.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

StatusEnum executeNode(ASTNodePtr node, ExecuteEnvironmentPtr env);

typedef enum {
    EXEC_FLAG_NONE = 0,
    EXEC_FLAG_BG = 1,
    EXEC_FLAG_AND = 2,
    EXEC_FLAG_OR = 4,
    EXEC_FLAG_CHILD_PROCESS = 8,
} ExecFlagEnum;

typedef struct {
    HashTablePtr env_table;
    int8_t flags;
    int8_t last_exec_status;
} ExecuteEnvironment, * ExecuteEnvironmentPtr;

#endif // EXECUTE_H