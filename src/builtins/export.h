#ifndef BUILTIN_EXPORT_H
#define BUILTIN_EXPORT_H

#include "executor/execute_types.h"

StatusEnum builtinExport(int16_t argc, char** argv, ExecuteEnvironmentPtr env);

#endif