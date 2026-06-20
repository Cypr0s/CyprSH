#ifndef BUILTIN_UNSET_H
#define BUILTIN_UNSET_H

#include "executor/execute_types.h"

StatusEnum builtinUnset(int16_t argc, char** argv, ExecuteEnvironmentPtr env);

#endif