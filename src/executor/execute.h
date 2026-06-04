#ifndef EXECUTE_H
#define EXECUTE_H

#include "data_structures/ast.h"
#include "error.h"
#include "utils/env.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "utils/strings.h"
#include "utils/file.h"
#include "builtins/builtins.h"

#define MAX_ARGS 64
 // subject to change, as more builtins will be added(maybe better solution than keeping count?)
#define BUILTIN_FUNCTION_COUNT 4


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
    uint8_t last_exec_status;
} ExecuteEnvironment, *ExecuteEnvironmentPtr;

typedef StatusEnum (*BuiltIn) (int16_t argc, char** argv, ExecuteEnvironmentPtr env);

typedef struct {
    char* name;
    BuiltIn func;
} BuiltinEntry;

StatusEnum executeNode(ASTNodePtr node, ExecuteEnvironmentPtr env);

#endif // EXECUTE_H