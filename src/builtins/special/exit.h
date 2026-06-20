#ifndef BUILTIN_EXIT_H
#define BUILTIN_EXIT_H

#include "executor/execute_types.h"

StatusEnum builtinExit(int16_t argc, char** argv, ExecuteEnvironmentPtr env);

#endif