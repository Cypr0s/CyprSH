#ifndef EXECUTE_H
#define EXECUTE_H

#include "executor/execute_types.h"
#include "utils/env.h"
#include "utils/strings.h"
#include "utils/file.h"
#include "builtins/builtins.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

StatusEnum executeNode(ASTNodePtr node, ExecuteEnvironmentPtr env);

#endif // EXECUTE_H