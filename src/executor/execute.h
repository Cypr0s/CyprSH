#ifndef EXECUTE_H
#define EXECUTE_H

#include "executor/execute_types.h"
#include "utils/env.h"
#include "utils/strings.h"
#include "utils/file.h"
#include "builtins/special/special_builtins.h"
#include "builtins/regular/regular_builtins.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

StatusEnum executeNode(ASTNodePtr node, ExecuteEnvironmentPtr env);

void executorCtor(ExecuteEnvironmentPtr env, HashTablePtr p_env);

#endif // EXECUTE_H