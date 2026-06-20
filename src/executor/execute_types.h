#ifndef EXECUTE_TYPES_H
#define EXECUTE_TYPES_H

#include "data_structures/ast.h"
#include "data_structures/htab.h"
#include "data_structures/function_list.h"
#include "error.h"
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MAX_ARGS 64

typedef enum {
    EXEC_FLAG_NONE = 0,
    EXEC_FLAG_BG = 1,
    EXEC_FLAG_AND = 2,
    EXEC_FLAG_OR = 4,
    EXEC_FLAG_CHILD_PROCESS = 8,
    EXEC_FLAG_EXIT = 16,
} ExecFlagEnum;

typedef struct {
    HashTablePtr env_table;
    FunctionList function_list;
    int8_t flags;
    uint8_t last_exec_status;
} ExecuteEnvironment, *ExecuteEnvironmentPtr;

typedef StatusEnum (*BuiltIn) (int16_t argc, char** argv, ExecuteEnvironmentPtr env);

typedef struct {
    char* name;
    BuiltIn func;
} BuiltinEntry;

#endif // EXECUTE_TYPES_H